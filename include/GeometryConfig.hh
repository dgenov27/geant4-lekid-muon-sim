#pragma once
#include "globals.hh"
#include <string>
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"




class G4LogicalVolume;
class G4VPhysicalVolume;


// Per-layer LEKID film stack (top-down on Si substrate)
struct FilmStack {
  G4double chipXY = 55*mm;        // chip size (square)
  G4double siThickness = 0.35*mm; // Si substrate
  // Films (top-down)
  G4double nbtiN_thick = 500*nm;  // NbTiN (full coverage)
  G4double al_thick    = 40*nm;   // Al absorber thickness
  G4double al_length   = 1.2*mm;  // Al strip length
  G4double al_width    = 0.05*mm; // Al strip width (50 µm)
  G4double al2o3_thick = 3*nm;    // native Al2O3 over Al
  G4double sin_thick   = 0*nm;    // optional SiN (set >0 and enable)
  // Toggles
  G4bool use_nbtiN = true;
  G4bool use_al    = true;
  G4bool use_al2o3 = true;
  G4bool use_sin   = false;
};

struct StackConfig {
  FilmStack layer[3];
  G4double gap12 = 5*mm; // substrate-to-substrate distance L1->L2
  G4double gap23 = 5*mm; // L2->L3
};

struct BeamConfig { G4double p_MeV; G4double beta; };

extern StackConfig gStack;  // global geometry config
extern BeamConfig  gBeam;   // global beam config (for MS uncertainty)

// Ensure custom film materials exist (NbTiN approx, Si3N4, Al2O3)
void EnsureCustomMaterials();

// Highland RMS scattering angle for thickness in radiation lengths (t)
G4double HighlandTheta0(G4double p_MeV, G4double beta, G4double t);

// Build a layered LEKID at 'center' inside 'worldLV'. Returns the mother PV.
G4VPhysicalVolume* BuildLayerStack(const std::string& name, const FilmStack& fs,
                                   G4LogicalVolume* worldLV, const G4ThreeVector& center);
