#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4ThreeVector.hh"
#include <map>
#include <set>
#include <string>

// Structure to hold particle data for each track
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
    bool hasEntryData;
    bool hasExitData;
    
    ParticleData() : eventID(0), trackID(0), layer(0), particleID(0), cumTr(-1),  // INITIALIZE cumTr
                     energyBefore(0.0), energyAfter(0.0), totalEnergyDeposited(0.0),
                     hasEntryData(false), hasExitData(false) {}
};

class MySensitiveDetector : public G4VSensitiveDetector {
public:
    MySensitiveDetector(const G4String& name);
    virtual ~MySensitiveDetector();
    
    virtual void Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    // Map to store particle data: key = "eventID_trackID_layer"
    std::map<std::string, ParticleData> fParticleData;
    
    // Set to keep track of processed track-layer combinations
    std::set<std::string> fProcessedTrackLayers;
    
    // Helper function to write data
    void WriteParticleData(const ParticleData& data);
};

#endif
