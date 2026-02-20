# Geant4 LEKID Muon Simulation

Geant4 Monte Carlo simulation of cosmic-ray muons traversing a three-layer LEKID detector stack. This repository contains the implementation used to study spatial localization of ionizing radiation events and their potential application to mitigating correlated error bursts in superconducting quantum processors.

The tagged release `v1.0.0` corresponds to the version used in the associated manuscript.

---

## Scientific Overview

This simulation models:

- Cosmic-ray muon tracks incident on a multilayer LEKID stack  
- Energy deposition within superconducting thin-film layers  
- Geometric extrapolation between detector layers  
- Multiple scattering uncertainty using the Highland approximation  
- Pixel-level hit registration and spatial deflection metrics  

Primary outputs quantify:

- Track deflection between layers  
- L2 position prediction residuals  
- Spatial uncertainty estimates  
- Per-event pixel localization  

The geometry is fully configurable and supports parameter sweeps for detector thicknesses, gap spacing, and chip dimensions.

---

## Repository Structure

```
include/                         Detector geometry and action classes
src/                             Geant4 implementation files
data/                            Simulation output datasets
  └── deflection_results.csv     Full dataset used in manuscript analysis
CMakeLists.txt                   Build configuration
README.md                        Project documentation
```

---

## Included Dataset

The file:

`data/deflection_results.csv`

contains the full simulation output used for the manuscript analysis.  
Each row corresponds to a simulated muon event and includes:

- Layer hit coordinates  
- Pixel indices  
- Extrapolated residuals  
- Deflection metrics  

This dataset allows readers to inspect the results directly without rebuilding and rerunning the simulation.


---

## Requirements

- Geant4 (tested with 11.x series)
- CMake ≥ 3.16
- C++17 compatible compiler

---

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

---

### Linux / macOS

```bash
mkdir build
cd build
cmake .. -DGeant4_DIR=/path/to/Geant4/lib/Geant4-<ver>
make -j
./lekid_deflection_sim
```

---

## Output Files (Generated at Runtime)

When the simulation is executed, it produces:

- `deflection_results.csv` — per-event layer positions, pixel indices, and straight-line residuals.
- `l2_uncertainty.csv` — L2 straight-line prediction and multiple scattering uncertainty estimates.

These files are written to the runtime directory.

---

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

---

## Reproducibility

The tagged release `v1.0.0` represents the exact implementation and dataset used for the manuscript results.

Tagged releases preserve archival versions for reproducibility.

---

## License

This project is licensed under the MIT License.
