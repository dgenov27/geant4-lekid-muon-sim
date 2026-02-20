#include "PrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include "G4PhysicalConstants.hh" 
#include "GeometryConfig.hh"
#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction() {
  fParticleGun = new G4ParticleGun(1);
  auto* particleTable = G4ParticleTable::GetParticleTable();
  auto* mu = particleTable->FindParticle("mu-");
  fParticleGun->SetParticleDefinition(mu);
  fParticleGun->SetParticleEnergy(6.0*GeV);
  // NOTE: position & direction are randomized per event in GeneratePrimaries().


}
PrimaryGeneratorAction::~PrimaryGeneratorAction() { delete fParticleGun; }
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    
 
        // --- Random starting position across the chip (uniform in x,y) ---
        double halfSize = gStack.layer[0].chipXY * 0.5;   // chip half-width in mm
        G4double x0 = (2 * G4UniformRand() - 1.0) * halfSize;
        G4double y0 = (2 * G4UniformRand() - 1.0) * halfSize;

        // Start ABOVE the stack and shoot downward (-z): L3 -> L2 -> L1
        const G4double z0 = 12.0 * mm;  // safely inside world, above L3 (~10.5 mm)
        fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));

        
        const double n = 2.0;       // larger n -> more vertical
        const double thetaMax = 60. * deg;   // cap max zenith angle
        const double cmax = std::cos(thetaMax);
        const double cmax_pow = std::pow(cmax, n + 1);

        // sample cos(theta) with pdf ? c^n on [cmax, 1]
        const double u = G4UniformRand();
        const double c = std::pow(u * (1.0 - cmax_pow) + cmax_pow, 1.0 / (n + 1));
        const double theta = std::acos(c);
        const double phi = twopi * G4UniformRand();

        const G4ThreeVector dir(
            std::sin(theta) * std::cos(phi),
            std::sin(theta) * std::sin(phi),
            -std::cos(theta)   // minus: downward (-z)
        );
        fParticleGun->SetParticleMomentumDirection(dir);



        // Fire the primary for this event
        fParticleGun->GeneratePrimaryVertex(anEvent);
    }


