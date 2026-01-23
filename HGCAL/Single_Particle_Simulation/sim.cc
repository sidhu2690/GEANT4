#include <iostream> 
#include "G4RunManager.hh" 
#include "G4UImanager.hh" 
#include "G4PhysListFactory.hh" 
#include "construction.hh" 
#include "action.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

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
    
    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();
    
    // UI manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    


    // interactive mode
    /*
    if (argc == 1) {
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        
        // Setup visualization
        UImanager->ApplyCommand("/vis/open OGL");
        UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
        UImanager->ApplyCommand("/vis/drawVolume");
        UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 1 1 1");
        UImanager->ApplyCommand("/vis/viewer/zoom 1.5");
        UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
        UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");
        
        // Start interactive session
        ui->SessionStart();
        delete ui;
    } else {
        // BATCH MODE: Execute macro file
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }
    */
    //
    // batch mode
    if (argc > 1) {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    } else {
        // Default: run 1 event
        UImanager->ApplyCommand("/run/beamOn 1");
    }
    
    //
    // Cleanup
    delete visManager;
    delete runManager;
    
    return 0;
}
