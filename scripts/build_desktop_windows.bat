@echo off
setlocal
cd /d "%~dp0\.."
echo [1/4] Checking required tools...
where cmake >nul 2>nul || (echo ERROR: Install CMake and add it to PATH.& exit /b 1)
where npm >nul 2>nul || (echo ERROR: Install Node.js LTS including npm.& exit /b 1)
echo [2/4] Building C++17 backend...
cmake -G "NMake Makefiles" -S . -B build-desktop -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build build-desktop || exit /b 1
echo [3/4] Running C++ tests...
ctest --test-dir build-desktop -C Release --output-on-failure || exit /b 1
echo [4/4] Installing desktop dependencies and packaging...
call npm install || exit /b 1
call npm run dist:win || exit /b 1
echo.
echo SUCCESS: Installer is in release\
echo The installer bundles the C++ backend. Electron supplies the desktop web runtime.
