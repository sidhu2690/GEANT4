#include "detector.hh"
#include "TrackInformation.hh"
#include <cmath>
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"

MySensitiveDetector::MySensitiveDetector(const G4String& name)
: G4VSensitiveDetector(name)
{
}

MySensitiveDetector::~MySensitiveDetector()
{
}

void MySensitiveDetector::Initialize(G4HCofThisEvent* hce)
{
    // Clear temporary data for new event
    fParticleData.clear();
    fProcessedTrackLayers.clear();
}

G4bool MySensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* history)
{
    // Validate step and track
    if (!step) return false;
    
    G4Track* track = step->GetTrack();
    if (!track) return false;
    
    // Get pre and post step points
    const G4StepPoint* preStepPoint = step->GetPreStepPoint();
    const G4StepPoint* postStepPoint = step->GetPostStepPoint();
    if (!preStepPoint || !postStepPoint) return false;
    
    // Get basic track information
    G4int trackID = track->GetTrackID();
    G4int particleID = track->GetDefinition()->GetPDGEncoding();
    G4double charge = track->GetDefinition()->GetPDGCharge();  // ADD THIS LINE
    
    // **KEY STEP: Extract cumTr from track information**
    G4int cumTr = -1;  // Default value if not found
    G4VUserTrackInformation* userInfo = track->GetUserInformation();
    if (userInfo) {
        TrackInformation* trackInfo = dynamic_cast<TrackInformation*>(userInfo);
        if (trackInfo) {
            cumTr = trackInfo->GetCumTr();
        }
    }
    
    // Get event ID safely
    G4int eventID = 0;
    const G4Event* currentEvent = G4RunManager::GetRunManager()->GetCurrentEvent();
    if (currentEvent) {
        eventID = currentEvent->GetEventID();
    }
    
    // Get layer information from copy number
    G4int copyNumber = preStepPoint->GetTouchable()->GetCopyNumber();
    G4int layer = copyNumber;
    
    // Create unique identifier based on entry position and angle
    G4ThreeVector entryPos = preStepPoint->GetPosition();
    G4ThreeVector entryMom = preStepPoint->GetMomentum();

    // Round to reasonable precision to create key
    G4double entryX = std::round(entryPos.x() / mm * 10.0) / 10.0;  // 0.1 mm precision
    G4double entryY = std::round(entryPos.y() / mm * 10.0) / 10.0;
    G4double entryPhi = std::round(std::atan2(entryMom.y(), entryMom.x()) * 100.0) / 100.0;  // 0.01 rad precision

    std::string trackLayerKey = std::to_string(eventID) + "_" + 
                                std::to_string(trackID) + "_" + 
                                std::to_string(layer) + "_" +
                                std::to_string(entryX) + "_" +
                                std::to_string(entryY) + "_" +
                                std::to_string(entryPhi)
    
    // Record entry data if this is the first step in this layer
    if (fParticleData.find(trackLayerKey) == fParticleData.end() && 
        fProcessedTrackLayers.find(trackLayerKey) == fProcessedTrackLayers.end()) {
        
        ParticleData& data = fParticleData[trackLayerKey];
        data.eventID = eventID;
        data.trackID = trackID;
        data.layer = layer;
        data.particleID = particleID;
        data.cumTr = cumTr;
        data.charge = charge;  // STORE charge
        data.energyBefore = preStepPoint->GetKineticEnergy();
        data.momentumBefore = preStepPoint->GetMomentum();
        data.positionEnter = preStepPoint->GetPosition();
        data.hasEntryData = true;
        data.totalEnergyDeposited = 0.0;
        data.hasExitData = false;
    }
    
    // Accumulate energy deposited in this step
    auto it = fParticleData.find(trackLayerKey);
    if (it != fParticleData.end()) {
        ParticleData& data = it->second;
        data.totalEnergyDeposited += step->GetTotalEnergyDeposit();
        
        // Update exit information (continuously updated until particle leaves)
        data.energyAfter = postStepPoint->GetKineticEnergy();
        data.momentumAfter = postStepPoint->GetMomentum();
        data.positionExit = postStepPoint->GetPosition();
        data.hasExitData = true;
        
        // Check if particle is leaving the volume or stopping
        G4StepStatus postStepStatus = postStepPoint->GetStepStatus();
        bool isLeavingVolume = (postStepStatus == fGeomBoundary);
        bool isStoppingInVolume = (track->GetTrackStatus() != fAlive);
        
        if (isLeavingVolume || isStoppingInVolume) {
            // Particle has completed its passage through this layer
            if (data.hasEntryData && data.hasExitData && data.totalEnergyDeposited > 10.0*eV) {
                WriteParticleData(data);
            }
            
            // Mark as processed and remove from active data
            fProcessedTrackLayers.insert(trackLayerKey);
            fParticleData.erase(it);
        }
    }
    
    return true;
}

