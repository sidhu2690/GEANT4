#include "run.hh"
#include "G4AnalysisManager.hh"
#include <sstream>

MyRunAction::MyRunAction() {
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    
    // Ntuple 0: Generator-level (truth-level) information
    man->CreateNtuple("GeneratorInfo", "Generator Level Particle Data");
    man->CreateNtupleIColumn("event_id"); // 0
    man->CreateNtupleIColumn("particle_id"); // 1
    man->CreateNtupleDColumn("px_MeV"); // 2
    man->CreateNtupleDColumn("py_MeV"); // 3
    man->CreateNtupleDColumn("pz_MeV"); // 4
    man->CreateNtupleDColumn("pTot_MeV"); // 5
    man->CreateNtupleDColumn("energy_MeV"); // 6
    man->CreateNtupleDColumn("eta"); // 7
    man->CreateNtupleDColumn("phi"); // 8
    man->CreateNtupleDColumn("theta"); // 9
    man->CreateNtupleIColumn("cumTr"); // 10
    man->CreateNtupleDColumn("pt_GeV"); // 11
    man->CreateNtupleDColumn("charge"); // 12
    man->CreateNtupleIColumn("layer"); // 13
    man->CreateNtupleDColumn("energy_deposited_MeV"); // 14
    man->CreateNtupleDColumn("xi"); // 15
    man->CreateNtupleDColumn("yi"); // 16
    man->CreateNtupleIColumn("seed"); // 17
    man->CreateNtupleDColumn("random_number"); // 18
    man->FinishNtuple(0);
    
    // Ntuple 1: Hit-level information (WITH cumTr AND charge ADDED)
    man->CreateNtuple("ParticleTracking", "Particle Tracking Data");
    man->CreateNtupleIColumn("eventID"); // 0
    man->CreateNtupleIColumn("track_id"); // 1
    man->CreateNtupleIColumn("layer"); // 2
    man->CreateNtupleDColumn("energy_before_MeV"); // 3
    man->CreateNtupleDColumn("energy_after_MeV"); // 4
    man->CreateNtupleDColumn("energy_deposited_MeV"); // 5
    man->CreateNtupleDColumn("px_before_MeV"); // 6
    man->CreateNtupleDColumn("py_before_MeV"); // 7
    man->CreateNtupleDColumn("pz_before_MeV"); // 8
    man->CreateNtupleDColumn("px_after_MeV"); // 9
    man->CreateNtupleDColumn("py_after_MeV"); // 10
    man->CreateNtupleDColumn("pz_after_MeV"); // 11
    man->CreateNtupleDColumn("x_enter_mm"); // 12
    man->CreateNtupleDColumn("y_enter_mm"); // 13
    man->CreateNtupleDColumn("z_enter_mm"); // 14
    man->CreateNtupleDColumn("x_exit_mm"); // 15
    man->CreateNtupleDColumn("y_exit_mm"); // 16
    man->CreateNtupleDColumn("z_exit_mm"); // 17
    man->CreateNtupleDColumn("r_enter_mm"); // 18
    man->CreateNtupleDColumn("r_exit_mm"); // 19
    man->CreateNtupleIColumn("particle_id"); // 20
    man->CreateNtupleIColumn("cumTr"); // 21
    man->CreateNtupleDColumn("charge"); // 22
    man->FinishNtuple(1);
}

MyRunAction::~MyRunAction() {
}

void MyRunAction::BeginOfRunAction(const G4Run* run) {
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->OpenFile("Photon_Pt_200_Eta_195_Events_2K_PU_000_Set01_Step1.root");
}

void MyRunAction::EndOfRunAction(const G4Run*) {
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();
}
