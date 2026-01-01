#include <iostream>
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4PhysListFactory.hh"  
#include "construction.hh"
#include "action.hh"

int main(int argc, char** argv) {
    // Run manager
    G4RunManager* runManager = new G4RunManager();
    
    // Detector construction
    runManager->SetUserInitialization(new MyDetectorConstruction());

    // Physics list (load built-in FTFP_BERT)
    G4PhysListFactory factory;
    auto physicsList = factory.GetReferencePhysList("FTFP_BERT");
    runManager->SetUserInitialization(physicsList);

    // Action initialization
    runManager->SetUserInitialization(new MyActionInitialization());
    
    // Initialize kernel
    runManager->Initialize();
    
    // UI manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    
    // BATCH MODE: Execute macro file if provided
    if (argc > 1) {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    } else {
        // Default: run 1 event
        UImanager->ApplyCommand("/run/beamOn 1");
    }
    
    // Cleanup
    delete runManager;
    
    return 0;
}