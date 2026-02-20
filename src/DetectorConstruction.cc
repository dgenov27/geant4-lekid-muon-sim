#include "DetectorConstruction.hh"
#include "GeometryConfig.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4ThreeVector.hh"
#include "G4Colour.hh"


DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  EnsureCustomMaterials();
  auto* nist = G4NistManager::Instance();

  // World box with generous margins
  const auto& fs0 = gStack.layer[0];
  G4double estHeight = 3*fs0.siThickness + gStack.gap12 + gStack.gap23 + 20*mm;
  auto* worldS  = new G4Box("World", 0.6*fs0.chipXY, 0.6*fs0.chipXY, estHeight*0.5);
  auto* worldLV = new G4LogicalVolume(worldS, nist->FindOrBuildMaterial("G4_Galactic"), "World");
  // Make world invisible so your detector stands out
  worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());
  auto* worldPV = new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), worldLV, "World", nullptr, false, 0, true);

  // Place three layered wafers: centers separated by substrate gaps
  G4double tSi1 = gStack.layer[0].siThickness;
  G4double tSi2 = gStack.layer[1].siThickness;
  G4double z1 = 0.0;
  G4double z2 = z1 + tSi1 + gStack.gap12;
  G4double z3 = z2 + tSi2 + gStack.gap23;

  BuildLayerStack("Layer1", gStack.layer[0], worldLV, G4ThreeVector(0,0,z1));
  BuildLayerStack("Layer2", gStack.layer[1], worldLV, G4ThreeVector(0,0,z2));
  BuildLayerStack("Layer3", gStack.layer[2], worldLV, G4ThreeVector(0,0,z3));

  G4cout << ">>> DetectorConstruction COMPLETE: z1=" << z1 / mm
      << " mm, z2=" << z2 / mm << " mm, z3=" << z3 / mm << " mm" << G4endl;

  return worldPV;
}
