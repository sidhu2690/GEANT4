#include <TFile.h>
#include <TTree.h>
#include <iostream>

using namespace std;

void filter(const char* inputFile,
                    const char* outputFile,
                    Double_t energyCut = 1e-5)  // MeV (default = 10 eV)
{

    // open input file
    TFile *inFile = TFile::Open(inputFile);
    if (!inFile || inFile->IsZombie()) {
        cout << "Error: cannot open input file: " << inputFile << endl;
        return;
    }

    // open output file
    TFile *outFile = new TFile(outputFile, "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        cout << "Error: cannot create output file: " << outputFile << endl;
        inFile->Close();
        return;
    }

    // Copy GeneratorInfo tree
    TTree *genTree = (TTree*)inFile->Get("GeneratorInfo");
    if (genTree) {
        outFile->cd();
        TTree *genClone = genTree->CloneTree(-1, "fast");
        genClone->Write();
    }

    TTree *inTree = (TTree*)inFile->Get("ParticleTracking");
    if (!inTree) {
        cout << "Error: ParticleTracking tree not found" << endl;
        outFile->Close();
        inFile->Close();
        return;
    }

    // Set branch address
    Double_t energy_deposited_MeV = 0.0;
    inTree->SetBranchAddress("energy_deposited_MeV",
                             &energy_deposited_MeV);

    // Clone tree structure only (no entries)
    outFile->cd();
    TTree *outTree = inTree->CloneTree(0);

    // Loop and filter
    Long64_t nEntries = inTree->GetEntries();
    Long64_t kept = 0;

    for (Long64_t i = 0; i < nEntries; ++i) {
        inTree->GetEntry(i);

        if (energy_deposited_MeV >= energyCut) {
            outTree->Fill();
            kept++;
        }
    }

    // Write and close
    outTree->Write();
    outFile->Close();
    inFile->Close();

    cout << "Filtered ROOT file created" << endl;
    cout << "Input file : " << inputFile << endl;
    cout << "Output file: " << outputFile << endl;
    cout << "Energy cut : " << energyCut << " MeV" << endl;
    cout << "Total entries: " << nEntries << endl;
    cout << "Kept entries : " << kept << endl;
}
