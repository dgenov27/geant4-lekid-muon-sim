# GEANT4 LEKID — Clean Build (v2)

## Build (Windows with Visual Studio + CMake)
1. Ensure Geant4 is installed (x64) and note your `Geant4_DIR` (e.g. `C:/Geant4/11.2.0/lib/Geant4-11.2.0`).
2. Open **x64 Native Tools Command Prompt for VS**.
3. Configure & build:
   ```bat
   cd <path-to-this-folder>
   cmake -S . -B build -DGeant4_DIR="C:/path/to/Geant4/lib/Geant4-<ver>"
   cmake --build build --config Release -j
   ```
4. Run:
   ```bat
   cd build
   ./lekid_deflection_sim.exe
   ```

## Build (Linux/macOS)
```bash
mkdir build && cd build
cmake .. -DGeant4_DIR=/path/to/Geant4/lib/Geant4-<ver>
make -j
./lekid_deflection_sim
```

## Outputs
- `deflection_results.csv` — per-event positions/pixels/residual
- `l2_uncertainty.csv` — L2 straight-line prediction + Highland MS uncertainty

## Adjust event count
Edit `vis.mac` last line: `/run/beamOn <N>`.

## Adjust geometry
Edit defaults in `GeometryConfig.hh`:
- `gStack.layer[i].{nbtiN_thick, al_thick, al_length, al_width, al2o3_thick, sin_thick, use_*}`
- gaps: `gStack.gap12`, `gStack.gap23`
- chip size: `gStack.layer[i].chipXY`
