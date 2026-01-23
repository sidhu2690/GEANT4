#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include <vector>

// Structure to hold particle generation information from file
struct ParticleGenInfo {
    G4int eventID;      // Evt#
    G4int cumTr;        // Cum_Tr#
    G4int pdgID;        // PDG_ID
    G4double pt;        // Pt
    G4double phi;       // Phi
    G4double theta;     // Theta
    G4double eta;       // Eta
};

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
    MyPrimaryGenerator();
    virtual ~MyPrimaryGenerator();
    
    virtual void GeneratePrimaries(G4Event* anEvent);
    
private:
    G4ParticleGun* fParticleGun;
    std::vector<ParticleGenInfo> fParticleData;
    G4int fCurrentIndex;
    
    void ReadParticleFile(const G4String& filename);
};

#endif