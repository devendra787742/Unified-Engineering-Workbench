const { app, BrowserWindow, ipcMain, dialog, shell } = require('electron');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

let mainWindow = null;
let activeChild = null;
let activeToken = 0;

function backendPath() {
  if (app.isPackaged) return path.join(process.resourcesPath, 'backend', 'workbench.exe');
  const candidates = [
    path.join(__dirname, '..', 'build-desktop', 'Release', 'workbench.exe'),
    path.join(__dirname, '..', 'build-desktop', 'workbench.exe'),
    path.join(__dirname, '..', 'build', 'Release', 'workbench.exe'),
    path.join(__dirname, '..', 'build', 'workbench.exe')
  ];
  return candidates.find(fs.existsSync) || candidates[0];
}
function resultsDir() {
  const dir = path.join(app.getPath('userData'), 'results');
  fs.mkdirSync(dir, { recursive: true });
  return dir;
}
function runCore(args) {
  return new Promise((resolve, reject) => {
    if (activeChild) return reject(new Error('Another C++ task is already running.'));
    const exe = backendPath();
    if (!fs.existsSync(exe)) return reject(new Error(`C++ backend not found: ${exe}. Build the desktop package first.`));
    const token = ++activeToken;
    const child = spawn(exe, args, { cwd: path.dirname(exe), windowsHide: true, shell: false });
    activeChild = child;
    let stdout = '', stderr = '';
    child.stdout.setEncoding('utf8'); child.stderr.setEncoding('utf8');
    child.stdout.on('data', chunk => { stdout += chunk; });
    child.stderr.on('data', chunk => { stderr += chunk; });
    child.on('error', err => {
      if (activeToken === token) activeChild = null;
      reject(err);
    });
    child.on('close', (code, signal) => {
      if (activeToken === token) activeChild = null;
      resolve({ exitCode: code === null ? -1 : code, signal, stdout, stderr });
    });
  });
}
function ensureSafeName(name) {
  const base = path.basename(String(name || 'upload.dat')).replace(/[^a-zA-Z0-9._-]/g, '_');
  if (!base || base === '.' || base === '..') throw new Error('Invalid file name.');
  return base;
}

ipcMain.handle('backend:status', () => {
  const exe = backendPath();
  return { available: fs.existsSync(exe), executable: exe, packaged: app.isPackaged };
});
ipcMain.handle('backend:importFile', async (_event, payload) => {
  if (!payload || !['stl', 'csv'].includes(payload.kind) || !payload.bytes) throw new Error('Invalid import request.');
  const name = ensureSafeName(payload.name);
  const expected = payload.kind === 'stl' ? /\.stl$/i : /\.csv$/i;
  if (!expected.test(name)) throw new Error(`Choose a ${payload.kind.toUpperCase()} file.`);
  const bytes = Buffer.from(payload.bytes);
  if (!bytes.length || bytes.length > 25 * 1024 * 1024) throw new Error('File is empty or exceeds 25 MB.');
  const dir = path.join(app.getPath('userData'), 'imports');
  fs.mkdirSync(dir, { recursive: true });
  const target = path.join(dir, `${Date.now()}_${name}`);
  fs.writeFileSync(target, bytes);
  const command = payload.kind === 'stl' ? 'import-stl' : 'import-csv';
  return runCore([command, target]);
});
ipcMain.handle('backend:runTask', async (_event, task, params = {}) => {
  const out = resultsDir();
  if (task === 'fea') {
    const n = Number(params.elements);
    if (!Number.isInteger(n) || n < 1 || n > 500) throw new Error('Mesh elements must be an integer from 1 to 500.');
    return runCore(['fea', String(n), path.join(out, 'thermal_latest.csv')]);
  }
  if (task === 'benchmark') {
    const cases = Number(params.cases), elements = Number(params.elements), seed = Number(params.seed);
    if (![cases, elements, seed].every(Number.isInteger) || cases < 1 || cases > 100000 || elements < 1 || elements > 500 || seed < 0) throw new Error('Invalid benchmark parameters.');
    return runCore(['batch', String(cases), String(elements), String(seed)]);
  }
  if (task === 'hardware') return runCore(['hardware']);
  if (task === 'backprop') return runCore(['backprop', path.join(out, 'backprop_latest.csv')]);
  throw new Error('Unsupported backend task.');
});
ipcMain.handle('backend:cancel', () => {
  if (!activeChild) return { cancelled: false, message: 'No task is currently running.' };
  activeChild.kill();
  return { cancelled: true, message: 'Cancellation requested.' };
});
ipcMain.handle('backend:export', async (_event, fileName) => {
  const safe = ensureSafeName(fileName);
  if (!['thermal_latest.csv', 'backprop_latest.csv'].includes(safe)) throw new Error('Unsupported export file.');
  const source = path.join(resultsDir(), safe);
  if (!fs.existsSync(source)) throw new Error('Run the corresponding C++ task before exporting.');
  const picked = await dialog.showSaveDialog(mainWindow, { defaultPath: safe, filters: [{ name: 'CSV', extensions: ['csv'] }] });
  if (picked.canceled || !picked.filePath) return { cancelled: true };
  fs.copyFileSync(source, picked.filePath);
  return { message: `Exported ${path.basename(picked.filePath)}` };
});
ipcMain.handle('backend:openResults', async () => shell.openPath(resultsDir()));

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1480, height: 960, minWidth: 980, minHeight: 680,
    backgroundColor: '#0b1119',
    title: 'Unified Engineering Workbench',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true
    }
  });
  mainWindow.loadFile(path.join(__dirname, '..', 'frontend', 'index.html'));
  mainWindow.on('closed', () => { mainWindow = null; });
}
app.whenReady().then(() => {
  app.setAppUserModelId('com.unifiedengineering.workbench');
  createWindow();
  app.on('activate', () => { if (BrowserWindow.getAllWindows().length === 0) createWindow(); });
});
app.on('before-quit', () => { if (activeChild) activeChild.kill(); });
app.on('window-all-closed', () => { if (process.platform !== 'darwin') app.quit(); });
