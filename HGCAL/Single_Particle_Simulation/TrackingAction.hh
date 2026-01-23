#ifndef TRACKINGACTION_HH
#define TRACKINGACTION_HH

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class MyTrackingAction : public G4UserTrackingAction {
public:
    MyTrackingAction();
    virtual ~MyTrackingAction();
    
    virtual void PreUserTrackingAction(const G4Track* track) override;
    virtual void PostUserTrackingAction(const G4Track* track) override;
};

#endif
