const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('workbench', Object.freeze({
  status: () => ipcRenderer.invoke('backend:status'),
  importFile: (kind, name, bytes) => ipcRenderer.invoke('backend:importFile', {
    kind, name, bytes: Array.from(bytes)
  }),
  runTask: (task, params) => ipcRenderer.invoke('backend:runTask', task, params),
  cancel: () => ipcRenderer.invoke('backend:cancel'),
  exportResult: fileName => ipcRenderer.invoke('backend:export', fileName),
  openResults: () => ipcRenderer.invoke('backend:openResults')
}));
