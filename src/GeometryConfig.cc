#include "GeometryConfig.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4UserLimits.hh"
#include "G4Colour.hh"
#include "G4ThreeVector.hh"
#include <algorithm>
#include <cmath>

StackConfig gStack;
BeamConfig  gBeam = { 4000*MeV, 1.0 };

void EnsureCustomMaterials() {
  auto* nist = G4NistManager::Instance();
  nist->FindOrBuildMaterial("G4_AIR");
  nist->FindOrBuildMaterial("G4_Galactic");
  nist->FindOrBuildMaterial("G4_Si");
  nist->FindOrBuildMaterial("G4_Al");

  auto* elSi = nist->FindOrBuildElement("Si");
  auto* elAl = nist->FindOrBuildElement("Al");
  auto* elN  = nist->FindOrBuildElement("N");
  auto* elO  = nist->FindOrBuildElement("O");
  auto* elTi = nist->FindOrBuildElement("Ti");
  static G4Element* elNb = nullptr;
  if (!elNb) elNb = new G4Element("Niobium", "Nb", 41., 92.90638*g/mole);

  if (!G4Material::GetMaterial("NbTiNApprox")) {
    auto* m = new G4Material("NbTiNApprox", 8.4*g/cm3, 3);
    m->AddElement(elNb, 1);
    m->AddElement(elTi, 1);
    m->AddElement(elN,  1);
  }
  if (!G4Material::GetMaterial("Si3N4")) {
    auto* m = new G4Material("Si3N4", 3.17*g/cm3, 2);
    m->AddElement(elSi, 3);
    m->AddElement(elN,  4);
  }
  if (!G4Material::GetMaterial("Al2O3_custom")) {
    auto* m = new G4Material("Al2O3_custom", 3.95*g/cm3, 2);
    m->AddElement(elAl, 2);
    m->AddElement(elO,  3);
  }
}

G4double HighlandTheta0(G4double p_MeV, G4double beta, G4double t) {
  t = std::max(t, 1e-12); // guard log(0)
  return (13.6*MeV)/(p_MeV*beta) * std::sqrt(t) * (1.0 + 0.038*std::log(t));
}

