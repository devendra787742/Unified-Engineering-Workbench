# Desktop integration (Electron + C++17)

The browser dashboard is now wired to the C++17 CLI through a restricted Electron preload bridge.

## Architecture

- `frontend/` remains the HTML/CSS/JavaScript dashboard and its interactive STL/temperature preview.
- `desktop/main.js` is the privileged main process. It validates task names and parameters, stores imported files under Electron's per-user application data directory, and starts the C++ executable with `spawn()` (never a shell command).
- `desktop/preload.js` exposes only the narrow `window.workbench` API to the renderer. Node integration is disabled and context isolation is enabled.
- The C++ executable remains the authority for STL/CSV validation, thermal FEA, parallel benchmarking, hardware mock execution and backpropagation.
- Generated CSV files are stored in the application's per-user `results` directory. Export uses a native Save dialog.

## Windows build

Prerequisites:
- Windows 10/11 x64
- CMake 3.16+
- Visual Studio C++ Build Tools (MSVC and Windows SDK)
- Node.js LTS with npm
- Internet access during `npm install` / Electron packaging to download development dependencies

Open the **Visual Studio 2019 x64 Native Tools Command Prompt** (the same prompt where `cl` worked), ensure CMake and npm are on `PATH`, then run from the project root:

```bat
scripts\build_desktop_windows.bat
```

The script uses the proven `NMake Makefiles` generator and a separate `build-desktop` directory to avoid colliding with the existing `build` directory. It builds the C++ project, runs CTest, installs Electron dependencies and creates a Windows NSIS installer under `release\`.

The installer bundles `workbench.exe`; Electron provides the desktop web runtime. End-user machines do not need Node.js or CMake. This packaging path does not use WebView2.

## Run in development

After running the build script (or after `npm install` and building the C++ target), use:

```bat
npm start
```

The development backend lookup checks `build\Release\workbench.exe` and `build\workbench.exe`. The packaged app looks for `resources\backend\workbench.exe`.

## Connected actions

- Loading STL/CSV in the desktop app sends the selected file bytes to the main process; the main process writes a bounded-size temporary copy and invokes `import-stl` / `import-csv` in C++.
- `Run C++ FEA` invokes `fea` and writes `thermal_latest.csv`.
- `Run C++ benchmark` invokes the real batch benchmark with the displayed default parameters (1000 cases, 40 elements, seed 42).
- Hardware and backpropagation buttons invoke their corresponding C++ commands.
- Cancel requests termination of the active child process. A terminated task may leave a partial output file; rerun the task before exporting.
- CSV export uses a native save dialog.

## Honest limitations / verification

The 3D geometry and nodal point-cloud rendering remain browser-side visualization code; the C++ parser validates the same selected files through the bridge. No mesh-to-temperature registration is implemented. Hardware adapters are mocks only. The benchmark chart's initial values are historical sample results; run the benchmark in the desktop app to generate a fresh result in the execution log. The chart is not automatically rewritten from parsed benchmark output yet.

A Windows installer has **not** been built or clean-machine tested in this development environment. Build and verify it on Windows using the script above. Do not claim the installer or clean-machine launch is verified until that test is performed.
