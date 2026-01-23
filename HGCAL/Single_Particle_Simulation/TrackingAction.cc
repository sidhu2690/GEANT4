#include "TrackingAction.hh"
#include "TrackInformation.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include <map>

// Static variables
static std::map<G4int, G4int> trackCumTrMap;
static G4int lastEventID = -1;  // Track which event we're in

MyTrackingAction::MyTrackingAction() {}

MyTrackingAction::~MyTrackingAction() {}

void MyTrackingAction::PreUserTrackingAction(const G4Track* track)
{
    // Get current event ID
    G4int currentEventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    
    // Clear map if this is a new event
    if (currentEventID != lastEventID) {
        trackCumTrMap.clear();
        lastEventID = currentEventID;
        G4cout << "=== New Event " << currentEventID << ": Cleared trackCumTrMap ===" << G4endl;
    }
    
    G4Track* nonConstTrack = const_cast<G4Track*>(track);
    
    if (track->GetParentID() == 0) {
        // This is a primary particle
        const G4DynamicParticle* dynamicParticle = track->GetDynamicParticle();
        if (dynamicParticle) {
            const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
            if (primaryParticle) {
                G4VUserPrimaryParticleInformation* primaryInfo = 
                    primaryParticle->GetUserInformation();
                    
                if (primaryInfo) {
                    PrimaryParticleInformation* ppInfo = 
                        dynamic_cast<PrimaryParticleInformation*>(primaryInfo);
                    if (ppInfo) {
                        G4int cumTr = ppInfo->GetCumTr();
                        
                        nonConstTrack->SetUserInformation(new TrackInformation(cumTr));
                        trackCumTrMap[track->GetTrackID()] = cumTr;
                        
                        G4cout << "Primary track " << track->GetTrackID() 
                               << " assigned cumTr = " << cumTr << G4endl;
                    }
                }
            }
        }
    } else {
        // Secondary particle - inherit from parent
        G4int parentID = track->GetParentID();
        
        auto it = trackCumTrMap.find(parentID);
        if (it != trackCumTrMap.end()) {
            G4int cumTr = it->second;
            
            nonConstTrack->SetUserInformation(new TrackInformation(cumTr));
            trackCumTrMap[track->GetTrackID()] = cumTr;
        } else {
            G4cout << "WARNING: Could not find cumTr for parent track " << parentID << G4endl;
        }
    }
}

void MyTrackingAction::PostUserTrackingAction(const G4Track* track)
{
    // Nothing needed here
}
