void digihit_matrix() {

    // open root file
    TFile *file = TFile::Open("Electron_Pt_025_Eta_185_Events_50_PU_000_Set01_Step2.root");
    if (!file || file->IsZombie()) {
        cout << "Error: Cannot open file!" << endl;
        return;
    }

    const int nEvents = 50;
    const int nLayers = 47;

    // ================= Pixel_CellWiseSegmentation =================

    // get the tree
    TTree *pixelTree = (TTree*)file->Get("Pixel_CellWiseSegmentation");
    if (pixelTree) {

        // branch variables
        Int_t event_id, layer;
        Double_t edep;

        pixelTree->SetBranchAddress("event_id", &event_id);
        pixelTree->SetBranchAddress("layer", &layer);
        pixelTree->SetBranchAddress("edep", &edep);

        Double_t pixelEnergyMatrix[nLayers + 1][nEvents];

        // initialise layers
        for (int lyr = 1; lyr <= nLayers; lyr++) {
            for (int evt = 0; evt < nEvents; evt++) {
                pixelEnergyMatrix[lyr][evt] = 0.0;
            }
        }

        // fill matrix
        Long64_t nEntries = pixelTree->GetEntries();
        for (Long64_t i = 0; i < nEntries; i++) {
            pixelTree->GetEntry(i);

            if (event_id >= 0 && event_id < nEvents &&
                layer >= 1 && layer <= nLayers) {

                pixelEnergyMatrix[layer][event_id] += edep;
            }
        }

        // write csv
        ofstream csvFile("pixel_matrix.csv");

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
                csvFile << "," << pixelEnergyMatrix[lyr][evt];
            }
            csvFile << endl;
        }

        // write total row
        csvFile << "Total";
        for (int evt = 0; evt < nEvents; evt++) {
            double sum = 0.0;
            for (int lyr = 1; lyr <= nLayers; lyr++) {
                sum += pixelEnergyMatrix[lyr][evt];
            }
            csvFile << "," << sum;
        }
        csvFile << endl;

        csvFile.close();
        cout << "pixel csv file created successfully" << endl;
        cout << "pixel entries processed: " << nEntries << endl;
    }



    // ================= Eta_Phi_CellWiseSegmentation =================

    // get the tree
    TTree *etaPhiTree = (TTree*)file->Get("Eta_Phi_CellWiseSegmentation");
    if (etaPhiTree) {

        // branch variables
        Int_t event_id, layer;
        Double_t edep;

        etaPhiTree->SetBranchAddress("event_id", &event_id);
        etaPhiTree->SetBranchAddress("layer", &layer);
        etaPhiTree->SetBranchAddress("edep", &edep);

        Double_t etaPhiEnergyMatrix[nLayers + 1][nEvents];

        // initialise layers
        for (int lyr = 1; lyr <= nLayers; lyr++) {
            for (int evt = 0; evt < nEvents; evt++) {
                etaPhiEnergyMatrix[lyr][evt] = 0.0;
            }
        }

        // fill matrix
        Long64_t nEntries = etaPhiTree->GetEntries();
        for (Long64_t i = 0; i < nEntries; i++) {
            etaPhiTree->GetEntry(i);

            if (event_id >= 0 && event_id < nEvents &&
                layer >= 1 && layer <= nLayers) {

                etaPhiEnergyMatrix[layer][event_id] += edep;
            }
        }

        // write csv
        ofstream csvFile("eta_phi_matrix.csv");

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
                csvFile << "," << etaPhiEnergyMatrix[lyr][evt];
            }
            csvFile << endl;
        }

        // write total row
        csvFile << "Total";
        for (int evt = 0; evt < nEvents; evt++) {
            double sum = 0.0;
            for (int lyr = 1; lyr <= nLayers; lyr++) {
                sum += etaPhiEnergyMatrix[lyr][evt];
            }
            csvFile << "," << sum;
        }
        csvFile << endl;

        csvFile.close();
        cout << "eta-phi csv file created successfully" << endl;
        cout << "eta-phi entries processed: " << nEntries << endl;
    }

    file->Close();
    cout << "done!" << endl;
}
