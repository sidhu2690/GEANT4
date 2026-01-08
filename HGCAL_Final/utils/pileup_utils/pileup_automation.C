#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void pileup_automation(
    const char* file = "/persistent/data1/ms21080/daily/pileup_files/20k/addition/Pileup_Step2.txt",
    const char* outputPath = "/persistent/data1/ms21080/daily/pileup_files/20k/addition/",
    int nPU = 10,
    int nOutputEvents = 5
) {
    // Load and compile the pileup macro
    gROOT->ProcessLine(".L PileupAddition.C+");

    ifstream f(file);
    if (!f.is_open()) {
        cout << "Error: cannot open input file list!" << endl;
        return;
    }

    string line;
    int fileCount = 0;

    cout << "========================================" << endl;
    cout << "Starting pileup automation (nPU = " << nPU << ")" << endl;
    cout << "========================================" << endl;

    while (getline(f, line)) {
        if (line.empty()) continue;

        fileCount++;

        // Extract filename
        int pos = line.find_last_of('/');
        string inputFile = line.substr(pos + 1);

        // Construct output filename
        string outputFile = inputFile;

        // Insert nPU_<nPU>_ right after "PileUp_"
        outputFile.insert(
            string("PileUp_").length(),
            Form("nPU_%d_", nPU)
        );

        string fullInputPath  = line;
        string fullOutputPath = string(outputPath) + "/" + outputFile;

        cout << "\n[" << fileCount << "] Processing" << endl;
        cout << "  Input : " << inputFile << endl;
        cout << "  Output: " << outputFile << endl;

        // Call createPileup
        string cmd = Form(
            "PileupAddition(\"%s\", \"%s\", %d, %d)",
            fullInputPath.c_str(),
            fullOutputPath.c_str(),
            nPU,
            nOutputEvents
        );

        gROOT->ProcessLine(cmd.c_str());

        cout << "  ✓ Done!" << endl;
        cout << "--------------------------------------" << endl;
    }

    cout << "\n========================================" << endl;
    cout << "Pileup addition completed!" << endl;
    cout << "Total files processed: " << fileCount << endl;
    cout << "========================================" << endl;
}

