#include "detector.hh"
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
    
    // Get event ID safely
    G4int eventID = 0;
    const G4Event* currentEvent = G4RunManager::GetRunManager()->GetCurrentEvent();
    if (currentEvent) {
        eventID = currentEvent->GetEventID();
    }
    
    // Get layer information from copy number
    G4int copyNumber = preStepPoint->GetTouchable()->GetCopyNumber();
    G4int layer = copyNumber;
    
    // Create unique identifier for this track-layer combination
    std::string trackLayerKey = std::to_string(eventID) + "_" + 
                                std::to_string(trackID) + "_" + 
                                std::to_string(layer);
    
    // Record entry data if this is the first step in this layer
    if (fParticleData.find(trackLayerKey) == fParticleData.end() && 
        fProcessedTrackLayers.find(trackLayerKey) == fProcessedTrackLayers.end()) {
        
        ParticleData& data = fParticleData[trackLayerKey];
        data.eventID = eventID;
        data.trackID = trackID;
        data.layer = layer;
        data.particleID = particleID;
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
            if (data.hasEntryData && data.hasExitData) {
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
    // (e.g., particles that stopped inside the volume)
    for (auto& pair : fParticleData) {
        const ParticleData& data = pair.second;
        if (data.hasEntryData && data.hasExitData) {
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
    
    // Fill ntuple columns (assuming ntuple ID = 0)
    man->FillNtupleIColumn(0, 0, data.eventID);
    man->FillNtupleIColumn(0, 1, data.trackID);
    man->FillNtupleIColumn(0, 2, data.layer);
    man->FillNtupleDColumn(0, 3, data.energyBefore / MeV);
    man->FillNtupleDColumn(0, 4, data.energyAfter / MeV);
    man->FillNtupleDColumn(0, 5, data.totalEnergyDeposited / MeV);
    man->FillNtupleDColumn(0, 6, data.momentumBefore.x() / MeV);
    man->FillNtupleDColumn(0, 7, data.momentumBefore.y() / MeV);
    man->FillNtupleDColumn(0, 8, data.momentumBefore.z() / MeV);
    man->FillNtupleDColumn(0, 9, data.momentumAfter.x() / MeV);
    man->FillNtupleDColumn(0, 10, data.momentumAfter.y() / MeV);
    man->FillNtupleDColumn(0, 11, data.momentumAfter.z() / MeV);
    man->FillNtupleDColumn(0, 12, data.positionEnter.x() / mm);
    man->FillNtupleDColumn(0, 13, data.positionEnter.y() / mm);
    man->FillNtupleDColumn(0, 14, data.positionEnter.z() / mm);
    man->FillNtupleDColumn(0, 15, data.positionExit.x() / mm);
    man->FillNtupleDColumn(0, 16, data.positionExit.y() / mm);
    man->FillNtupleDColumn(0, 17, data.positionExit.z() / mm);
    man->FillNtupleDColumn(0, 18, rEnter / mm);
    man->FillNtupleDColumn(0, 19, rExit / mm);
    man->FillNtupleIColumn(0, 20, data.particleID);
    
    // Commit this row to the ntuple
    man->AddNtupleRow(0);
}
