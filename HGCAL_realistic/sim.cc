#include <iostream>
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4PhysListFactory.hh"  
#include "construction.hh"
#include "action.hh"

int main(int argc, char** argv) {
    // Run manager
    G4RunManager* runManager = new G4RunManager();
    
    // Detector construction
    runManager->SetUserInitialization(new MyDetectorConstruction());

    // ✅ Physics list (load built-in FTFP_BERT_EMZ)
    G4PhysListFactory factory;
    auto physicsList = factory.GetReferencePhysList("FTFP_BERT");
    runManager->SetUserInitialization(physicsList);

    // Action initialization
    runManager->SetUserInitialization(new MyActionInitialization());
    
    // Initialize kernel
    runManager->Initialize();
    
    // Visualization manager
    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();
    
    // UI executive
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
    
    // UI manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    
    // Visualization commands
    UImanager->ApplyCommand("/vis/open OGL");
    UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 1 1 1");
    UImanager->ApplyCommand("/vis/drawVolume");
    UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
    UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
    
    // Start interactive session
    ui->SessionStart();
    
    // Cleanup
    delete ui;
    delete visManager;
    delete runManager;
    
    return 0;
}

