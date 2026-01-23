#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void automation(const char* file = "/persistent/data1/ms21080/Data/Electrons_Step1.txt",
                const char* outputPath = "/persistent/data1/ms21080/Data/Fixed_pT_Step2_Data/Electron_Step2") {

    // Load and compile the macro
    gROOT->ProcessLine(".L cellwise_segmentation.C+");

    ifstream f(file);
    string line;
    int fileCount = 0;

    cout << "========================================" << endl;
    cout << "Starting batch digitisation process..." << endl;
    cout << "========================================" << endl;

    while (getline(f, line)) {
        if (line.empty()) continue;

        fileCount++;

        int pos = line.find_last_of('/');
        string inputPath = line.substr(0, pos);
        string inputFile = line.substr(pos + 1);

        string outputFile = inputFile;
        outputFile.replace(outputFile.find("Step1"), 5, "Step2");

        string fullInputPath = line;
        string fullOutputPath = string(outputPath) + "/" + outputFile;

        cout << "\n[" << fileCount << "] Processing..." << endl;
        cout << "  Input:  " << inputFile << endl;
        cout << "  Output: " << outputFile << endl;
        cout << "  Input Path:  " << inputPath << endl;
        cout << "  Output Path: " << outputPath << endl;

        // Call using ProcessLine with proper escaping
        string cmd = Form("cellwise_segmentation(\"%s\", \"%s\")",
                         fullInputPath.c_str(), fullOutputPath.c_str());
        gROOT->ProcessLine(cmd.c_str());

        cout << "  ✓ Done!" << endl;
        cout << "--------------------------------------" << endl;
    }

    cout << "\n========================================" << endl;
    cout << "Batch Processing Complete!" << endl;
    cout << "Total files: " << fileCount << endl;
    cout << "========================================" << endl;
}
