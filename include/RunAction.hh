#ifndef RunAction_h
#define RunAction_h 1
#include "G4UserRunAction.hh"
#include "G4ThreeVector.hh"
#include <map>
#include <string>

struct HitPos { bool has=false; G4ThreeVector pos; };
class G4Run;

class RunAction : public G4UserRunAction {
public:
  RunAction();
  ~RunAction() override;
  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction(const G4Run*) override;
  void AddEdep(double edep) { totalEdep += edep; }
  void SetLayerHit(long eventID, const std::string& layer, const G4ThreeVector& pos);
  void RecordLayer2Actual(long eventID, const G4ThreeVector& pos);
private:
  double totalEdep = 0.0;
  std::map<long, std::map<std::string, HitPos>> eventHits;
};
#endif
