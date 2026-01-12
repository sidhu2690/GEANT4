void rootToText{
    // Open the ROOT file
    TFile *fin = TFile::Open("pythia8_PU_Events_20K.root");
    if (!fin || fin->IsZombie()) {
        cout << "Error: Cannot open ROOT file!" << endl;
        return;
    }
    
    TTree *t = (TTree*)fin->Get("gen");
    if (!t) {
        cout << "Error: Cannot find tree 'gen'!" << endl;
        return;
    }
    
    // Function to check if particle is stable/detectable
    auto isStableParticle = [](int pdg) -> bool {
        int abs_pdg = abs(pdg);
        
        // Quarks (will hadronise - EXCLUDE)
        if (abs_pdg >= 1 && abs_pdg <= 6) return false;
        
        // Gluons (will hadronise - EXCLUDE)
        if (abs_pdg == 21) return false;
        
        // Short-lived resonances that decay before reaching the detector - EXCLUDE
        if (abs_pdg == 223) return false;  // omega meson
        if (abs_pdg == 213) return false;  // rho+
        if (abs_pdg == 113) return false;  // rho0
        if (abs_pdg == 221) return false;  // eta meson
        if (abs_pdg == 311) return false;  // K0 (generic neutral kaon, decays to K_S or K_L)
        if (abs_pdg == 323) return false;  // K*
        if (abs_pdg == 310) return false;  // K0s
        if (abs_pdg == 313) return false;  // K*0
        if (abs_pdg == 111) return false;  // pi0 (decays to 2 photons very quickly)
        if (abs_pdg == 2101) return false;  
        if (abs_pdg == 2203) return false;  
        if (abs_pdg == 433) return false;  
        if (abs_pdg == 3122) return false;  
        if (abs_pdg == 3212) return false;  
        if (abs_pdg == 431) return false;  
        if (abs_pdg == 3114) return false;  
        if (abs_pdg == 2224) return false;  
        if (abs_pdg == 2214) return false;  
        if (abs_pdg == 333) return false;  
        if (abs_pdg == 331) return false;  
        if (abs_pdg == 521) return false;  
        if (abs_pdg == 3222) return false;  
        if (abs_pdg == 2114) return false;  
        if (abs_pdg == 3112) return false;  
        if (abs_pdg == 3214) return false;  
        if (abs_pdg == 3322) return false;  
        if (abs_pdg == 1114) return false;  
        if (abs_pdg == 413) return false;  
        if (abs_pdg == 411) return false;  
        if (abs_pdg == 423) return false;  
        if (abs_pdg == 421) return false;  
        if (abs_pdg == 16) return false;  
        
        return true;  // By default, if not excluded above, consider it detectable
    };
    
    // Variables to read from the tree
    Int_t n_particle;
    Int_t pdg_id[10000];
    Float_t pT[10000];
    Float_t eta[10000];
    Float_t phi[10000];
    
    // Set branch addresses
    t->SetBranchAddress("n_particle", &n_particle);
    t->SetBranchAddress("pdg_id", pdg_id);
    t->SetBranchAddress("pT", pT);
    t->SetBranchAddress("eta", eta);
    t->SetBranchAddress("phi", phi);
    
    // Open output text file
    ofstream outfile("generated_data.txt");
    if (!outfile.is_open()) {
        cout << "Error: Cannot create output file!" << endl;
        fin->Close();
        return;
    }
    
    // Write header
    outfile << "Evt#   Cum_Tr#   PDG_ID     Pt         Phi       Theta       Eta" << endl;
    
    // Process 20000 events
    int events_to_process = 20000;
    
    cout << "Processing " << events_to_process << " events..." << endl;
    
    for (int evt = 0; evt < events_to_process; evt++) {
        t->GetEntry(evt);
        
        int cumTrack = 0;
        
        // Loop through all particles in this event
        for (int i = 0; i < n_particle; i++) {
            if (pT[i] > 0.3 && eta[i] >= 1.5 && eta[i] <= 3.1 && isStableParticle(pdg_id[i])) {   // we are only considering the positive eta values
                // Calculate theta from eta: theta = 2 * arctan(exp(-eta))
                double theta = 2.0 * atan(exp(-eta[i]));
                
                // Write to file with fixed-width formatting
                outfile << setw(3) << evt 
                        << setw(9) << cumTrack
                        << setw(9) << pdg_id[i]
                        << setw(11) << fixed << setprecision(4) << pT[i]
                        << setw(11) << fixed << setprecision(4) << phi[i]
                        << setw(11) << fixed << setprecision(4) << theta
                        << setw(11) << fixed << setprecision(4) << eta[i]
                        << endl;
                cumTrack++;
            }
        }
        
        cout << "Event " << evt << ": " << cumTrack << " particles written" << endl;
    }
    
    outfile.close();
    fin->Close();
    
    cout << "\nDone! Output saved to generated_data.txt" << endl;
    cout << "Processed " << events_to_process << " events from ROOT file" << endl;
}
