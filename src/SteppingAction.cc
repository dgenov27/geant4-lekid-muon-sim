#include "SteppingAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "RunAction.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"  // only needed if you print mm; harmless to keep


SteppingAction::SteppingAction() : G4UserSteppingAction() {
    G4cout << ">>> SteppingAction CONSTRUCTED" << G4endl;
}
SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step) {
    G4cout << "[DEBUG] stepping..." << G4endl;
    // Post step point & true boundary check (entrance to a new volume)
    const auto post = step->GetPostStepPoint();
    if (!post) return;
    if (post->GetStepStatus() != fGeomBoundary) return;

    // Only the primary should define the "first hit"
    const auto track = step->GetTrack();
    if (!track || track->GetParentID() != 0) return;

    // Volumes on both sides of the boundary
    const auto pre = step->GetPreStepPoint();
    auto* preVol = pre ? pre->GetPhysicalVolume() : nullptr;
    auto* postVol = post ? post->GetPhysicalVolume() : nullptr;
    if (!postVol) return;

    const std::string preName = preVol ? preVol->GetName() : "";
    const std::string postName = postVol ? postVol->GetName() : "";

    // Log the actual boundary we crossed
    G4cout << "[BOUNDARY] " << (preName.empty() ? "NULL" : preName)
        << " -> " << (postName.empty() ? "NULL" : postName)
        << "  z=" << post->GetPosition().z() / mm << " mm" << G4endl;


    // Current event ID
    const G4Event* ev = G4EventManager::GetEventManager()->GetConstCurrentEvent();
    const long eventID = ev ? ev->GetEventID() : -1;

    // RunAction access (GetUserRunAction returns a const base ptr)
    auto* rm = G4RunManager::GetRunManager();
    const G4UserRunAction* baseUA = rm ? rm->GetUserRunAction() : nullptr;
    if (!baseUA) { G4cout << "[DEBUG] No RunAction yet\n"; return; }

    // cast to your concrete RunAction and drop constness
    auto* runAct = const_cast<RunAction*>(dynamic_cast<const RunAction*>(baseUA));
    if (!runAct) { G4cout << "[DEBUG] RunAction cast failed\n"; return; }


    // (optional) keep your energy accumulation
    runAct->AddEdep(step->GetTotalEnergyDeposit());

    const auto pos = post->GetPosition(); // G4ThreeVector

    // Only accept FIRST entry into actual films/Si (not mother volumes).
    // For downward beam, this makes the topmost film the layer's reference plane.
    const std::string& name = postName;

    if (name == "Layer1_AlStrip_phys" || name == "Layer1_Al2O3_phys" ||
        name == "Layer1_SiN_phys" || name == "Layer1_NbTiN_phys" ||
        name == "Layer1_Si_phys") {
        runAct->SetLayerHit(eventID, "Layer1", pos);
        G4cout << "[evt " << eventID << "] ENTER L1 (film/Si) at " << pos / mm << " mm" << G4endl;
    }
    else if (name == "Layer2_AlStrip_phys" || name == "Layer2_Al2O3_phys" ||
        name == "Layer2_SiN_phys" || name == "Layer2_NbTiN_phys" ||
        name == "Layer2_Si_phys") {
        runAct->RecordLayer2Actual(eventID, pos);
        G4cout << "[evt " << eventID << "] ENTER L2 (film/Si) at " << pos / mm << " mm" << G4endl;
    }
    else if (name == "Layer3_AlStrip_phys" || name == "Layer3_Al2O3_phys" ||
        name == "Layer3_SiN_phys" || name == "Layer3_NbTiN_phys" ||
        name == "Layer3_Si_phys") {
        runAct->SetLayerHit(eventID, "Layer3", pos);
        G4cout << "[evt " << eventID << "] ENTER L3 (film/Si) at " << pos / mm << " mm" << G4endl;
    }


}
