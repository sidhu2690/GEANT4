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
    
    // Constants (same as cellwise_segmentation)
    const int N_eta = 736;
    const int N_phi = 736;
    const double eta_min = 1.4;
    const double eta_max = 3.1;
    const double phi_min = 0.0;
    const double phi_max = 360.0;
    const double del_eta = (eta_max - eta_min) / N_eta;
    const double del_phi = (phi_max - phi_min) / N_phi;
    
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
    int skipped = 0;
    
    // Process each event
    for (int event_id : eventList) {
        
        // Get generator info for this event
        if (genIndex.find(event_id) == genIndex.end()) {
            skipped++;
            continue;
        }
        genTree->GetEntry(genIndex[event_id]);
        
        // Convert phi from [0, 2π] to [0, 360] degrees
        double phi_deg = gen_phi * (180.0 / M_PI);
        
        // Compute seed cell indices
        int ieta_seed = (int)round((fabs(gen_eta) - eta_min) / del_eta);
        int iphi_seed = (int)round((phi_deg - phi_min) / del_phi);
        
        // Skip if seed is outside valid range
        if (ieta_seed < 0 || ieta_seed >= N_eta || iphi_seed < 0 || iphi_seed >= N_phi) {
            skipped++;
            continue;
        }
        
        // Build energy map for this event
        vector<vector<double>> energy(N_eta, vector<double>(N_phi, 0.0));
        
        for (Long64_t entry : sigIndex[event_id]) {
            sigTree->GetEntry(entry);
            if (sig_ieta >= 0 && sig_ieta < N_eta && sig_iphi >= 0 && sig_iphi < N_phi) {
                energy[sig_ieta][sig_iphi] += sig_edep;
            }
        }
        
        // Compute E_total from max box (39x39 or less at edges)
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
            skipped++;
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
    cout << "Processed: " << processed << endl;
    cout << "Skipped: " << skipped << endl;
    cout << "Output: " << outputFile << endl;
    cout << "===============" << endl;
}
