#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "G4PhysListFactory.hh"
#include "G4EmStandardPhysics_option4.hh"

int main(int argc, char** argv) {
  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);
  runManager->SetUserInitialization(new DetectorConstruction());

  G4PhysListFactory physFactory;
  auto* phys = physFactory.GetReferencePhysList("FTFP_BERT");
  phys->ReplacePhysics(new G4EmStandardPhysics_option4());
  runManager->SetUserInitialization(phys);

  runManager->SetUserInitialization(new ActionInitialization());

  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  G4UImanager* ui = G4UImanager::GetUIpointer();
  if (argc == 1) {
    G4UIExecutive* uiExec = new G4UIExecutive(argc, argv);
    ui->ApplyCommand("/control/execute vis.mac");
    uiExec->SessionStart();
    delete uiExec;
  } else {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    ui->ApplyCommand(command + fileName);
  }

  delete visManager;
  delete runManager;
  return 0;
}
