#include <map>
#include <vector>
#include <set>
#include <tuple>
#include <iostream>

void createPileup(const char* inputFile = "PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root", 
                  const char* outputFile = "PileUp_nPU_10_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root", 
                  int nPU = 10) {
    
    using namespace std;
    
    // Open input file
    TFile* fin = TFile::Open(inputFile, "READ");
    if (!fin || fin->IsZombie()) {
        cerr << "Error opening input file!" << endl;
        return;
    }
    
    TTree* genTree = (TTree*)fin->Get("GeneratorInfo");
    TTree* pixelTree = (TTree*)fin->Get("Pixel_CellWiseSegmentation");
    TTree* etaPhiTree = (TTree*)fin->Get("Eta_Phi_CellWiseSegmentation");
    
    // Build event index for pixel tree
    cout << "Building event indices..." << endl;
    map<int, vector<Long64_t>> pixelEventIndex;
    map<int, vector<Long64_t>> etaPhiEventIndex;
    
    {
        int event_id;
        pixelTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < pixelTree->GetEntries(); i++) {
            pixelTree->GetEntry(i);
            pixelEventIndex[event_id].push_back(i);
        }
        pixelTree->ResetBranchAddresses();
    }
    
    {
        int event_id;
        etaPhiTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < etaPhiTree->GetEntries(); i++) {
            etaPhiTree->GetEntry(i);
            etaPhiEventIndex[event_id].push_back(i);
        }
        etaPhiTree->ResetBranchAddresses();
    }
    
    // Get list of unique event IDs
    vector<int> eventList;
    for (auto& kv : pixelEventIndex) {
        eventList.push_back(kv.first);
    }
    int nEvents = eventList.size();
    cout << "Found " << nEvents << " unique events" << endl;
    
    // Create output file
    TFile* fout = new TFile(outputFile, "RECREATE");
    
    // Clone GenInfo tree (all entries)
    cout << "Cloning GeneratorInfo tree..." << endl;
    TTree* outGenTree = genTree->CloneTree(-1, "fast");
    outGenTree->Write();
    
    // Set up input branches for pixel tree
    int p_event_id, p_layer, p_i, p_j, p_ADC;
    double p_xi, p_yi, p_zi, p_theta, p_phi, p_eta, p_edep;
    
    pixelTree->SetBranchAddress("event_id", &p_event_id);
    pixelTree->SetBranchAddress("layer", &p_layer);
    pixelTree->SetBranchAddress("i", &p_i);
    pixelTree->SetBranchAddress("j", &p_j);
    pixelTree->SetBranchAddress("xi", &p_xi);
    pixelTree->SetBranchAddress("yi", &p_yi);
    pixelTree->SetBranchAddress("zi", &p_zi);
    pixelTree->SetBranchAddress("theta", &p_theta);
    pixelTree->SetBranchAddress("phi", &p_phi);
    pixelTree->SetBranchAddress("eta", &p_eta);
    pixelTree->SetBranchAddress("edep", &p_edep);
    pixelTree->SetBranchAddress("ADC", &p_ADC);
    
    // Set up input branches for eta-phi tree
    int e_event_id, e_layer, e_ieta, e_iphi, e_ADC;
    double e_xi, e_yi, e_zi, e_theta, e_phi, e_eta, e_edep;
    
    etaPhiTree->SetBranchAddress("event_id", &e_event_id);
    etaPhiTree->SetBranchAddress("layer", &e_layer);
    etaPhiTree->SetBranchAddress("ieta", &e_ieta);
    etaPhiTree->SetBranchAddress("iphi", &e_iphi);
    etaPhiTree->SetBranchAddress("xi", &e_xi);
    etaPhiTree->SetBranchAddress("yi", &e_yi);
    etaPhiTree->SetBranchAddress("zi", &e_zi);
    etaPhiTree->SetBranchAddress("theta", &e_theta);
    etaPhiTree->SetBranchAddress("phi", &e_phi);
    etaPhiTree->SetBranchAddress("eta", &e_eta);
    etaPhiTree->SetBranchAddress("edep", &e_edep);
    etaPhiTree->SetBranchAddress("ADC", &e_ADC);
    
    // Create output pixel tree
    int op_event_id, op_layer, op_i, op_j, op_ADC;
    double op_xi, op_yi, op_zi, op_theta, op_phi, op_eta, op_edep;
    
    TTree* outPixelTree = new TTree("Pixel_CellWiseSegmentation", "Pixel Cell-wise Segmented Hit Data");
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
    
    // Create output eta-phi tree
    int oe_event_id, oe_layer, oe_ieta, oe_iphi, oe_ADC;
    double oe_xi, oe_yi, oe_zi, oe_theta, oe_phi, oe_eta, oe_edep;
    
    TTree* outEtaPhiTree = new TTree("Eta_Phi_CellWiseSegmentation", "Eta-Phi Cell-wise Segmented Hit Data");
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
    
    // Event mapping tree (stores which events were combined)
    int map_event_id;
    vector<int> evts_used;
    TTree* evtMapTree = new TTree("EventsUsed", "Pileup event mapping");
    evtMapTree->Branch("event_id", &map_event_id, "event_id/I");
    evtMapTree->Branch("evts_used", &evts_used);
    
    // Random generator
    TRandom3 rng(0);
    
    // Hit structure
    struct Hit {
        double xi, yi, zi, theta, phi, eta, edep;
        int ADC;
    };
    
    cout << "Processing " << nEvents << " output events (nPU = " << nPU << ")..." << endl;
    
    // Loop over output events
    for (int outEvt = 1; outEvt <= nEvents; outEvt++) {
        if (outEvt % 2000 == 0) 
            cout << "Event " << outEvt << "/" << nEvents << endl;
        
        evts_used.clear();
        map_event_id = outEvt;
        
        // random nPU
        set<int> puIndices;
        while ((int)puIndices.size() < nPU) {
            puIndices.insert(rng.Integer(nEvents));
        }

        for (int idx : puIndices) {
            evts_used.push_back(eventList[idx]);
        }

        // Combine pixel hits: key = (layer, i, j)
        map<tuple<int,int,int>, Hit> pixelHitMap;
        
        for (int evtId : evts_used) {
            auto it = pixelEventIndex.find(evtId);
            if (it == pixelEventIndex.end()) continue;
            
            for (Long64_t entry : it->second) {
                pixelTree->GetEntry(entry);
                auto key = make_tuple(p_layer, p_i, p_j);
                
                if (pixelHitMap.find(key) == pixelHitMap.end()) {
                    pixelHitMap[key] = {p_xi, p_yi, p_zi, p_theta, p_phi, p_eta, p_edep, p_ADC};
                } else {
                    pixelHitMap[key].edep += p_edep;
                    pixelHitMap[key].ADC += p_ADC;
                }
            }
        }
        
        // Write pixel hits
        for (auto& kv : pixelHitMap) {
            op_event_id = outEvt;
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
        
        for (int evtId : evts_used) {
            auto it = etaPhiEventIndex.find(evtId);
            if (it == etaPhiEventIndex.end()) continue;
            
            for (Long64_t entry : it->second) {
                etaPhiTree->GetEntry(entry);
                auto key = make_tuple(e_layer, e_ieta, e_iphi);
                
                if (etaPhiHitMap.find(key) == etaPhiHitMap.end()) {
                    etaPhiHitMap[key] = {e_xi, e_yi, e_zi, e_theta, e_phi, e_eta, e_edep, e_ADC};
                } else {
                    etaPhiHitMap[key].edep += e_edep;
                    etaPhiHitMap[key].ADC += e_ADC;
                }
            }
        }
        
        // Write eta-phi hits
        for (auto& kv : etaPhiHitMap) {
            oe_event_id = outEvt;
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
        
        evtMapTree->Fill();
    }
    
    cout << "Writing output..." << endl;
    outPixelTree->Write();
    outEtaPhiTree->Write();
    evtMapTree->Write();
    
    fout->Close();
    fin->Close();
    
    cout << "Done! Created " << nEvents << " pileup events with nPU=" << nPU << endl;
}