void MySensitiveDetector::EndOfEvent(G4HCofThisEvent* hce)
{
    // Write any remaining data that wasn't written during the event
    for (auto& pair : fParticleData) {
        const ParticleData& data = pair.second;
        if (data.hasEntryData && data.hasExitData && data.totalEnergyDeposited > 10.0*eV) {
            WriteParticleData(data);
        }
    }
    
    // Clear all data structures for next event
    fParticleData.clear();
    fProcessedTrackLayers.clear();
}

void MySensitiveDetector::WriteParticleData(const ParticleData& data)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    
    // Calculate radial distances in x-y plane
    G4double rEnter = std::sqrt(data.positionEnter.x() * data.positionEnter.x() + 
                                data.positionEnter.y() * data.positionEnter.y());
    G4double rExit = std::sqrt(data.positionExit.x() * data.positionExit.x() + 
                               data.positionExit.y() * data.positionExit.y());
    
    // Fill ntuple columns 
    man->FillNtupleIColumn(1, 0, data.eventID);
    man->FillNtupleIColumn(1, 1, data.trackID);
    man->FillNtupleIColumn(1, 2, data.layer);
    man->FillNtupleDColumn(1, 3, data.energyBefore / MeV);
    man->FillNtupleDColumn(1, 4, data.energyAfter / MeV);
    man->FillNtupleDColumn(1, 5, data.totalEnergyDeposited / MeV);
    man->FillNtupleDColumn(1, 6, data.momentumBefore.x() / MeV);
    man->FillNtupleDColumn(1, 7, data.momentumBefore.y() / MeV);
    man->FillNtupleDColumn(1, 8, data.momentumBefore.z() / MeV);
    man->FillNtupleDColumn(1, 9, data.momentumAfter.x() / MeV);
    man->FillNtupleDColumn(1, 10, data.momentumAfter.y() / MeV);
    man->FillNtupleDColumn(1, 11, data.momentumAfter.z() / MeV);
    man->FillNtupleDColumn(1, 12, data.positionEnter.x() / mm);
    man->FillNtupleDColumn(1, 13, data.positionEnter.y() / mm);
    man->FillNtupleDColumn(1, 14, data.positionEnter.z() / mm);
    man->FillNtupleDColumn(1, 15, data.positionExit.x() / mm);
    man->FillNtupleDColumn(1, 16, data.positionExit.y() / mm);
    man->FillNtupleDColumn(1, 17, data.positionExit.z() / mm);
    man->FillNtupleDColumn(1, 18, rEnter / mm);
    man->FillNtupleDColumn(1, 19, rExit / mm);
    man->FillNtupleIColumn(1, 20, data.particleID);
    man->FillNtupleIColumn(1, 21, data.cumTr);
    man->FillNtupleDColumn(1, 22, data.charge);  
    
    // Commit this row to the ntuple
    man->AddNtupleRow(1);
}
