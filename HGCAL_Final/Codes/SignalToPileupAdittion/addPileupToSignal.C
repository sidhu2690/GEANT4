#include <map>
#include <vector>
#include <set>
#include <tuple>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TROOT.h>


void addPileupToSignal(const char* signalFile = "Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root",
                       const char* pileupFile = "PileUp_nPU_035_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root",
                       const char* outputFile = "Electron_nPU_35_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root") {
    
    using namespace std;
    
    // Open input files
    TFile* fSignal = TFile::Open(signalFile, "READ");
    if (!fSignal || fSignal->IsZombie()) {
        cerr << "Error opening signal file!" << endl;
        return;
    }
    
    TFile* fPileup = TFile::Open(pileupFile, "READ");
    if (!fPileup || fPileup->IsZombie()) {
        cerr << "Error opening pileup file!" << endl;
        return;
    }
    
    // Get signal trees
    TTree* sigGenTree = (TTree*)fSignal->Get("GeneratorInfo");
    TTree* sigEtaPhiTree = (TTree*)fSignal->Get("Eta_Phi_CellWiseSegmentation");
    
    // Get pileup trees
    TTree* puGenTree = (TTree*)fPileup->Get("GeneratorInfo");
    TTree* puEtaPhiTree = (TTree*)fPileup->Get("Eta_Phi_CellWiseSegmentation");
    
    // Build signal event indices
    cout << "Building signal event indices..." << endl;
    map<int, vector<Long64_t>> sigEtaPhiEventIndex;
    
    {
        int event_id;
        sigEtaPhiTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < sigEtaPhiTree->GetEntries(); i++) {
            sigEtaPhiTree->GetEntry(i);
            sigEtaPhiEventIndex[event_id].push_back(i);
        }
        sigEtaPhiTree->ResetBranchAddresses();
    }
    
    vector<int> sigEventList;
    for (auto& kv : sigEtaPhiEventIndex) {
        sigEventList.push_back(kv.first);
    }
    int nSignalEvents = sigEventList.size();
    cout << "Found " << nSignalEvents << " signal events." << endl;
    
    // Build pileup event indices
    cout << "Building pileup event indices..." << endl;
    map<int, vector<Long64_t>> puEtaPhiEventIndex;
    
    {
        int event_id;
        puEtaPhiTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < puEtaPhiTree->GetEntries(); i++) {
            puEtaPhiTree->GetEntry(i);
            puEtaPhiEventIndex[event_id].push_back(i);
        }
        puEtaPhiTree->ResetBranchAddresses();
    }
    
    vector<int> puEventList;
    for (auto& kv : puEtaPhiEventIndex) {
        puEventList.push_back(kv.first);
    }
    int nPileupEvents = puEventList.size();
    cout << "Found " << nPileupEvents << " pileup events." << endl;
    
    if (nSignalEvents > nPileupEvents) {
        cerr << "Error: More signal events (" << nSignalEvents << ") than pileup events (" << nPileupEvents << ")!" << endl;
        cerr << "Cannot create unique pileup mapping." << endl;
        return;
    }
    
    // Create output file
    TFile* fout = new TFile(outputFile, "RECREATE");
    
    // Clone separate GeneratorInfo trees
    cout << "Cloning Signal GeneratorInfo tree..." << endl;
    TTree* outSigGenTree = sigGenTree->CloneTree(-1, "fast");
    outSigGenTree->SetName("Signal_GeneratorInfo");
    outSigGenTree->SetTitle("Signal Generator Level Particle Data");
    outSigGenTree->Write();
    
    cout << "Cloning Pileup GeneratorInfo tree..." << endl;
    TTree* outPuGenTree = puGenTree->CloneTree(-1, "fast");
    outPuGenTree->SetName("Pileup_GeneratorInfo");
    outPuGenTree->SetTitle("Pileup Generator Level Particle Data");
    outPuGenTree->Write();
    
    // Clone signal EtaPhi tree
    cout << "Cloning Signal EtaPhi tree..." << endl;
    TTree* outSigEtaPhiTree = sigEtaPhiTree->CloneTree(-1, "fast");
    outSigEtaPhiTree->SetName("Signal_Eta_Phi_CellWiseSegmentation");
    outSigEtaPhiTree->SetTitle("Signal Eta-Phi Cell-wise Segmented Hit Data");
    outSigEtaPhiTree->Write();
    
    // Clone pileup EtaPhi tree
    cout << "Cloning Pileup EtaPhi tree..." << endl;
    TTree* outPuEtaPhiTree = puEtaPhiTree->CloneTree(-1, "fast");
    outPuEtaPhiTree->SetName("Pileup_Eta_Phi_CellWiseSegmentation");
    outPuEtaPhiTree->SetTitle("Pileup Eta-Phi Cell-wise Segmented Hit Data");
    outPuEtaPhiTree->Write();
    
    // Generate unique random pileup event mapping
    cout << "Generating unique random pileup event mapping..." << endl;
    TRandom3 rng(0);
    
    map<int, int> eventMapping;
    set<int> usedPuIndices;
    
    for (int sigEvt : sigEventList) {
        int puIdx;
        do {
            puIdx = rng.Integer(puEventList.size());
        } while (usedPuIndices.find(puIdx) != usedPuIndices.end());
        
        usedPuIndices.insert(puIdx);
        eventMapping[sigEvt] = puEventList[puIdx];
    }
    
    // Create event mapping tree
    int map_event_id, map_pileup_event_id;
    TTree* evtMapTree = new TTree("EventMapping", "Signal to Pileup event mapping");
    evtMapTree->Branch("event_id", &map_event_id, "event_id/I");
    evtMapTree->Branch("pileup_event_id", &map_pileup_event_id, "pileup_event_id/I");
    
    for (auto& kv : eventMapping) {
        map_event_id = kv.first;
        map_pileup_event_id = kv.second;
        evtMapTree->Fill();
    }
    evtMapTree->Write();
    
    // Set up branches for signal eta-phi tree
    int se_event_id, se_layer, se_ieta, se_iphi, se_ADC;
    double se_xi, se_yi, se_zi, se_theta, se_phi, se_eta, se_edep;
    
    sigEtaPhiTree->SetBranchAddress("event_id", &se_event_id);
    sigEtaPhiTree->SetBranchAddress("layer", &se_layer);
    sigEtaPhiTree->SetBranchAddress("ieta", &se_ieta);
    sigEtaPhiTree->SetBranchAddress("iphi", &se_iphi);
    sigEtaPhiTree->SetBranchAddress("xi", &se_xi);
    sigEtaPhiTree->SetBranchAddress("yi", &se_yi);
    sigEtaPhiTree->SetBranchAddress("zi", &se_zi);
    sigEtaPhiTree->SetBranchAddress("theta", &se_theta);
    sigEtaPhiTree->SetBranchAddress("phi", &se_phi);
    sigEtaPhiTree->SetBranchAddress("eta", &se_eta);
    sigEtaPhiTree->SetBranchAddress("edep", &se_edep);
    sigEtaPhiTree->SetBranchAddress("ADC", &se_ADC);
    
    // Set up branches for pileup eta-phi tree
    int pe_event_id, pe_layer, pe_ieta, pe_iphi, pe_ADC;
    double pe_xi, pe_yi, pe_zi, pe_theta, pe_phi, pe_eta, pe_edep;
    
    puEtaPhiTree->SetBranchAddress("event_id", &pe_event_id);
    puEtaPhiTree->SetBranchAddress("layer", &pe_layer);
    puEtaPhiTree->SetBranchAddress("ieta", &pe_ieta);
    puEtaPhiTree->SetBranchAddress("iphi", &pe_iphi);
    puEtaPhiTree->SetBranchAddress("xi", &pe_xi);
    puEtaPhiTree->SetBranchAddress("yi", &pe_yi);
    puEtaPhiTree->SetBranchAddress("zi", &pe_zi);
    puEtaPhiTree->SetBranchAddress("theta", &pe_theta);
    puEtaPhiTree->SetBranchAddress("phi", &pe_phi);
    puEtaPhiTree->SetBranchAddress("eta", &pe_eta);
    puEtaPhiTree->SetBranchAddress("edep", &pe_edep);
    puEtaPhiTree->SetBranchAddress("ADC", &pe_ADC);
    
    // Create combined eta-phi tree
    int oe_event_id, oe_layer, oe_ieta, oe_iphi, oe_ADC;
    double oe_xi, oe_yi, oe_zi, oe_theta, oe_phi, oe_eta, oe_edep;
    
    TTree* outEtaPhiTree = new TTree("Eta_Phi_CellWiseSegmentation", "Combined Eta-Phi Cell-wise Segmented Hit Data");
    outEtaPhiTree->SetAutoSave(0);
    outEtaPhiTree->SetAutoFlush(50000);
    
    outEtaPhiTree->Branch("event_id", &oe_event_id, "event_id/I");
    outEtaPhiTree->Branch("layer", &oe_layer, "layer/I");
    outEtaPhiTree->Branch("ieta", &oe_ieta, "ieta/I");
    outEtaPhiTree->Branch("iphi", &oe_iphi, "iphi/I");
    outEtaPhiTree->Branch("xi", &oe_xi, "xi/D");
    outEtaPhiTree->Branch("yi", &oe_yi, "yi/D");
    outEtaPhiTree->Branch("zi", &oe_zi, "zi/D");
    outEtaPhiTree->Branch("theta", &oe_theta, "theta/D");
    outEtaPhiTree->Branch("phi", &oe_phi, "phi/D");
    outEtaPhiTree->Branch("eta", &oe_eta, "eta/D");
    outEtaPhiTree->Branch("edep", &oe_edep, "edep/D");
    outEtaPhiTree->Branch("ADC", &oe_ADC, "ADC/I");
    
    // Hit structure
    struct Hit {
        double xi, yi, zi, theta, phi, eta, edep;
        int ADC;
    };
    
    cout << "Processing " << nSignalEvents << " signal events..." << endl;
    
    int processed = 0;
    for (int sigEvt : sigEventList) {
        processed++;
        if (processed % 5000 == 0) {
            cout << "Event " << processed << "/" << nSignalEvents << endl;
        }
        
        int puEvt = eventMapping[sigEvt];
        
        // Combine eta-phi hits: key = (layer, ieta, iphi)
        map<tuple<int,int,int>, Hit> etaPhiHitMap;
        
        // Add signal hits first
        auto sigEtaIt = sigEtaPhiEventIndex.find(sigEvt);
        if (sigEtaIt != sigEtaPhiEventIndex.end()) {
            for (Long64_t entry : sigEtaIt->second) {
                sigEtaPhiTree->GetEntry(entry);
                auto key = make_tuple(se_layer, se_ieta, se_iphi);
                etaPhiHitMap[key] = {se_xi, se_yi, se_zi, se_theta, se_phi, se_eta, se_edep, se_ADC};
            }
        }
        
        // Add pileup hits (sum edep and ADC if overlapping)
        auto puEtaIt = puEtaPhiEventIndex.find(puEvt);
        if (puEtaIt != puEtaPhiEventIndex.end()) {
            for (Long64_t entry : puEtaIt->second) {
                puEtaPhiTree->GetEntry(entry);
                auto key = make_tuple(pe_layer, pe_ieta, pe_iphi);
                
                if (etaPhiHitMap.find(key) == etaPhiHitMap.end()) {
                    etaPhiHitMap[key] = {pe_xi, pe_yi, pe_zi, pe_theta, pe_phi, pe_eta, pe_edep, pe_ADC};
                } else {
                    etaPhiHitMap[key].edep += pe_edep;
                    etaPhiHitMap[key].ADC += pe_ADC;
                }
            }
        }
        
        // Write combined eta-phi hits
        for (auto& kv : etaPhiHitMap) {
            oe_event_id = sigEvt;
            oe_layer = get<0>(kv.first);
            oe_ieta = get<1>(kv.first);
            oe_iphi = get<2>(kv.first);
            oe_xi = kv.second.xi;
            oe_yi = kv.second.yi;
            oe_zi = kv.second.zi;
            oe_theta = kv.second.theta;
            oe_phi = kv.second.phi;
            oe_eta = kv.second.eta;
            oe_edep = kv.second.edep;
            oe_ADC = kv.second.ADC;
            outEtaPhiTree->Fill();
        }

    if (processed % 100 == 0) cout << "Output event " << processed << " completed." << endl;
    }
    
    cout << "Writing output..." << endl;
    outEtaPhiTree->Write();
    
    fout->Close();
    fSignal->Close();
    fPileup->Close();
    
    cout << "Done! Output file: " << outputFile << endl;
    cout << "Trees in output:" << endl;
    cout << "  - Signal_GeneratorInfo (cloned from signal)" << endl;
    cout << "  - Pileup_GeneratorInfo (cloned from pileup)" << endl;
    cout << "  - Signal_Eta_Phi_CellWiseSegmentation (cloned from signal)" << endl;
    cout << "  - Pileup_Eta_Phi_CellWiseSegmentation (cloned from pileup)" << endl;
    cout << "  - EventMapping (signal event -> pileup event mapping)" << endl;
    cout << "  - Eta_Phi_CellWiseSegmentation (combined signal + pileup)" << endl;
}
