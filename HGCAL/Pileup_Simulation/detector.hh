#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4ThreeVector.hh"
#include <map>
#include <string>

struct ParticleData {
    G4int eventID;
    G4int trackID;
    G4int layer;
    G4int particleID;
    G4int cumTr;  
    G4double charge;
    G4double energyBefore;
    G4double energyAfter;
    G4double totalEnergyDeposited;
    G4ThreeVector momentumBefore;
    G4ThreeVector momentumAfter;
    G4ThreeVector positionEnter;
    G4ThreeVector positionExit;
    G4double etaEnter;
    G4double phiEnter;
    G4double etaExit;
    G4double phiExit;
    
    ParticleData() : eventID(0), trackID(0), layer(0), particleID(0), cumTr(-1),
                     charge(0.0), energyBefore(0.0), energyAfter(0.0), 
                     totalEnergyDeposited(0.0),
                     etaEnter(0.0), phiEnter(0.0), etaExit(0.0), phiExit(0.0) {}
};

class MySensitiveDetector : public G4VSensitiveDetector {
public:
    MySensitiveDetector(const G4String& name);
    virtual ~MySensitiveDetector();
    
    virtual void Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    std::map<std::string, ParticleData> fParticleData;
    void WriteParticleData(const ParticleData& data);
};

#endif
