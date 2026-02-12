#include <map>
#include <vector>
#include <set>
#include <tuple>
#include <iostream>
#include <cmath>
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TROOT.h>

void addPileupToSignal(
    const char* signalFile = "Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root",
    const char* pileupFile = "PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root",
    const char* outputFile = "Electron_nPU_35_Pt_025_Eta_170_Events_2K_Set01_Step2.root",
    int nPU = 35,
    int classLabel = 0)
{
    using namespace std;

    // Constants for eta-phi grid
    const int N_eta = 736;
    const int N_phi = 736;
    const double eta_min = 1.4;
    const double eta_max = 3.1;
    const double del_eta = (eta_max - eta_min) / N_eta;
    const double del_phi = 360.0 / N_phi;
    const int max_f = 40;

    // Constants for validation tree
    const int N_layers = 47;
    const double MIP_MeV = 0.90;
    const double threshold_MeV = 0.1 * MIP_MeV;

    // =====================================================================
    // Open input files
    // =====================================================================
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

    // Get pileup trees (raw single-interaction pileup events)
    TTree* puGenTree = (TTree*)fPileup->Get("GeneratorInfo");
    TTree* puEtaPhiTree = (TTree*)fPileup->Get("Eta_Phi_CellWiseSegmentation");

    // =====================================================================
    // Build signal generator event index
    // =====================================================================
    cout << "Building signal generator event index..." << endl;
    map<int, Long64_t> sigGenEventIndex;
    {
        int event_id;
        sigGenTree->SetBranchAddress("event_id", &event_id);
        for (Long64_t i = 0; i < sigGenTree->GetEntries(); i++) {
            sigGenTree->GetEntry(i);
            sigGenEventIndex[event_id] = i;
        }
        sigGenTree->ResetBranchAddresses();
    }

    // =====================================================================
    // Build signal eta-phi event index
    // =====================================================================
    cout << "Building signal eta-phi event indices..." << endl;
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

    // =====================================================================
    // Build pileup eta-phi event index (raw single-interaction events)
    // =====================================================================
    cout << "Building pileup eta-phi event indices..." << endl;
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
    cout << "Found " << nPileupEvents << " raw pileup events." << endl;

    if (nPU > nPileupEvents) {
        cerr << "Error: nPU (" << nPU << ") exceeds available pileup events ("
             << nPileupEvents << ")!" << endl;
        return;
    }

    // =====================================================================
    // Create output file
    // =====================================================================
    TFile* fout = new TFile(outputFile, "RECREATE");

    // Clone signal GeneratorInfo
    cout << "Cloning Signal GeneratorInfo tree..." << endl;
    TTree* outSigGenTree = sigGenTree->CloneTree(-1, "fast");
    outSigGenTree->SetName("Signal_GeneratorInfo");
    outSigGenTree->SetTitle("Signal Generator Level Particle Data");
    outSigGenTree->Write();

    // Clone pileup GeneratorInfo
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

    // Clone pileup EtaPhi tree (raw single-interaction)
    cout << "Cloning Pileup EtaPhi tree..." << endl;
    TTree* outPuEtaPhiTree = puEtaPhiTree->CloneTree(-1, "fast");
    outPuEtaPhiTree->SetName("Pileup_Eta_Phi_CellWiseSegmentation");
    outPuEtaPhiTree->SetTitle("Pileup Eta-Phi Cell-wise Segmented Hit Data");
    outPuEtaPhiTree->Write();

    fout->cd();

    // =====================================================================
    // Random generator
    // =====================================================================
    TRandom3* rng = new TRandom3(0);

    // =====================================================================
    // Event mapping tree: stores which nPU pileup events were used per signal event
    // =====================================================================
    int map_event_id;
    vector<int>* evts_used = new vector<int>();
    TTree* evtMapTree = new TTree("EventMapping", "Signal to Pileup event mapping");
    evtMapTree->Branch("event_id", &map_event_id, "event_id/I");
    evtMapTree->Branch("pileup_events_used", &evts_used);

    // =====================================================================
    // Set up branches for signal generator tree (for YOLO labels)
    // =====================================================================
    int gen_event_id;
    double gen_eta, gen_phi;
    sigGenTree->SetBranchAddress("event_id", &gen_event_id);
    sigGenTree->SetBranchAddress("eta", &gen_eta);
    sigGenTree->SetBranchAddress("phi", &gen_phi);

    // =====================================================================
    // Set up branches for signal eta-phi tree
    // =====================================================================
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

    // =====================================================================
    // Set up branches for pileup eta-phi tree
    // =====================================================================
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

    // =====================================================================
    // Create combined eta-phi tree (signal + merged pileup)
    // =====================================================================
    int oe_event_id, oe_layer, oe_ieta, oe_iphi, oe_ADC;
    double oe_xi, oe_yi, oe_zi, oe_theta, oe_phi, oe_eta, oe_edep;

    TTree* outEtaPhiTree = new TTree("Eta_Phi_CellWiseSegmentation",
                                     "Combined Eta-Phi Cell-wise Segmented Hit Data");
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

    // =====================================================================
    // Create YOLO labels tree
    // =====================================================================
    int yolo_event_id, yolo_class_label, yolo_ieta_seed, yolo_iphi_seed;
    int yolo_f90_eta, yolo_f95_eta, yolo_f98_eta;
    int yolo_f90_phi, yolo_f95_phi, yolo_f98_phi;

    TTree* yoloTree = new TTree("YOLOLabels", "YOLO Object Detection Labels");
    yoloTree->Branch("event_id", &yolo_event_id, "event_id/I");
    yoloTree->Branch("class_label", &yolo_class_label, "class_label/I");
    yoloTree->Branch("ieta_seed", &yolo_ieta_seed, "ieta_seed/I");
    yoloTree->Branch("iphi_seed", &yolo_iphi_seed, "iphi_seed/I");
    yoloTree->Branch("f90_eta", &yolo_f90_eta, "f90_eta/I");
    yoloTree->Branch("f95_eta", &yolo_f95_eta, "f95_eta/I");
    yoloTree->Branch("f98_eta", &yolo_f98_eta, "f98_eta/I");
    yoloTree->Branch("f90_phi", &yolo_f90_phi, "f90_phi/I");
    yoloTree->Branch("f95_phi", &yolo_f95_phi, "f95_phi/I");
    yoloTree->Branch("f98_phi", &yolo_f98_phi, "f98_phi/I");

    // =====================================================================
    // Create validation tree
    // =====================================================================
    int val_event_id;
    int NPix[N_layers];
    float EMin[N_layers];
    float EMax[N_layers];
    float ETotal[N_layers];

    TTree* valTree = new TTree("validation_tree", "Validation Data per Layer");
    valTree->Branch("event_id", &val_event_id, "event_id/I");
    valTree->Branch("NPix", NPix, "NPix[47]/I");
    valTree->Branch("EMin", EMin, "EMin[47]/F");
    valTree->Branch("EMax", EMax, "EMax[47]/F");
    valTree->Branch("ETotal", ETotal, "ETotal[47]/F");

    // =====================================================================
    // Hit structure
    // =====================================================================
    struct Hit {
        double xi, yi, zi, theta, phi, eta, edep;
        int ADC;
    };

    // =====================================================================
    // Main event loop
    // =====================================================================
    cout << "Processing " << nSignalEvents << " signal events with nPU=" << nPU << "..." << endl;

    int processed = 0;
    for (int sigEvt : sigEventList) {
        processed++;
        if (processed % 10 == 0) {
            cout << "Event " << processed << "/" << nSignalEvents << endl;
        }

        // =================================================================
        // Step 1: Select nPU unique random pileup events for this signal event
        // =================================================================
        evts_used->clear();
        map_event_id = sigEvt;

        set<int> puIndices;
        while ((int)puIndices.size() < nPU) {
            puIndices.insert(rng->Integer(puEventList.size()));
        }

        for (int idx : puIndices) {
            evts_used->push_back(puEventList[idx]);
        }

        // =================================================================
        // Step 2: Build combined hit map (signal + merged nPU pileup events)
        // =================================================================
        map<tuple<int,int,int>, Hit> etaPhiHitMap;

        // Energy grid for YOLO labels (signal only, summed over layers)
        vector<vector<double>> energy(N_eta, vector<double>(N_phi, 0.0));

        // --- Add signal hits ---
        auto sigEtaIt = sigEtaPhiEventIndex.find(sigEvt);
        if (sigEtaIt != sigEtaPhiEventIndex.end()) {
            for (Long64_t entry : sigEtaIt->second) {
                sigEtaPhiTree->GetEntry(entry);
                auto key = make_tuple(se_layer, se_ieta, se_iphi);
                etaPhiHitMap[key] = {se_xi, se_yi, se_zi, se_theta,
                                     se_phi, se_eta, se_edep, se_ADC};

                // Fill energy grid for YOLO (signal only)
                if (se_ieta >= 0 && se_ieta < N_eta &&
                    se_iphi >= 0 && se_iphi < N_phi) {
                    energy[se_ieta][se_iphi] += se_edep;
                }
            }
        }

        // --- Add nPU pileup hits (merge on-the-fly) ---
        for (int puEvtId : *evts_used) {
            auto puEtaIt = puEtaPhiEventIndex.find(puEvtId);
            if (puEtaIt == puEtaPhiEventIndex.end()) continue;

            for (Long64_t entry : puEtaIt->second) {
                puEtaPhiTree->GetEntry(entry);
                auto key = make_tuple(pe_layer, pe_ieta, pe_iphi);

                if (etaPhiHitMap.find(key) == etaPhiHitMap.end()) {
                    etaPhiHitMap[key] = {pe_xi, pe_yi, pe_zi, pe_theta,
                                         pe_phi, pe_eta, pe_edep, pe_ADC};
                } else {
                    etaPhiHitMap[key].edep += pe_edep;
                    etaPhiHitMap[key].ADC += pe_ADC;
                }
            }
        }

        // =================================================================
        // Step 3: Write combined eta-phi hits
        // =================================================================
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

        // =================================================================
        // Step 4: Validation tree computation
        // =================================================================
        for (int l = 0; l < N_layers; l++) {
            NPix[l] = 0;
            EMin[l] = 1e9;
            EMax[l] = 0.0;
            ETotal[l] = 0.0;
        }

        for (auto& kv : etaPhiHitMap) {
            int layer = get<0>(kv.first);
            double edep = kv.second.edep;

            if (layer >= 1 && layer <= N_layers) {
                int idx = layer - 1;

                ETotal[idx] += edep;

                if (edep > threshold_MeV) {
                    NPix[idx]++;
                }

                if (edep < EMin[idx]) {
                    EMin[idx] = edep;
                }

                if (edep > EMax[idx]) {
                    EMax[idx] = edep;
                }
            }
        }

        for (int l = 0; l < N_layers; l++) {
            if (EMin[l] > 1e8) {
                EMin[l] = 0.0;
            }
        }

        val_event_id = sigEvt;
        valTree->Fill();

        // =================================================================
        // Step 5: YOLO labels computation
        // =================================================================
        if (sigGenEventIndex.find(sigEvt) != sigGenEventIndex.end()) {
            sigGenTree->GetEntry(sigGenEventIndex[sigEvt]);

            // Compute seed positions
            yolo_ieta_seed = (int)round((fabs(gen_eta) - eta_min) / del_eta);

            double phi_rad = gen_phi;
            if (phi_rad > M_PI) phi_rad -= 2.0 * M_PI;
            double phi_deg = 180.0 + phi_rad * (180.0 / M_PI);
            yolo_iphi_seed = (int)round(phi_deg / del_phi);

            // Compute eta profile
            vector<double> eta_sum(N_eta, 0.0);
            double e_total = 0.0;
            for (int i = 0; i < N_eta; i++) {
                for (int j = 0; j < N_phi; j++) {
                    eta_sum[i] += energy[i][j];
                    e_total += energy[i][j];
                }
            }

            // Compute phi profile
            vector<double> phi_sum(N_phi, 0.0);
            for (int j = 0; j < N_phi; j++) {
                for (int i = 0; i < N_eta; i++) {
                    phi_sum[j] += energy[i][j];
                }
            }

            // Initialize f values to max
            yolo_f90_eta = yolo_f95_eta = yolo_f98_eta = max_f;
            yolo_f90_phi = yolo_f95_phi = yolo_f98_phi = max_f;

            if (e_total > 0) {
                // Find f90, f95, f98 for eta
                for (int k = 0; k <= max_f; k++) {
                    double total = 0.0;
                    for (int delta = -k; delta <= k; delta++) {
                        int idx = yolo_ieta_seed + delta;
                        if (idx >= 0 && idx < N_eta) {
                            total += eta_sum[idx];
                        }
                    }
                    double frac = total / e_total;
                    if (yolo_f90_eta == max_f && frac >= 0.90) yolo_f90_eta = k;
                    if (yolo_f95_eta == max_f && frac >= 0.95) yolo_f95_eta = k;
                    if (yolo_f98_eta == max_f && frac >= 0.98) { yolo_f98_eta = k; break; }
                }

                // Find f90, f95, f98 for phi
                for (int k = 0; k <= max_f; k++) {
                    double total = 0.0;
                    for (int delta = -k; delta <= k; delta++) {
                        int idx = yolo_iphi_seed + delta;
                        if (idx >= 0 && idx < N_phi) {
                            total += phi_sum[idx];
                        }
                    }
                    double frac = total / e_total;
                    if (yolo_f90_phi == max_f && frac >= 0.90) yolo_f90_phi = k;
                    if (yolo_f95_phi == max_f && frac >= 0.95) yolo_f95_phi = k;
                    if (yolo_f98_phi == max_f && frac >= 0.98) { yolo_f98_phi = k; break; }
                }
            }

            yolo_event_id = sigEvt;
            yolo_class_label = classLabel;
            yoloTree->Fill();
        }

        // =================================================================
        // Step 6: Fill event mapping
        // =================================================================
        evtMapTree->Fill();

        if (processed % 10 == 0)
            cout << "Output event " << processed << " completed." << endl;
    }

    // =====================================================================
    // Write and close
    // =====================================================================
    cout << "Writing output..." << endl;
    outEtaPhiTree->Write();
    yoloTree->Write();
    valTree->Write();
    evtMapTree->Write();

    sigGenTree->ResetBranchAddresses();
    puEtaPhiTree->ResetBranchAddresses();
    sigEtaPhiTree->ResetBranchAddresses();

    fout->Close();
    fSignal->Close();
    fPileup->Close();

    delete evts_used;
    delete rng;

    cout << "Done! Output file: " << outputFile << endl;
    cout << "Trees in output:" << endl;
    cout << "  - Signal_GeneratorInfo (cloned from signal)" << endl;
    cout << "  - Pileup_GeneratorInfo (cloned from pileup)" << endl;
    cout << "  - Signal_Eta_Phi_CellWiseSegmentation (cloned from signal)" << endl;
    cout << "  - Pileup_Eta_Phi_CellWiseSegmentation (cloned from pileup)" << endl;
    cout << "  - EventMapping (signal event -> vector of " << nPU << " pileup event IDs used)" << endl;
    cout << "  - Eta_Phi_CellWiseSegmentation (combined signal + " << nPU << " merged pileup)" << endl;
    cout << "  - YOLOLabels (YOLO object detection labels with f90/f95/f98)" << endl;
    cout << "  - validation_tree (NPix, EMin, EMax, ETotal per layer)" << endl;
}
