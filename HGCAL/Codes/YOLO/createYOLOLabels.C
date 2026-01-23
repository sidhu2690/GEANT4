#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <TFile.h>
#include <TTree.h>

void createYOLOLabels(const char* inputFile = "Electron_nPU_35_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root",
                      const char* outputFile = "yolo_labels.txt",
                      double containment_threshold = 0.995,
                      int max_box_n = 20,
                      int class_id = 0) {
    
    using namespace std;
    
    // Constants (matching segmentation)
    const int N_eta = 736;
    const int N_phi = 736;
    const double eta_min = 1.4;
    const double eta_max = 3.1;
    const double del_eta = (eta_max - eta_min) / N_eta;
    const double del_phi = 360.0 / N_phi;  // in degrees
    
    cout << "=== YOLO Label Creator ===" << endl;
    cout << "Input file: " << inputFile << endl;
    cout << "Output file: " << outputFile << endl;
    cout << "Containment threshold: " << containment_threshold << endl;
    cout << "Max box n: " << max_box_n << endl;
    cout << endl;
    
    // Open input file
    TFile* fin = TFile::Open(inputFile, "READ");
    if (!fin || fin->IsZombie()) {
        cerr << "Error: Cannot open input file!" << endl;
        return;
    }
    
    // Get trees
    TTree* genTree = (TTree*)fin->Get("Signal_GeneratorInfo");
    TTree* sigTree = (TTree*)fin->Get("Signal_Eta_Phi_CellWiseSegmentation");
    
    if (!genTree || !sigTree) {
        cerr << "Error: Missing required trees!" << endl;
        fin->Close();
        return;
    }
    
    // Build generator info index: event_id -> entry number
    map<int, Long64_t> genIndex;
    int evt;
    genTree->SetBranchAddress("event_id", &evt);
    for (Long64_t i = 0; i < genTree->GetEntries(); i++) {
        genTree->GetEntry(i);
        genIndex[evt] = i;
    }
    genTree->ResetBranchAddresses();
    
    // Build signal tree index: event_id -> list of entry numbers
    map<int, vector<Long64_t>> sigIndex;
    sigTree->SetBranchAddress("event_id", &evt);
    for (Long64_t i = 0; i < sigTree->GetEntries(); i++) {
        sigTree->GetEntry(i);
        sigIndex[evt].push_back(i);
    }
    sigTree->ResetBranchAddresses();
    
    // Set up branches for reading
    double gen_eta, gen_phi;
    genTree->SetBranchAddress("event_id", &evt);
    genTree->SetBranchAddress("eta", &gen_eta);
    genTree->SetBranchAddress("phi", &gen_phi);
    
    int sig_ieta, sig_iphi;
    double sig_edep;
    sigTree->SetBranchAddress("ieta", &sig_ieta);
    sigTree->SetBranchAddress("iphi", &sig_iphi);
    sigTree->SetBranchAddress("edep", &sig_edep);
    
    // Get list of events to process
    vector<int> eventList;
    for (auto& kv : sigIndex) {
        eventList.push_back(kv.first);
    }
    
    // Open output file
    ofstream outFile(outputFile);
    outFile << "# event_id class_id x_center y_center width height" << endl;
    
    cout << "Processing " << eventList.size() << " events..." << endl;
    
    int processed = 0;
    int skipped_no_gen = 0;
    int skipped_out_of_range = 0;
    int skipped_no_energy = 0;
    
    // Process each event
    for (int event_id : eventList) {
        
        // Get generator info for this event
        if (genIndex.find(event_id) == genIndex.end()) {
            skipped_no_gen++;
            continue;
        }
        genTree->GetEntry(genIndex[event_id]);
        
        // =====================================================
        // ieta_seed: same as segmentation
        // =====================================================

        int ieta_seed = (int)round((fabs(gen_eta) - eta_min) / del_eta);
        

        // =====================================================
        // iphi_seed: directly invert segmentation formula
        // 
        // Segmentation used:
        //   phi_rad = atan2(yi, xi)                    // range: (-π, +π]
        //   phi_deg = 180 + phi_rad * (180/π)         // range: [0, 360)
        //   iphi = round(phi_deg / del_phi)
        //
        // gen_phi is in [0, 2π], so convert to (-π, +π] first
        // =====================================================


        double phi_rad = gen_phi;
        if (phi_rad > M_PI) phi_rad -= 2.0 * M_PI;  // convert [0,2π] → (-π,+π]
        
        double phi_deg = 180.0 + phi_rad * (180.0 / M_PI);
        int iphi_seed = (int)round(phi_deg / del_phi)
        
        // Skip if seed is outside valid range
        if (ieta_seed < 0 || ieta_seed >= N_eta || iphi_seed < 0 || iphi_seed >= N_phi) {
            skipped_out_of_range++;
            continue;
        }
        
        // Build energy map for this event
        // Note: sig_iphi is 1-indexed in the tree, convert to 0-indexed
        vector<vector<double>> energy(N_eta, vector<double>(N_phi, 0.0));
        
        for (Long64_t entry : sigIndex[event_id]) {
            sigTree->GetEntry(entry);
            int iphi_0idx = sig_iphi - 1;  // convert 1-indexed to 0-indexed
            if (sig_ieta >= 0 && sig_ieta < N_eta && iphi_0idx >= 0 && iphi_0idx < N_phi) {
                energy[sig_ieta][iphi_0idx] += sig_edep;
            }
        }
        
        // Compute E_total from max box
        int hw_max = max_box_n - 1;
        double E_total = 0.0;
        
        for (int di = -hw_max; di <= hw_max; di++) {
            for (int dj = -hw_max; dj <= hw_max; dj++) {
                int i = ieta_seed + di;
                int j = iphi_seed + dj;
                if (i >= 0 && i < N_eta && j >= 0 && j < N_phi) {
                    E_total += energy[i][j];
                }
            }
        }
        
        // Skip if no energy
        if (E_total <= 0) {
            skipped_no_energy++;
            continue;
        }
        
        // Find smallest box that contains threshold fraction of energy
        int final_n = max_box_n;
        
        for (int n = 1; n <= max_box_n; n++) {
            int hw = n - 1;
            double E_box = 0.0;
            
            for (int di = -hw; di <= hw; di++) {
                for (int dj = -hw; dj <= hw; dj++) {
                    int i = ieta_seed + di;
                    int j = iphi_seed + dj;
                    if (i >= 0 && i < N_eta && j >= 0 && j < N_phi) {
                        E_box += energy[i][j];
                    }
                }
            }
            
            if (E_box / E_total >= containment_threshold) {
                final_n = n;
                break;
            }
        }
        
        // Compute final box boundaries (clipped to valid range)
        int hw = final_n - 1;
        int ieta_lo = max(0, ieta_seed - hw);
        int ieta_hi = min(N_eta - 1, ieta_seed + hw);
        int iphi_lo = max(0, iphi_seed - hw);
        int iphi_hi = min(N_phi - 1, iphi_seed + hw);
        
        // Convert to YOLO format (normalized 0-1)
        double x_center = (iphi_lo + iphi_hi + 1) / (2.0 * N_phi);
        double y_center = (ieta_lo + ieta_hi + 1) / (2.0 * N_eta);
        double width = (double)(iphi_hi - iphi_lo + 1) / N_phi;
        double height = (double)(ieta_hi - ieta_lo + 1) / N_eta;
        
        // Write to output file
        outFile << event_id << " " << class_id << " "
                << fixed << setprecision(6)
                << x_center << " " << y_center << " "
                << width << " " << height << endl;
        
        processed++;
        
        if (processed % 500 == 0) {
            cout << "Processed " << processed << " events..." << endl;
        }
    }
    
    // Cleanup
    outFile.close();
    genTree->ResetBranchAddresses();
    sigTree->ResetBranchAddresses();
    fin->Close();
    
    // Summary
    cout << "\n=== Summary ===" << endl;
    cout << "Total events: " << eventList.size() << endl;
    cout << "Processed: " << processed << endl;
    cout << "Skipped (no gen info): " << skipped_no_gen << endl;
    cout << "Skipped (out of range): " << skipped_out_of_range << endl;
    cout << "Skipped (no energy): " << skipped_no_energy << endl;
    cout << "Output: " << outputFile << endl;
    cout << "===============" << endl;
}
