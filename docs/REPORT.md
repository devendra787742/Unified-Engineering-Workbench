# Unified Engineering Workbench — Engineering Report

**Prototype status:** C++17 numerical/data core implemented and locally verified. The assignment's Windows one-click GUI and interactive 3D visualizations are not yet complete; see limitations.

## 1. Design and architecture

The prototype separates numerical/data functions in `src/workbench.cpp` from build configuration and automated tests. The C++ command-line executable exposes thermal FEA, deterministic batch benchmarking, STL/CSV validation, hardware mocks and backpropagation. CMake provides a reproducible source build. A production submission should place a UI/presentation layer over these core APIs without moving numerical or file-processing logic out of C++.

## 2. Thermal FEA formulation and verification

The model solves `-d/dx(kA dT/dx) = qA` on a 1 m rod, with constant `A=1e-4 m²`, `k=10 W/(m K)`, `q=10000 W/m³`, `T(0)=100 °C`, `T(L)=20 °C`, and insulated lateral surfaces. Two-node linear elements use `Ke=(kA/le)[[1,-1],[-1,1]]` and `fe=(qA le/2)[1,1]ᵀ`. The endpoint temperatures are imposed by eliminating prescribed degrees of freedom.

The analytical solution is `T_exact(x)=100-80x/L+[q/(2k)]x(L-x)`. Maximum error is evaluated at each element midpoint using linear FE interpolation. For this constant-coefficient quadratic solution, the midpoint error is `q le²/(8k)=125/N² °C`.

| N | Maximum midpoint error (°C) |
|---:|---:|
| 10 | 1.250000 |
| 20 | 0.312500 |
| 40 | 0.078125 |
| 80 | 0.019531 |

The error decreases by four when the element count doubles, consistent with second-order convergence. A useful debugging insight: the coefficient is `q/(2k)=500`, not `q/k`; omitting the factor of two doubles the predicted midpoint error.

![Thermal profiles](thermal_convergence.png)

![FEA convergence](fea_error_convergence.png)

## 3. CAD / nodal result imports

The parser accepts ASCII STL `vertex x y z` records and rejects incomplete/empty/non-finite triangle data. The example is a self-created unit cube with 12 triangles. The CSV parser requires exactly `node_id,x_m,y_m,z_m,temperature_C`, validates finite numeric values and rejects empty datasets. `temperature_nodes_synthetic.csv` is explicitly synthetic and is not claimed to be an ANSYS export. The example coordinate units are metres and temperature is Celsius. Current prototype validates imports but does not yet provide rotate/pan/zoom/fit-to-view 3D rendering, node inspection or a temperature legend.

## 4. Multicore execution

The batch generator uses fixed seed `20261101`, 2,000 independent cases, 80 elements per case, positive `k` sampled from [5,25] W/(m K), and positive `q` sampled from [1,000,20,000] W/m³. Serial and parallel paths call the same solver; an atomic work index assigns cases to a bounded number of `std::thread` workers and stores results in deterministic input order. One warm-up is excluded; five measured repetitions are used and the median is reported. File I/O and rendering are excluded.

| Workers | Median time (s) | Speedup | Efficiency | Max difference (°C) |
|---:|---:|---:|---:|---:|
| 1 | 0.218792 | 1.000 | 1.000 | 0 |
| 2 | 0.113629 | 1.926 | 0.963 | 0 |
| 3 | 0.076636 | 2.855 | 0.952 | 0 |
| 4 | 0.063874 | 3.425 | 0.856 | 0 |

Measured host: AMD EPYC 9V74 80-Core Processor as exposed to this runtime; 1 socket, 5 available logical CPUs / 5 exposed cores, 5.8 GiB RAM. The benchmark is environment/container-specific and should be repeated on the candidate's Windows 11 target. Compiler information is in `results/compiler_info.txt`; build flags are CMake Release with `-O2 -Wall -Wextra -Wpedantic` on this Linux test host. Four-worker efficiency falls below 1 due to scheduling and thread overhead; no claim is made that these numbers generalize to other machines. Launch-to-ready time was not measured.

## 5. Embedded integration design

`docs/EMBEDDED_DESIGN.md` provides the host/MCU/FPGA/JTAG block diagram and interface assumptions. The C++ mocks implement discovery, capability fields, connect/disconnect and simulated telemetry. A forced FPGA timeout is caught and reported as recoverable. The JTAG adapter is a debug transport, not an interchangeable compute board. Physical pinouts, voltage levels and connector mapping must be confirmed against exact board manuals before any wiring; no hardware was connected.

## 6. Backpropagation

For `x=1`, `y=2`, `h=tanh(wx+b)`, `ŷ=vh+c`, and `J=0.5(ŷ-y)²`, the initial forward pass gives `h=0.4621171573`, `ŷ=0.4621171573`, and `J=1.182541819`. The gradients are `dJ/dc=e`, `dJ/dv=e h`, `dJ/dw=e v (1-h²)x`, and `dJ/db=e v (1-h²)`, where `e=ŷ-y`. Numerically: `dw=db=-1.20946447524`, `dv=-0.710682047486`, `dc=-1.53788284274`. A simultaneous update with learning rate 0.1 gives `w=0.620946447524`, `b=0.120946447524`, `v=1.07106820475`, `c=0.153788284274`. Central finite differences with epsilon `1e-6` agree with the analytic gradients to substantially better than `1e-5`.

![Backprop loss](backprop_loss.png)

## 7. Test outcomes and limitations

The local CMake build and CTest suite pass. Tests cover FEA midpoint errors and boundary values, serial/parallel agreement within `1e-8 °C`, invalid mesh rejection, and analytic-vs-finite-difference gradients. STL and CSV example files were also run through the import validators.

Outstanding assignment-critical work: implement a responsive interface with progress/cancellation/export; interactive STL and 3D temperature point-cloud views; node inspection and temperature legend; Windows 11 `Launch.exe` packaging and clean-machine validation; editor one-action run configuration; record actual Windows machine metadata and launch-to-ready time; produce timestamped demo evidence; and create the required handwritten (maximum two-page) appendix by writing/scanning the derivation. No Windows launcher or real hardware integration is included in this prototype.
