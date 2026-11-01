@echo off
setlocal
where cmake >nul 2>nul || (echo CMake is required to BUILD this package & exit /b 1)
cmake -S . -B build -A x64 || exit /b 1
cmake --build build --config Release || exit /b 1
ctest --test-dir build -C Release --output-on-failure || exit /b 1
if not exist package mkdir package
if not exist package\results mkdir package\results
copy /Y build\Release\Launch.exe package\Launch.exe >nul || exit /b 1
copy /Y build\Release\workbench.exe package\workbench.exe >nul || exit /b 1
xcopy /E /I /Y data package\data >nul
xcopy /E /I /Y results package\results >nul
echo Windows package ready in package\. Double-click package\Launch.exe.
