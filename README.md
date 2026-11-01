# Unified Engineering Workbench (prototype)

A C++17 engineering-workbench core implementing the numerical and data-processing portions of the PASS Science Works common internship assignment. This is a source prototype, not a finished Windows packaged GUI: the standalone `Launch.exe`, interactive 3D frontend and clean-machine Windows packaging still need to be completed and verified on Windows 11.

## Implemented
- 1D steady-state two-node linear-element thermal FEA with Dirichlet endpoint temperatures and midpoint analytical error.
- Deterministic synthetic batch generation and real CPU worker-pool execution; serial/parallel agreement check and five-repeat median timing.
- Strict ASCII STL triangle and nodal temperature CSV parsers with finite-value and empty-data checks.
- Simulated MCU, FPGA and JTAG adapters, telemetry, connect/disconnect and a recoverable FPGA timeout.
- Single-sample tanh backpropagation, simultaneous update, and 100-step loss CSV.
- CMake build and core tests.

## Build (CMake 3.16+, C++17 compiler)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Run
```bash
# Thermal meshes; creates results/fea_Nxx.csv
./build/workbench fea 10 results/fea_N10.csv
./build/workbench fea 20 results/fea_N20.csv
./build/workbench fea 40 results/fea_N40.csv
./build/workbench fea 80 results/fea_N80.csv

# Imports
./build/workbench import-stl data/unit_cube_ascii.stl
./build/workbench import-csv data/temperature_nodes_synthetic.csv

# Parallel benchmark: count, elements per case, fixed seed
./build/workbench batch 2000 80 20261101

# Hardware mock and backpropagation
./build/workbench hardware
./build/workbench backprop results/backprop_loss.csv
```

## Thermal model
`-d/dx(k A dT/dx) = q A`, with `L=1 m`, `A=1e-4 m^2`, `k=10 W/(m K)`, `q=10000 W/m^3`, `T(0)=100 C`, `T(L)=20 C`. Lateral surfaces are insulated; properties are constant. The exact solution is `T(x)=100-80x/L+[q/(2k)]x(L-x)`. Endpoint values are imposed by elimination, not penalty stiffness.

## Imports and limitations
- STL: ASCII facet/vertex form only; native CAD solids and binary STL are out of scope. Coordinates are treated as model units supplied by the file; the example is self-created and dimensionless (unit cube).
- CSV: exact header `node_id,x_m,y_m,z_m,temperature_C`; coordinates in metres and temperature in Celsius. The included five-node dataset is synthetic, not an ANSYS export.
- Current CLI validates imported data but does not yet render an interactive 3D view. A UI adapter can consume the validated triangle/node arrays.
- Hardware is simulated only. No physical wiring, firmware deployment or FPGA programming is performed.
- Benchmark timings are machine-specific. Record CPU model, physical/logical core counts, RAM, compiler and flags from the actual target machine before submission. The CLI reports logical thread count; physical cores/RAM/compiler should be added to the run log.
- No Windows `Launch.exe` is included. Build and package on Windows 11 with the chosen compiler/runtime; do not represent a locally built Linux binary as the required Windows launcher.

## Data provenance
- `data/unit_cube_ascii.stl`: self-created ASCII STL, 12 triangles forming a unit cube; no external asset.
- `data/temperature_nodes_synthetic.csv`: self-created synthetic nodal temperature sample; explicitly not an ANSYS export.

## AI and ownership disclosure
This prototype was drafted with assistance from OpenAI ChatGPT (GPT-6). All generated code must be reviewed, built, tested and understood by the candidate. Original candidate work should include local edits, additional tests, machine-specific benchmark evidence, UI integration and version-control history. Do not claim unperformed tests or fabricate prompt logs. Add the actual prompts, edits, incorrect suggestions found and validation outcomes to `docs/AI_USE_LOG.md`.

## Submission readiness
See `SUBMISSION_STATUS.md` before using this as a submission. The current bundle is a tested source prototype, not a finished assignment-compliant package. In particular, it does not contain `Launch.exe` or the required interactive 3D GUI. Do not claim a successful Windows standalone launch until it has been built and tested on Windows 11.

## Windows desktop GUI addition

See `docs/WINDOWS_GUI.md`. `src/launcher_win.cpp` contains a native Win32 UI for the C++ core; run `scripts/package_windows.bat` on a Windows 11 x64 development machine. This source has not yet been compiled or tested on Windows. The ZIP does not include a built Launch.exe. The desktop UI currently validates imported files but does not render an interactive 3D view. See `SUBMISSION_STATUS.md` before submission.

## Modern web dashboard preview

A dependency-free browser UI prototype is available at `frontend/index.html`. Open it in a modern browser to inspect the dashboard, load ASCII STL and temperature CSV examples, rotate/pan/zoom the lightweight 3D previews, inspect temperature nodes, and view the recorded FEA/benchmark/backpropagation evidence. See `docs/WEB_DASHBOARD_PREVIEW.md` for the precise scope and limitations.

**Important:** This is a frontend preview, not yet a WebView2-integrated standalone Windows desktop package. The C++17 CLI remains the authoritative numerical backend. Browser-side file parsing is used for visualization preview only; it does not replace or prove execution of the C++ parsers. The final WebView2 bridge, clean-machine packaging, and runtime verification remain to be completed.


## Integrated Windows desktop application

The Electron desktop shell now exposes a restricted bridge from the modern dashboard to the C++17 executable. See `docs/DESKTOP_INTEGRATION.md` and run `scripts/build_desktop_windows.bat` on Windows with CMake, MSVC Build Tools, Windows SDK, and Node.js LTS. The script uses `build-desktop/` so it does not conflict with the original NMake `build/` folder. The installer output is written to `release/`.

The connected desktop actions run C++ STL/CSV validation, thermal FEA, parallel benchmark, hardware mocks, and backpropagation. Imported files and generated results are stored in the app's per-user data directory. The browser-only preview opened directly from `file://` does not have the native bridge; use `npm start` or the packaged desktop app for C++ integration.

**Packaging status:** source and build scripts are provided, but a Windows installer and clean-machine test have not been produced or verified here. The C++ 3D mesh parser and browser renderer are still separate components; no mesh-to-temperature registration or physical hardware access is claimed.
