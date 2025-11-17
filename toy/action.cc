#include "action.hh"
#include "generator.hh"
#include "run.hh"

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
}
