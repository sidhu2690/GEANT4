#include "generator.hh"
#include "TrackInformation.hh"  
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4AnalysisManager.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include <cmath>

MyPrimaryGenerator::MyPrimaryGenerator()
    : G4VUserPrimaryGeneratorAction(),
      fParticleGun(nullptr)
{
    fParticleGun = new G4ParticleGun(1);
    G4cout << "MyPrimaryGenerator: Initialized" << G4endl;
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event* anEvent)
{
    G4int eventID = anEvent->GetEventID();
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();

    // Get particle definition - mu+ (PDG -13)
    G4ParticleDefinition* particle = particleTable->FindParticle(-13);
    if (!particle) {
        G4cout << "ERROR: Unknown PDG ID, cannot generate primaries!" << G4endl;
        return;
    }
    
    G4cout << "Event " << eventID << ": Generating primary particles" << G4endl;
    
    // Generate 1 particle per event (change if needed)
    G4int nParticlesPerEvent = 1;
    
    for (int i = 0; i < nParticlesPerEvent; i++) {
        
        // ==========================================
        // POSITION: at origin (0, 0, 0)
        // ==========================================
        G4double x = 0.0 * cm;
        G4double y = 0.0 * cm;
        G4double z = 0.0 * cm;

        // ==========================================
        // KINEMATIC PARAMETERS
        // ==========================================
        G4double pT = 10.0 * GeV;  // FIXED pT at 5 GeV
        
        // Random pseudorapidity between 1.5 and 3.0
        G4double eta = G4RandFlat::shoot(1.5, 3.0);  
        
        // Random azimuthal angle (0 to 2π)
        G4double phi = G4RandFlat::shoot(0., 2. * CLHEP::pi);
        
        // ==========================================
        // CALCULATED VALUES
        // ==========================================
        // Calculate theta from pseudorapidity
        G4double theta = 2.0 * std::atan(std::exp(-eta));
        
        // Calculate momentum components
        G4double px = pT * std::cos(phi);
        G4double py = pT * std::sin(phi);
        G4double pz = pT * std::sinh(eta);
        G4double pTot = std::sqrt(px*px + py*py + pz*pz);
        
        // Calculate total energy
        G4double mass = particle->GetPDGMass();
        G4double energy = std::sqrt(pTot * pTot + mass * mass);
        
        // ==========================================
        // CONFIGURE PARTICLE GUN
        // ==========================================
        fParticleGun->SetParticleDefinition(particle);
        fParticleGun->SetParticlePosition(G4ThreeVector(x, y, z));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz).unit());
        fParticleGun->SetParticleMomentum(pTot);
        
        // Fire the particle
        fParticleGun->GeneratePrimaryVertex(anEvent);
        
        // ==========================================
        // ATTACH USER INFORMATION
        // ==========================================
        G4PrimaryVertex* vertex = anEvent->GetPrimaryVertex(anEvent->GetNumberOfPrimaryVertex() - 1);
        if (vertex) {
            G4PrimaryParticle* primary = vertex->GetPrimary();
            if (primary) {
                primary->SetUserInformation(new PrimaryParticleInformation(i));
            }
        }
        
        // ==========================================
        // FILL NTUPLE
        // ==========================================
        man->FillNtupleIColumn(0, 0, eventID);
        man->FillNtupleIColumn(0, 1, -13);                    // PDG ID (mu+)
        man->FillNtupleDColumn(0, 2, px / MeV);
        man->FillNtupleDColumn(0, 3, py / MeV);
        man->FillNtupleDColumn(0, 4, pz / MeV);
        man->FillNtupleDColumn(0, 5, pTot / MeV);
        man->FillNtupleDColumn(0, 6, energy / MeV);
        man->FillNtupleDColumn(0, 7, eta);
        man->FillNtupleDColumn(0, 8, phi);
        man->FillNtupleDColumn(0, 9, theta);
        man->FillNtupleIColumn(0, 10, i);                     // cumTr
        man->FillNtupleDColumn(0, 11, pT / MeV);
        man->FillNtupleDColumn(0, 12, 1.0);                   // charge
        man->FillNtupleIColumn(0, 13, 0);                     // layer
        man->FillNtupleDColumn(0, 14, 0.0);                   // energy_deposited
        man->FillNtupleDColumn(0, 15, 0.0);                   // xi
        man->FillNtupleDColumn(0, 16, 0.0);                   // yi

        man->AddNtupleRow(0);
        
        G4cout << "  Particle " << i << ": mu+ | pT=" << pT/GeV << " GeV | eta=" << eta 
               << " | phi=" << phi << " rad | pTot=" << pTot/GeV << " GeV" << G4endl;
    }
}
