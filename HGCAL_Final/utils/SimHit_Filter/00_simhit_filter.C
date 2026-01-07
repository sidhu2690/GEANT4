void filter_simhits() {

    // open input file
    TFile *inFile = TFile::Open(
        "Electron_Pt_025_Eta_185_Events_50_PU_000_Set01_Step1.root"
    );
    if (!inFile || inFile->IsZombie()) {
        cout << "Error: cannot open input file" << endl;
        return;
    }

    // create output file
    TFile *outFile = new TFile(
        "Electron_Pt_025_Eta_185_Events_50_PU_000_Set01_Step1_filtered.root",
        "RECREATE"
    );

    // copy GeneratorInfo tree 

    TTree *genTree = (TTree*)inFile->Get("GeneratorInfo");
    if (genTree) {
        outFile->cd();
        TTree *genClone = genTree->CloneTree(-1, "fast");
        genClone->Write();
    }

    // filter ParticleTracking tree 

    TTree *inTree = (TTree*)inFile->Get("ParticleTracking");
    if (!inTree) {
        cout << "Error: ParticleTracking tree not found" << endl;
        outFile->Close();
        inFile->Close();
        return;
    }

    // branch variable
    Double_t energy_deposited_MeV;
    inTree->SetBranchAddress("energy_deposited_MeV", &energy_deposited_MeV);

    // clone tree structure only (no entries)
    outFile->cd();
    TTree *outTree = inTree->CloneTree(0);

    // energy threshold (10 eV)
    const Double_t energyCut = 1e-5; // MeV

    Long64_t nEntries = inTree->GetEntries();
    Long64_t kept = 0;

    for (Long64_t i = 0; i < nEntries; i++) {
        inTree->GetEntry(i);

        if (energy_deposited_MeV >= energyCut) {
            outTree->Fill();
            kept++;
        }
    }

    // write output
    outTree->Write();
    outFile->Close();
    inFile->Close();

    cout << "Filtered ROOT file created" << endl;
    cout << "Total entries: " << nEntries << endl;
    cout << "Kept entries : " << kept << endl;
}
