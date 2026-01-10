#include <map>
#include <vector>
#include <set>
#include <tuple>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TROOT.h>

void addPileupToSignal(const char* signalFile = "MuonM_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root",
                       const char* pileupFile = "PileUp_nPU_35_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root",
                       const char* outputFile = "MuonM_Pt_025_Eta_170_Events_2K_WithPU_Step2.root") {
    
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
    TTree* sigPixelTree = (TTree*)fSignal->Get("Pixel_CellWiseSegmentation");
    TTree* sigEtaPhiTree = (TTree*)fSignal->Get("Eta_Phi_CellWiseSegmentation");
    
    // Get pileup trees
    TTree* puGenTree = (TTree*)fPileup->Get("GeneratorInfo");
    TTree* puPixelTree = (TTree*)fPileup->Get("Pixel_CellWiseSegmentation");
    TTree* puEtaPhiTree = (TTree*)fPileup->Get("Eta_Phi_CellWiseSegmentation");
    
    // Build signal event indices
    cout << "Building signal event indices..." << endl;
    map<int, vector<Long64_t>> sigPixelEventIndex;
    map<int, vector<Long64_t>> sigEtaPhiEventIndex;
    
    {
        int event_id;
        sigPixelTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < sigPixelTree->GetEntries(); i++) {
            sigPixelTree->GetEntry(i);
            sigPixelEventIndex[event_id].push_back(i);
        }
        sigPixelTree->ResetBranchAddresses();
    }
    
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
    for (auto& kv : sigPixelEventIndex) {
        sigEventList.push_back(kv.first);
    }
    int nSignalEvents = sigEventList.size();
    cout << "Found " << nSignalEvents << " signal events." << endl;
    
    // Build pileup event indices
    cout << "Building pileup event indices..." << endl;
    map<int, vector<Long64_t>> puPixelEventIndex;
    map<int, vector<Long64_t>> puEtaPhiEventIndex;
    
    {
        int event_id;
        puPixelTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < puPixelTree->GetEntries(); i++) {
            puPixelTree->GetEntry(i);
            puPixelEventIndex[event_id].push_back(i);
        }
        puPixelTree->ResetBranchAddresses();
    }
    
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
    for (auto& kv : puPixelEventIndex) {
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
    
    // Clone signal GeneratorInfo
    cout << "Cloning Signal GeneratorInfo..." << endl;
    TTree* outSigGenTree = sigGenTree->CloneTree(-1, "fast");
    outSigGenTree->SetName("Signal_GeneratorInfo");
    outSigGenTree->SetTitle("Signal Generator Level Particle Data");
    outSigGenTree->Write();
    
    // Clone signal Pixel tree
    cout << "Cloning Signal Pixel tree..." << endl;
    TTree* outSigPixelTree = sigPixelTree->CloneTree(-1, "fast");
    outSigPixelTree->SetName("Signal_Pixel_CellWiseSegmentation");
    outSigPixelTree->SetTitle("Signal Pixel Cell-wise Segmented Hit Data");
    outSigPixelTree->Write();
    
    // Clone signal EtaPhi tree
    cout << "Cloning Signal EtaPhi tree..." << endl;
    TTree* outSigEtaPhiTree = sigEtaPhiTree->CloneTree(-1, "fast");
    outSigEtaPhiTree->SetName("Signal_Eta_Phi_CellWiseSegmentation");
    outSigEtaPhiTree->SetTitle("Signal Eta-Phi Cell-wise Segmented Hit Data");
    outSigEtaPhiTree->Write();
    
    // Clone pileup GeneratorInfo
    cout << "Cloning Pileup GeneratorInfo..." << endl;
    TTree* outPuGenTree = puGenTree->CloneTree(-1, "fast");
    outPuGenTree->SetName("Pileup_GeneratorInfo");
    outPuGenTree->SetTitle("Pileup Generator Level Particle Data");
    outPuGenTree->Write();
    
    // Clone pileup Pixel tree
    cout << "Cloning Pileup Pixel tree..." << endl;
    TTree* outPuPixelTree = puPixelTree->CloneTree(-1, "fast");
    outPuPixelTree->SetName("Pileup_Pixel_CellWiseSegmentation");
    outPuPixelTree->SetTitle("Pileup Pixel Cell-wise Segmented Hit Data");
    outPuPixelTree->Write();
    
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
    
    // Set up branches for signal pixel tree
    int sp_event_id, sp_layer, sp_i, sp_j, sp_ADC;
    double sp_xi, sp_yi, sp_zi, sp_theta, sp_phi, sp_eta, sp_edep;
    
    sigPixelTree->SetBranchAddress("event_id", &sp_event_id);
    sigPixelTree->SetBranchAddress("layer", &sp_layer);
    sigPixelTree->SetBranchAddress("i", &sp_i);
    sigPixelTree->SetBranchAddress("j", &sp_j);
    sigPixelTree->SetBranchAddress("xi", &sp_xi);
    sigPixelTree->SetBranchAddress("yi", &sp_yi);
    sigPixelTree->SetBranchAddress("zi", &sp_zi);
    sigPixelTree->SetBranchAddress("theta", &sp_theta);
    sigPixelTree->SetBranchAddress("phi", &sp_phi);
    sigPixelTree->SetBranchAddress("eta", &sp_eta);
    sigPixelTree->SetBranchAddress("edep", &sp_edep);
    sigPixelTree->SetBranchAddress("ADC", &sp_ADC);
    
    // Set up branches for pileup pixel tree
    int pp_event_id, pp_layer, pp_i, pp_j, pp_ADC;
    double pp_xi, pp_yi, pp_zi, pp_theta, pp_phi, pp_eta, pp_edep;
    
    puPixelTree->SetBranchAddress("event_id", &pp_event_id);
    puPixelTree->SetBranchAddress("layer", &pp_layer);
    puPixelTree->SetBranchAddress("i", &pp_i);
    puPixelTree->SetBranchAddress("j", &pp_j);
    puPixelTree->SetBranchAddress("xi", &pp_xi);
    puPixelTree->SetBranchAddress("yi", &pp_yi);
    puPixelTree->SetBranchAddress("zi", &pp_zi);
    puPixelTree->SetBranchAddress("theta", &pp_theta);
    puPixelTree->SetBranchAddress("phi", &pp_phi);
    puPixelTree->SetBranchAddress("eta", &pp_eta);
    puPixelTree->SetBranchAddress("edep", &pp_edep);
    puPixelTree->SetBranchAddress("ADC", &pp_ADC);
    
    // Create combined pixel tree
    int op_event_id, op_layer, op_i, op_j, op_ADC;
    double op_xi, op_yi, op_zi, op_theta, op_phi, op_eta, op_edep;
    
    TTree* outPixelTree = new TTree("Pixel_CellWiseSegmentation", "Combined Pixel Cell-wise Segmented Hit Data");
    outPixelTree->SetAutoSave(0);
    outPixelTree->SetAutoFlush(50000);
    
    outPixelTree->Branch("event_id", &op_event_id, "event_id/I");
    outPixelTree->Branch("layer", &op_layer, "layer/I");
    outPixelTree->Branch("i", &op_i, "i/I");
    outPixelTree->Branch("j", &op_j, "j/I");
    outPixelTree->Branch("xi", &op_xi, "xi/D");
    outPixelTree->Branch("yi", &op_yi, "yi/D");
    outPixelTree->Branch("zi", &op_zi, "zi/D");
    outPixelTree->Branch("theta", &op_theta, "theta/D");
    outPixelTree->Branch("phi", &op_phi, "phi/D");
    outPixelTree->Branch("eta", &op_eta, "eta/D");
    outPixelTree->Branch("edep", &op_edep, "edep/D");
    outPixelTree->Branch("ADC", &op_ADC, "ADC/I");
    
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
        
        // Combine pixel hits: key = (layer, i, j)
        map<tuple<int,int,int>, Hit> pixelHitMap;
        
        // Add signal hits first
        auto sigIt = sigPixelEventIndex.find(sigEvt);
        if (sigIt != sigPixelEventIndex.end()) {
            for (Long64_t entry : sigIt->second) {
                sigPixelTree->GetEntry(entry);
                auto key = make_tuple(sp_layer, sp_i, sp_j);
                pixelHitMap[key] = {sp_xi, sp_yi, sp_zi, sp_theta, sp_phi, sp_eta, sp_edep, sp_ADC};
            }
        }
        
        // Add pileup hits (sum edep and ADC if overlapping)
        auto puIt = puPixelEventIndex.find(puEvt);
        if (puIt != puPixelEventIndex.end()) {
            for (Long64_t entry : puIt->second) {
                puPixelTree->GetEntry(entry);
                auto key = make_tuple(pp_layer, pp_i, pp_j);
                
                if (pixelHitMap.find(key) == pixelHitMap.end()) {
                    pixelHitMap[key] = {pp_xi, pp_yi, pp_zi, pp_theta, pp_phi, pp_eta, pp_edep, pp_ADC};
                } else {
                    pixelHitMap[key].edep += pp_edep;
                    pixelHitMap[key].ADC += pp_ADC;
                }
            }
        }
        
        // Write combined pixel hits
        for (auto& kv : pixelHitMap) {
            op_event_id = sigEvt;
            op_layer = get<0>(kv.first);
            op_i = get<1>(kv.first);
            op_j = get<2>(kv.first);
            op_xi = kv.second.xi;
            op_yi = kv.second.yi;
            op_zi = kv.second.zi;
            op_theta = kv.second.theta;
            op_phi = kv.second.phi;
            op_eta = kv.second.eta;
            op_edep = kv.second.edep;
            op_ADC = kv.second.ADC;
            outPixelTree->Fill();
        }
        
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
    }
    
    cout << "Writing output..." << endl;
    outPixelTree->Write();
    outEtaPhiTree->Write();
    
    fout->Close();
    fSignal->Close();
    fPileup->Close();
    
    cout << "Done! Output file: " << outputFile << endl;
    cout << "Trees in output:" << endl;
    cout << "  - Signal_GeneratorInfo (cloned from signal)" << endl;
    cout << "  - Signal_Pixel_CellWiseSegmentation (cloned from signal)" << endl;
    cout << "  - Signal_Eta_Phi_CellWiseSegmentation (cloned from signal)" << endl;
    cout << "  - Pileup_GeneratorInfo (cloned from pileup)" << endl;
    cout << "  - Pileup_Pixel_CellWiseSegmentation (cloned from pileup)" << endl;
    cout << "  - Pileup_Eta_Phi_CellWiseSegmentation (cloned from pileup)" << endl;
    cout << "  - EventMapping (signal event -> pileup event mapping)" << endl;
    cout << "  - Pixel_CellWiseSegmentation (combined signal + pileup)" << endl;
    cout << "  - Eta_Phi_CellWiseSegmentation (combined signal + pileup)" << endl;
}
