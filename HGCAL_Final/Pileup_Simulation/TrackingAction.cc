#include "TrackingAction.hh"
#include "TrackInformation.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include <map>

// Static map to store cumTr for each track
static std::map<G4int, G4int> trackCumTrMap;

MyTrackingAction::MyTrackingAction() {}

MyTrackingAction::~MyTrackingAction() {}

void MyTrackingAction::PreUserTrackingAction(const G4Track* track)
{
    G4Track* nonConstTrack = const_cast<G4Track*>(track);
    
    if (track->GetParentID() == 0) {
        // This is a primary particle
        // Transfer information from PrimaryParticle to Track
        
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
                        
                        // Create TrackInformation for this track
                        nonConstTrack->SetUserInformation(new TrackInformation(cumTr));
                        
                        // Store in map for secondaries
                        trackCumTrMap[track->GetTrackID()] = cumTr;
                        
                        G4cout << "Primary track " << track->GetTrackID() 
                               << " assigned cumTr = " << cumTr << G4endl;
                    }
                }
            }
        }
    } else {
        // This is a secondary particle - inherit from parent
        G4int parentID = track->GetParentID();
        
        auto it = trackCumTrMap.find(parentID);
        if (it != trackCumTrMap.end()) {
            G4int cumTr = it->second;
            
            // Assign to this track
            nonConstTrack->SetUserInformation(new TrackInformation(cumTr));
            
            // Store for future secondaries from this track
            trackCumTrMap[track->GetTrackID()] = cumTr;
        } else {
            G4cout << "WARNING: Could not find cumTr for parent track " << parentID << G4endl;
        }
    }
}

void MyTrackingAction::PostUserTrackingAction(const G4Track* track)
{
    // Optional: Clean up map entries for finished tracks to save memory
    // trackCumTrMap.erase(track->GetTrackID());
}
