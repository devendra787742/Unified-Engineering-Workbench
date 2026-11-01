# Windows desktop interface (new)

The Win32 native launcher is implemented in `src/launcher_win.cpp`. It has buttons for Geometry, Temperature CSV, Thermal FEA, Performance, Hardware and Backpropagation. It invokes the C++ CLI in a worker thread using a hidden child process and captures its output in a log. File dialogs, status, mesh validation and results-folder opening are provided. This is a **partial UI**, not an interactive 3D viewer: STL/CSV files are currently validated but not rendered; the CSV graphs in `results/` are also not plotted in the desktop window.

## Build on Windows 11 x64

Install CMake and Visual Studio 2022 with Desktop development with C++ (build-time only). Open an x64 Developer Command Prompt and run `scripts\package_windows.bat` in the repository root. It creates `package\Launch.exe` and `package\workbench.exe`; move/copy the whole `package` folder to a clean Windows 11 computer and double-click Launch.exe. The launcher itself does not invoke compilers, an editor, or the internet. The application uses standard Windows system libraries and the C++ runtime associated with the selected toolchain; verify the runtime dependencies on a clean machine before submission.

VS Code: open the folder and select **Terminal > Run Task > Run packaged application (Windows)** after packaging. CMake Tools is optional; VS Code's built-in task support suffices.

## Verification needed

This environment cannot build or execute Win32 applications. The new GUI code is untested on Windows. Capture the launch-to-ready timing on your actual machine and record CPU, physical/logical cores, RAM, compiler, flags, and five timing runs there. Do not claim these are measured unless you have done them. `Launch.exe` is not included in the current archive and is a mandatory missing submission item.

The assignment also requires actual interactive STL triangle rendering, 3D point-cloud rendering with inspection, cancellation or safe completion, handwritten work, and genuine screenshots/video. These are not complete yet.
