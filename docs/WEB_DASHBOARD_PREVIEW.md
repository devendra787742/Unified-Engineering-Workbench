# Modern web dashboard preview

The `frontend/` directory contains a dependency-free HTML/CSS/JavaScript dashboard prototype.

## Open it

Open `frontend/index.html` in a modern desktop browser. No npm install, build step, external CDN, or internet connection is required.

## Included in this preview

- Overview with verified local backend results supplied during development.
- ASCII STL text preview with interactive rotate, pan, zoom and reset controls.
- Temperature CSV preview with coordinate-based point cloud, temperature colour mapping and nearest-node inspection.
- FEA convergence and parallel benchmark charts.
- MCU / FPGA / JTAG mock status view.
- Backpropagation finite-difference comparison.

## Important integration boundary

This is a frontend preview, not yet the final Windows desktop package. The C++17 CLI remains the authoritative parser and numerical backend. The browser preview parses the selected STL/CSV in JavaScript solely to render the local visualization; do not treat that preview parsing as evidence that the C++ parser ran for the same browser-selected file. The dashboard's benchmark and verification figures are the observed values recorded from the current local prototype run, not live measurements.

The next integration step is to host this UI in a Windows WebView2 shell and expose a narrow, validated bridge to the existing C++ backend. The standalone package must include/legally bundle required WebView2 bootstrap/runtime dependencies or document the runtime prerequisite; clean-machine launch must be tested before claiming delivery complete.

## Preview limitations

- The 3D view uses a lightweight custom canvas projection and painter's ordering, not a full CAD renderer or Three.js. It is suitable for a small prototype STL, not large production meshes.
- Browser preview file limit: 25 MB. The C++ parser currently has its own separate validation path.
- No mesh-to-temperature registration is implemented.
- No physical MCU, FPGA, or JTAG device is accessed; hardware states are simulated.


## Integrated desktop mode

For the C++-connected desktop application, use the Electron shell and follow `docs/DESKTOP_INTEGRATION.md`. The desktop app exposes a restricted `window.workbench` bridge; opening `frontend/index.html` directly in a browser intentionally remains a frontend-only preview. In desktop mode, STL/CSV uploads are also validated by the C++ parser, and FEA, benchmark, hardware mock, and backpropagation actions execute the C++ command-line engine.
