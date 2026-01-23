#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void electron_var_pileup_automation(
    const char* electronList =
        "/persistent/data1/ms21080/Data/Signal_PileUp_Data/job/electron/electron_files.txt",

    const char* pileupList =
        "/persistent/data1/ms21080/Data/Signal_PileUp_Data/job/PileUp.txt",

    const char* outputDir =
        "/persistent/data1/ms21080/Data/Signal_PileUp_Data/Electron_Var_Pileup",

    int classLabel = 0   // electron
) {

    gROOT->ProcessLine(".L addPileupToSignal.C+");

    ifstream fe(electronList);
    ifstream fp(pileupList);

    if (!fe.is_open() || !fp.is_open()) {
        cerr << "Error opening input txt files!" << endl;
        return;
    }

    cout << "==================================================" << endl;
    cout << " Electron × Pileup automation started" << endl;
    cout << "==================================================" << endl;

    string electronFile, pileupFile;
    int jobCount = 0;

    // --------------------------------------------------
    // Loop over ELECTRON files (outer loop)
    // --------------------------------------------------
    while (getline(fe, electronFile)) {
        if (electronFile.empty()) continue;

        // Extract electron filename
        int posE = electronFile.find_last_of('/');
        string eleFileName = electronFile.substr(posE + 1);

        // --------------------------------------------------
        // Reset pileup file stream for each electron
        // --------------------------------------------------
        fp.clear();
        fp.seekg(0, ios::beg);

        // --------------------------------------------------
        // Loop over PILEUP files (inner loop)
        // --------------------------------------------------
        while (getline(fp, pileupFile)) {
            if (pileupFile.empty()) continue;

            jobCount++;

            // Extract nPU tag (e.g. nPU_035)
            size_t puPos = pileupFile.find("nPU_");
            string nPUtag = pileupFile.substr(puPos, 7);

            // Build output filename
            string outFile = eleFileName;

            // Replace nPU_000 → nPU_XXX
            size_t posReplace = outFile.find("nPU_000");
            if (posReplace != string::npos) {
                outFile.replace(posReplace, 7, nPUtag);
            }

            string fullOutPath = string(outputDir) + "/" + outFile;

            // --------------------------------------------------
            // Logging
            // --------------------------------------------------
            cout << "\n[" << jobCount << "] Processing" << endl;
            cout << "  Electron : " << eleFileName << endl;
            cout << "  Pileup   : " << nPUtag << endl;
            cout << "  Output   : " << fullOutPath << endl;

            // --------------------------------------------------
            // Call addPileupToSignal
            // --------------------------------------------------
            string cmd = Form(
                "addPileupToSignal(\"%s\", \"%s\", \"%s\", %d)",
                electronFile.c_str(),
                pileupFile.c_str(),
                fullOutPath.c_str(),
                classLabel
            );

            gROOT->ProcessLine(cmd.c_str());

            cout << "  ✓ Done" << endl;
            cout << "----------------------------------------------" << endl;
        }
    }

    cout << "\n==================================================" << endl;
    cout << " Automation finished successfully" << endl;
    cout << " Total outputs created: " << jobCount << endl;
    cout << "==================================================" << endl;
}