G4VPhysicalVolume* BuildLayerStack(const std::string& name, const FilmStack& fs,
                                   G4LogicalVolume* worldLV, const G4ThreeVector& center) {
  // Mother in air (hosts wafer + films)
  G4double topFilms = (fs.use_nbtiN?fs.nbtiN_thick:0) + (fs.use_al?fs.al_thick:0) +
                      (fs.use_al2o3?fs.al2o3_thick:0) + (fs.use_sin?fs.sin_thick:0);
  G4double motherT = fs.siThickness + topFilms + 10*um;

  auto* motherS  = new G4Box((name+"_mother_s").c_str(), fs.chipXY*0.5, fs.chipXY*0.5, motherT*0.5);
  auto* motherLV = new G4LogicalVolume(motherS, G4Material::GetMaterial("G4_AIR"), (name+"_log").c_str());
  auto* motherPV = new G4PVPlacement(nullptr, center, motherLV, (name+"_phys").c_str(), worldLV, false, 0);
  motherLV->SetVisAttributes(G4VisAttributes::GetInvisible());

  G4double z = -motherT*0.5;

  // Si substrate
  auto* siS  = new G4Box((name+"_Si_s").c_str(), fs.chipXY*0.5, fs.chipXY*0.5, fs.siThickness*0.5);
  auto* siLV = new G4LogicalVolume(siS, G4Material::GetMaterial("G4_Si"), (name+"_Si_log").c_str());
  new G4PVPlacement(nullptr, G4ThreeVector(0,0, z + fs.siThickness*0.5), siLV, (name+"_Si_phys").c_str(), motherLV, false, 0);
  z += fs.siThickness;
  siLV->SetUserLimits(new G4UserLimits(1*um));
  {
      auto* va = new G4VisAttributes(G4Colour(0.20, 0.60, 1.00, 0.16)); // light blue, ~16% opaque
      va->SetForceSolid(true);
      va->SetForceAuxEdgeVisible(true); // outlines
      siLV->SetVisAttributes(va);
  }


  // NbTiN full film
  if (fs.use_nbtiN && fs.nbtiN_thick>0) {
    auto* nbS  = new G4Box((name+"_NbTiN_s").c_str(), fs.chipXY*0.5, fs.chipXY*0.5, fs.nbtiN_thick*0.5);
    auto* nbLV = new G4LogicalVolume(nbS, G4Material::GetMaterial("NbTiNApprox"), (name+"_NbTiN_log").c_str());
    new G4PVPlacement(nullptr, G4ThreeVector(0,0, z + fs.nbtiN_thick*0.5), nbLV, (name+"_NbTiN_phys").c_str(), motherLV, false, 0);
    z += fs.nbtiN_thick;
    nbLV->SetUserLimits(new G4UserLimits(5*nm));
    {
        auto* va = new G4VisAttributes(G4Colour(0.35, 0.35, 0.40, 1.00)); // dark grey
        va->SetForceSolid(true);
        va->SetForceAuxEdgeVisible(true);
        nbLV->SetVisAttributes(va);
    }

  }

  // Al absorber strip (patterned, narrow, centered)
  if (fs.use_al && fs.al_thick>0) {
    auto* alS  = new G4Box((name+"_AlStrip_s").c_str(), fs.al_width*0.5, fs.al_length*0.5, fs.al_thick*0.5);
    auto* alLV = new G4LogicalVolume(alS, G4Material::GetMaterial("G4_Al"), (name+"_AlStrip_log").c_str());
    new G4PVPlacement(nullptr, G4ThreeVector(0,0, z + fs.al_thick*0.5), alLV, (name+"_AlStrip_phys").c_str(), motherLV, false, 0);
    z += fs.al_thick;
    alLV->SetUserLimits(new G4UserLimits(2*nm));
    {
        auto* va = new G4VisAttributes(G4Colour(0.95, 0.80, 0.15, 1.00)); // gold
        va->SetForceSolid(true);
        va->SetForceAuxEdgeVisible(true);
        alLV->SetVisAttributes(va);
    }


    if (fs.use_al2o3 && fs.al2o3_thick>0) {
      auto* oxS  = new G4Box((name+"_Al2O3_s").c_str(), fs.al_width*0.5, fs.al_length*0.5, fs.al2o3_thick*0.5);
      auto* oxLV = new G4LogicalVolume(oxS, G4Material::GetMaterial("Al2O3_custom"), (name+"_Al2O3_log").c_str());
      new G4PVPlacement(nullptr, G4ThreeVector(0,0, z + fs.al2o3_thick*0.5), oxLV, (name+"_Al2O3_phys").c_str(), motherLV, false, 0);
      z += fs.al2o3_thick;
      oxLV->SetUserLimits(new G4UserLimits(2*nm));
      {
          auto* va2 = new G4VisAttributes(G4Colour(0.95, 0.55, 0.20, 0.80)); // orange, 80% opaque
          va2->SetForceSolid(true);
          va2->SetForceAuxEdgeVisible(true);
          oxLV->SetVisAttributes(va2);
      }

    }
  }

  // Optional SiN membrane/passivation (over absorber footprint by default)
  if (fs.use_sin && fs.sin_thick>0) {
    G4double sinHalfX = (fs.use_al ? fs.al_width*0.5 : fs.chipXY*0.5);
    G4double sinHalfY = (fs.use_al ? fs.al_length*0.5 : fs.chipXY*0.5);
    auto* sinS  = new G4Box((name+"_SiN_s").c_str(), sinHalfX, sinHalfY, fs.sin_thick*0.5);
    auto* sinLV = new G4LogicalVolume(sinS, G4Material::GetMaterial("Si3N4"), (name+"_SiN_log").c_str());
    new G4PVPlacement(nullptr, G4ThreeVector(0,0, z + fs.sin_thick*0.5), sinLV, (name+"_SiN_phys").c_str(), motherLV, false, 0);
    z += fs.sin_thick;
    sinLV->SetUserLimits(new G4UserLimits(5*nm));
    {
        auto* va = new G4VisAttributes(G4Colour(0.20, 0.85, 0.85, 0.60)); // cyan, 60% opaque
        va->SetForceSolid(true);
        va->SetForceAuxEdgeVisible(true);
        sinLV->SetVisAttributes(va);
    }

  }

  return motherPV;
}
