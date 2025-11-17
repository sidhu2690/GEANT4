#include "generator.hh"
#include "Randomize.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include "G4SystemOfUnits.hh"
#include <cmath>

MyPrimaryGenerator::MyPrimaryGenerator() {
    fParticleGun = new G4ParticleGun(1);
}

MyPrimaryGenerator::~MyPrimaryGenerator() {
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event* anEvent) {
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* electron = particleTable->FindParticle("e-");

    for (int i = 0; i < 1; i++) {
        // Position: at origin (0, 0, 0)
        G4double x = 0.0 * cm;
        G4double y = 0.0 * cm;
        G4double z = 0.0 * cm;

        // Fixed transverse momentum
        G4double pT = 10.0 * GeV;  // pT = 10 GeV
        
        // Random pseudorapidity between 1.5 and 3.0
        G4double eta = G4RandFlat::shoot(1.5, 3.0);  // Uniform in [1.5, 3.0]
        
        // Calculate theta from pseudorapidity
        G4double theta = 2.0 * std::atan(std::exp(-eta));
        
        // Random azimuthal angle
        G4double phi = G4RandFlat::shoot(0., 2. * CLHEP::pi);
        
        // Calculate momentum components
        G4double px   = pT * std::cos(phi);
        G4double py   = pT * std::sin(phi);
        G4double pz   = pT * std::sinh(eta);
        G4double pTot = pT * std::cosh(eta);

        // Configure particle gun
        fParticleGun->SetParticleDefinition(electron);
        fParticleGun->SetParticlePosition(G4ThreeVector(x, y, z));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz).unit());
        fParticleGun->SetParticleMomentum(pTot);

        // Fire
        fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}
