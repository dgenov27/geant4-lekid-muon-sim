# Geant4 LEKID Muon Simulation

Geant4 Monte Carlo simulation of cosmic-ray muons traversing a three-layer LEKID detector stack. This repository contains the implementation used to study spatial localization of ionizing radiation events and their potential application to mitigating correlated error bursts in superconducting quantum processors.

The tagged release `v1.0.0` corresponds to the version used in the associated manuscript.

## Scientific Overview

This simulation models cosmic-ray muon tracks incident on a multilayer LEKID stack, energy deposition within superconducting thin-film layers, geometric extrapolation between detector layers, multiple scattering uncertainty using the Highland approximation, and pixel-level hit registration with spatial deflection metrics.

Primary outputs quantify track deflection between layers, L2 position prediction residuals, spatial uncertainty estimates, and per-event pixel localization. The geometry is fully configurable and supports parameter sweeps for detector thicknesses, gap spacing, and chip dimensions.

## Repository Structure

```
include/              Detector geometry and action classes
src/                  Geant4 implementation files
CMakeLists.txt        Build configuration
README.md             Project documentation
```

## Requirements

- Geant4 (tested with 11.x series)
- CMake ≥ 3.16
- C++17 compatible compiler
- Windows (Visual Studio x64) or Linux/macOS

## Build Instructions

### Windows (Visual Studio + CMake)

1. Install Geant4 (x64).
2. Locate your `Geant4_DIR`, for example:

   ```
   C:/Geant4/11.2.0/lib/Geant4-11.2.0
   ```

3. Open **x64 Native Tools Command Prompt for Visual Studio**.
4. Configure and build:

   ```bat
   cd <path-to-repository>
   cmake -S . -B build -DGeant4_DIR="C:/path/to/Geant4/lib/Geant4-<ver>"
   cmake --build build --config Release -j
   ```

5. Run:

   ```bat
   cd build
   ./lekid_deflection_sim.exe
   ```

### Linux / macOS

```bash
mkdir build
cd build
cmake .. -DGeant4_DIR=/path/to/Geant4/lib/Geant4-<ver>
make -j
./lekid_deflection_sim
```

## Output Files

The simulation generates:

- `deflection_results.csv` — per-event layer positions, pixel indices, and straight-line residuals.
- `l2_uncertainty.csv` — L2 straight-line prediction and multiple scattering uncertainty estimates.

Output files are written to the runtime directory.

## Simulation Configuration

### Event Count

Edit `vis.mac`:

```
/run/beamOn <N>
```

### Geometry Parameters

Default geometry is defined in:

```
include/GeometryConfig.hh
```

Configurable parameters include:

- Layer thicknesses:
  - `nbtiN_thick`
  - `al_thick`
  - `al2o3_thick`
  - `sin_thick`
- Inductor dimensions:
  - `al_length`
  - `al_width`
- Inter-layer gaps:
  - `gStack.gap12`
  - `gStack.gap23`
- Chip lateral dimension:
  - `chipXY`

## Reproducibility

The tagged release `v1.0.0` represents the exact implementation used for the manuscript results. 

## License

This project is licensed under the MIT License.
