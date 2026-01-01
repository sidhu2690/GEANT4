#include "generator.hh"           // MUST BE FIRST - includes class definition
#include "TrackInformation.hh"  
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "Randomize.hh"
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>

MyPrimaryGenerator::MyPrimaryGenerator() : fCurrentIndex(0) {
    fParticleGun = new G4ParticleGun(1);
    
    // Print random engine information
    CLHEP::HepRandomEngine* engine = CLHEP::HepRandom::getTheEngine();
    G4cout << "========================================" << G4endl;
    G4cout << "Random Engine Information:" << G4endl;
    G4cout << "Engine name: " << engine->name() << G4endl;
    G4cout << "========================================" << G4endl;
    
    // Read the particle file
    ReadParticleFile("generated_data.txt");  
}

MyPrimaryGenerator::~MyPrimaryGenerator() {
    delete fParticleGun;
}

void MyPrimaryGenerator::ReadParticleFile(const G4String& filename) {
    G4cout << "======================================" << G4endl;
    G4cout << "Attempting to open file: " << filename << G4endl;
    
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        G4cout << "ERROR: Cannot open particle file: " << filename << G4endl;
        return;
    }
    
    G4cout << "File opened successfully!" << G4endl;
    
    std::string line;
    // Skip header line
    if (!std::getline(infile, line)) {
        G4cout << "ERROR: Cannot read header line!" << G4endl;
        return;
    }
    G4cout << "Header: " << line << G4endl;
    
    int lineCount = 0;
    while (std::getline(infile, line)) {
        lineCount++;
        std::istringstream iss(line);
        ParticleGenInfo particle;
        
        // Read 7 columns: Evt#, Cum_Tr#, PDG_ID, Pt, Phi, Theta, Eta
        if (iss >> particle.eventID >> particle.cumTr >> particle.pdgID 
                >> particle.pt >> particle.phi >> particle.theta >> particle.eta) {
            
            // Skip particles with invalid values (inf, nan, zero pt)
            if (std::isfinite(particle.eta) && std::isfinite(particle.pt) && 
                std::isfinite(particle.phi) && std::isfinite(particle.theta) &&
                particle.pt > 0.0) {
                
                fParticleData.push_back(particle);
                if (lineCount <= 3) {
                    G4cout << "Parsed line " << lineCount << ": Event=" << particle.eventID 
                           << " CumTr=" << particle.cumTr 
                           << " PDG=" << particle.pdgID
                           << " pT=" << particle.pt 
                           << " Phi=" << particle.phi 
                           << " Theta=" << particle.theta
                           << " Eta=" << particle.eta << G4endl;
                }
            } else {
                if (lineCount <= 20) {  // Only show first 20 warnings
                    G4cout << "WARNING: Skipping line " << lineCount << " with invalid values (inf/nan/zero): " 
                           << line << G4endl;
                }
            }
        } else {
            if (lineCount <= 20) {  // Only show first 20 warnings
                G4cout << "WARNING: Failed to parse line " << lineCount << ": " << line << G4endl;
            }
        }
    }
    
    infile.close();
    G4cout << "Loaded " << fParticleData.size() << " particles from " << filename << G4endl;
    G4cout << "Using eta and pT values from file" << G4endl;
    G4cout << "======================================" << G4endl;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event* anEvent) {
    if (fParticleData.empty()) {
        G4cout << "ERROR: No particle data available!" << G4endl;
        return;
    }
    
    G4int eventID = anEvent->GetEventID();
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    
    // Get seed and generate random number for this event
    long seed = CLHEP::HepRandom::getTheSeed();
    long randomNumber = static_cast<long>(G4UniformRand() * 1e12);
    
    // Find all particles for this event
    std::vector<ParticleGenInfo> eventParticles;
    for (const auto& particle : fParticleData) {
        if (particle.eventID == eventID) {
            eventParticles.push_back(particle);
        }
    }
    
    if (eventParticles.empty()) {
        G4cout << "WARNING: No particles found for event " << eventID << G4endl;
        return;
    }
    
    G4cout << "Event " << eventID << ": Generating " << eventParticles.size() << " particles" << G4endl;
    
    // Generate all particles for this event
    for (const auto& genInfo : eventParticles) {
        // Get particle definition from file
        G4ParticleDefinition* particle = particleTable->FindParticle(genInfo.pdgID);
        if (!particle) {
            G4cout << "WARNING: Unknown PDG ID " << genInfo.pdgID << ", skipping..." << G4endl;
            continue;
        }
        
        // ==========================================
        // VALUES FROM FILE
        // ==========================================
        G4double eta = std::fabs(genInfo.eta);         // Take absolute value of eta (detector on one side only)
        G4double pT = genInfo.pt * MeV;                // pT from file (convert to MeV)
        G4double phi = genInfo.phi;                    // phi from file (already in radians)
        
        // ==========================================
        // CALCULATED VALUES
        // ==========================================
        // Calculate theta from eta: theta = 2 * atan(exp(-eta))
        G4double theta = 2.0 * std::atan(std::exp(-eta));
        
        // Calculate momentum components
        // Using eta formulation (more accurate in HEP):
        G4double px = pT * std::cos(phi);
        G4double py = pT * std::sin(phi);
        G4double pz = pT * std::sinh(eta);  // pz = pT * sinh(eta)
        G4double pTot = std::sqrt(px*px + py*py + pz*pz);
        
        // Position: FIXED at origin
        G4double x = 0.0 * cm;
        G4double y = 0.0 * cm;
        G4double z = 0.0 * cm;
        
        // Calculate energy (E = sqrt(p^2 + m^2))
        G4double mass = particle->GetPDGMass();
        G4double energy = std::sqrt(pTot * pTot + mass * mass);
        
        // Get particle charge
        G4double charge = particle->GetPDGCharge() / CLHEP::eplus;  // Convert to units of e
        
        // Configure particle gun
        fParticleGun->SetParticleDefinition(particle);
        fParticleGun->SetParticlePosition(G4ThreeVector(x, y, z));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz).unit());
        fParticleGun->SetParticleMomentum(pTot);
        
        // Fire the particle
        fParticleGun->GeneratePrimaryVertex(anEvent);
        
        // **KEY STEP: Attach cumTr to the primary particle**
        // Get the vertex that was just created
        G4PrimaryVertex* vertex = anEvent->GetPrimaryVertex(anEvent->GetNumberOfPrimaryVertex() - 1);
        if (vertex) {
            G4PrimaryParticle* primary = vertex->GetPrimary();
            if (primary) {
                // Use PrimaryParticleInformation (not TrackInformation)
                primary->SetUserInformation(new PrimaryParticleInformation(genInfo.cumTr));
            }
        }
        
        // Store generator-level information in ntuple 0
        man->FillNtupleIColumn(0, 0, eventID);
        man->FillNtupleIColumn(0, 1, genInfo.pdgID);         // PDG ID from file
        man->FillNtupleDColumn(0, 2, px / MeV);
        man->FillNtupleDColumn(0, 3, py / MeV);
        man->FillNtupleDColumn(0, 4, pz / MeV);
        man->FillNtupleDColumn(0, 5, pTot / MeV);        
        man->FillNtupleDColumn(0, 6, energy / MeV);
        man->FillNtupleDColumn(0, 7, eta);                   // eta from file
        man->FillNtupleDColumn(0, 8, phi);                   // phi from file (in radians)
        man->FillNtupleDColumn(0, 9, theta);                 // Calculated from eta
        man->FillNtupleIColumn(0, 10, genInfo.cumTr);        // Cum_Tr# from file
        man->FillNtupleDColumn(0, 11, pT / MeV);             // pT from file
        man->FillNtupleDColumn(0, 12, charge);               // charge from particle definition
        man->FillNtupleIColumn(0, 13, 0);                    // layer (0 for vertex)
        man->FillNtupleDColumn(0, 14, 0.0);                  // energy_deposited (0 for vertex)
        man->FillNtupleDColumn(0, 15, 0.0);                  // xi (not available, set to 0)
        man->FillNtupleDColumn(0, 16, 0.0);                  // yi (not available, set to 0)
        man->FillNtupleIColumn(0, 17, static_cast<G4int>(seed));           // seed
        man->FillNtupleDColumn(0, 18, static_cast<G4double>(randomNumber)); // random number

        man->AddNtupleRow(0);
    }
}