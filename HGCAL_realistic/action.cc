#include "action.hh"
#include "generator.hh"
#include "run.hh"
#include "TrackingAction.hh"  // ADD THIS

// Constructor
MyActionInitialization::MyActionInitialization() {}

// Destructor
MyActionInitialization::~MyActionInitialization() {}

// Build method
void MyActionInitialization::Build() const {
    // Primary generator
    MyPrimaryGenerator* generator = new MyPrimaryGenerator();
    SetUserAction(generator);

    // Run action for ROOT output
    MyRunAction* runAction = new MyRunAction();
    SetUserAction(runAction);
    
    // Tracking action for cumTr inheritance (ADD THIS)
    MyTrackingAction* trackingAction = new MyTrackingAction();
    SetUserAction(trackingAction);
}
