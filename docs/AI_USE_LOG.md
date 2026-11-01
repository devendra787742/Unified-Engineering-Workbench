# AI-use and ownership log (complete honestly before submission)

- Tool/model: OpenAI ChatGPT, GPT-6.
- Assistance: initial project decomposition, C++17 prototype scaffolding, thermal-FEA derivation/check calculations, test design and documentation drafting.
- Candidate edits: **Record your actual edits here.**
- Validation: **Record actual compiler, OS, commands, test outputs and dates here.**
- Incorrect/unsuitable suggestion challenge: A first-pass manual midpoint-error calculation used the wrong coefficient for the quadratic exact solution. Rechecking the specified expression `q/(2k)` gives midpoint errors `125/N^2` C, not `250/N^2` C. The implementation and test expectations use the corrected coefficient. Candidate should independently reproduce this check.
- Ownership: This is an AI-assisted prototype. Candidate must identify which code was retained, changed or independently written. Do not claim authorship of unmodified generated sections.


## Desktop integration assistance (2026-11-02)

- Assistance: added an Electron main-process/preload bridge design and frontend controls for invoking the existing C++17 CLI; added Windows packaging instructions and documentation.
- Files added/updated: `desktop/main.js`, `desktop/preload.js`, `package.json`, `scripts/build_desktop_windows.bat`, `frontend/index.html`, `frontend/app.js`, `frontend/styles.css`, `.gitignore`, `README.md`, `SUBMISSION_STATUS.md`, and desktop integration documentation.
- Validation performed in the authoring environment: Node.js syntax checks passed for `frontend/app.js`, `desktop/main.js`, and `desktop/preload.js`; CMake configured and built the C++17 CLI with GNU C++ on Linux; CTest passed 1/1; manual CLI smoke tests passed for STL import (12 triangles), CSV import (5 nodes), FEA (40 elements), hardware mocks, backpropagation, and batch benchmark.
- Not performed: Windows MSVC build of the Electron-integrated package, `npm install`, Windows installer generation, Windows GUI launch, clean-machine test, or candidate handwritten derivation/video evidence.
- Candidate action: review and understand the bridge, record personal edits and local Windows test outputs, and complete the ownership statement before submission. This log does not replace the candidate's own disclosure.
