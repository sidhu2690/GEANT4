#include "run.hh"
#include "G4AnalysisManager.hh"
#include <sstream>

MyRunAction::MyRunAction()
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    
    // Create ntuple matching the original CSV structure
    man->CreateNtuple("ParticleTracking", "Particle Tracking Data");
    man->CreateNtupleIColumn("eventID");              // Column 1
    man->CreateNtupleIColumn("track_id");              // Column 1
    man->CreateNtupleIColumn("layer");                 // Column 2
    man->CreateNtupleDColumn("energy_before_MeV");     // Column 3
    man->CreateNtupleDColumn("energy_after_MeV");      // Column 4
    man->CreateNtupleDColumn("energy_deposited_MeV");  // Column 5
    man->CreateNtupleDColumn("px_before_MeV");         // Column 6
    man->CreateNtupleDColumn("py_before_MeV");         // Column 7
    man->CreateNtupleDColumn("pz_before_MeV");         // Column 8
    man->CreateNtupleDColumn("px_after_MeV");          // Column 9
    man->CreateNtupleDColumn("py_after_MeV");          // Column 10
    man->CreateNtupleDColumn("pz_after_MeV");          // Column 11
    man->CreateNtupleDColumn("x_enter_mm");            // Column 12
    man->CreateNtupleDColumn("y_enter_mm");            // Column 13
    man->CreateNtupleDColumn("z_enter_mm");            // Column 14
    man->CreateNtupleDColumn("x_exit_mm");             // Column 15
    man->CreateNtupleDColumn("y_exit_mm");             // Column 16
    man->CreateNtupleDColumn("z_exit_mm");             // Column 17
    man->CreateNtupleDColumn("r_enter_mm");            // Column 18
    man->CreateNtupleDColumn("r_exit_mm");             // Column 19
    man->CreateNtupleIColumn("particle_id");           // Column 20
    man->FinishNtuple(0);
}

MyRunAction::~MyRunAction()
{
}

void MyRunAction::BeginOfRunAction(const G4Run* run)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    G4int runID = run->GetRunID();
    
    std::stringstream strRunID;
    strRunID << runID;
    
    man->OpenFile("hgcal_output"+strRunID.str()+".root");
}

void MyRunAction::EndOfRunAction(const G4Run*)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();
}
