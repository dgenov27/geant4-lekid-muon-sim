#ifndef SteppingAction_h
#define SteppingAction_h 1
#include "G4UserSteppingAction.hh"
class G4Step;
class SteppingAction : public G4UserSteppingAction {
public:
  SteppingAction();
  ~SteppingAction() override;
  void UserSteppingAction(const G4Step*) override;
};
#endif
