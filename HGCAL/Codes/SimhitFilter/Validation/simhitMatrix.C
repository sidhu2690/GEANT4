
void simhitMatrix() {
    // open root file
    TFile *file = TFile::Open("Electron_Pt_025_Eta_185_Events_50_PU_000_Set01_Step1.root");
    if (!file || file->IsZombie()) {
        cout << "Error: Cannot open file!" << endl;
        return;
    }
    // get the tree
    TTree *tree = (TTree*)file->Get("ParticleTracking");
    if (!tree) {
        cout << "Error: Cannot find ParticleTracking tree!" << endl;
        return;
    }
    // branch variables
    Int_t eventID, layer;
    Double_t energy_deposited_MeV;
    tree->SetBranchAddress("eventID", &eventID);
    tree->SetBranchAddress("layer", &layer);
    tree->SetBranchAddress("energy_deposited_MeV", &energy_deposited_MeV);
    const int nEvents = 50;
    const int nLayers = 47;
    Double_t energyMatrix[nLayers + 1][nEvents];
    // initialise layers
    for (int lyr = 1; lyr <= nLayers; lyr++) {
        for (int evt = 0; evt < nEvents; evt++) {
            energyMatrix[lyr][evt] = 0.0;
        }
    }
    // fill matrix
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        if (eventID >= 0 && eventID < nEvents &&
            layer >= 1 && layer <= nLayers) {
            energyMatrix[layer][eventID] += energy_deposited_MeV;
        }
    }
    // write csv
    ofstream csvFile("SimHit_energy_matrix.csv");
    // header of csv
    csvFile << "Layer";
    for (int evt = 0; evt < nEvents; evt++) {
        csvFile << ",Event_" << evt;
    }
    csvFile << endl;
    // fill it
    for (int lyr = 1; lyr <= nLayers; lyr++) {
        csvFile << lyr;
        for (int evt = 0; evt < nEvents; evt++) {
            csvFile << "," << energyMatrix[lyr][evt];
        }
        csvFile << endl;
    }
    // write total row
    csvFile << "Total";
    for (int evt = 0; evt < nEvents; evt++) {
        double sum = 0.0;
        for (int lyr = 1; lyr <= nLayers; lyr++) {
            sum += energyMatrix[lyr][evt];
        }
        csvFile << "," << sum;
    }
    csvFile << endl;
    csvFile.close();
    file->Close();
    cout << "csv file created successfully" << endl;
    cout << "total entries processed: " << nEntries << endl;
}

