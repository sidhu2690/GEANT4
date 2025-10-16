#include "generator.hh"
#include "Randomize.hh"               // for G4RandFlat::shoot
#include "CLHEP/Units/PhysicalConstants.h" // for CLHEP::pi
#include "G4SystemOfUnits.hh"

MyPrimaryGenerator::MyPrimaryGenerator() {
    fParticleGun = new G4ParticleGun(1); // single particle per gun call
}

MyPrimaryGenerator::~MyPrimaryGenerator() {
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event* anEvent) {
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* muon = particleTable->FindParticle("e-"); // positive muon

    // ---- Geometry info ----
    // Source at center: z = 0
    // First detector layer starts around z = 322 cm
    // Max detector radius: 256 cm
    // For particles to hit detectors, limit the angular spread
    G4double thetaMax = 0.80 * std::atan(0.4776); // 

    for (int i = 0; i < 1; i++) {
        // Position: x, y, z =(0, 0, 0) cm
        G4double x = G4RandFlat::shoot(0.0) * cm;
        G4double y = G4RandFlat::shoot(0.0) * cm;
        G4double z = 0.0 * cm;  // Source at center

        // Momentum magnitude (10 GeV)
        G4double pTot = 10.0 * GeV;

        // Sample theta within acceptance cone
        // Use cos(theta) to ensure uniform solid-angle distribution
        G4double cosThetaMin = std::cos(thetaMax);
        G4double cosTheta    = 1.0 - (1.0 - cosThetaMin) * G4RandFlat::shoot(0., 1.);
        G4double theta       = std::acos(cosTheta);

        // Azimuthal angle
        G4double phi = G4RandFlat::shoot(0., 2. * CLHEP::pi);

        // Momentum components (pointing in +z direction generally)
        G4double px = pTot * std::sin(theta) * std::cos(phi);
        G4double py = pTot * std::sin(theta) * std::sin(phi);
        G4double pz = pTot * std::cos(theta);

        // Configure particle gun
        fParticleGun->SetParticleDefinition(muon);
        fParticleGun->SetParticlePosition(G4ThreeVector(x, y, z));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz).unit());
        fParticleGun->SetParticleMomentum(pTot);

        // Fire
        fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}
