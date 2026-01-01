#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
    MyPrimaryGenerator();
    virtual ~MyPrimaryGenerator();
    
    virtual void GeneratePrimaries(G4Event* anEvent);
    
private:
    G4ParticleGun* fParticleGun;
};

#endif
