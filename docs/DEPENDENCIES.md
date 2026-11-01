# Dependencies and versions

- C++ language standard: C++17.
- CMake minimum: 3.16.
- Build dependency: CMake and a C++17 compiler; Ninja is used only by the optional CMake preset.
- Runtime/library dependencies: C++ standard library and platform threading library (`Threads::Threads`).
- Third-party libraries: none.
- Licences: project source is supplied for candidate evaluation; no third-party code or external geometry assets are bundled. Verify and add a chosen project licence before public redistribution.
- Editor: VS Code with CMake Tools is recommended for configuring/building; the `.vscode/tasks.json` tasks run the CMake build and CLI. This editor configuration is not the required standalone packaged Windows application.
