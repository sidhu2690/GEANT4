#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void filter_automation(const char* file = "/persistent/data1/ms21080/Data/Positrons_Step1.txt",
                const char* outputPath = "/persistent/data1/ms21080/Data/Fixed_pT_Step1_Data/Positron_Step1_Filtered") {

    // Load and compile the macro
    gROOT->ProcessLine(".L filter.C+");   // if we do gROOT-> ProcessLine("1+2) ==> same as "root[0] 1+2"

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
        outputFile.replace(outputFile.find("Step1"), 5, "Step1_trimmed");

        string fullInputPath = line;
        string fullOutputPath = string(outputPath) + "/" + outputFile;

        cout << "\n[" << fileCount << "] Processing..." << endl;
        cout << "  Input:  " << inputFile << endl;
        cout << "  Output: " << outputFile << endl;
        cout << "  Input Path:  " << inputPath << endl;
        cout << "  Output Path: " << outputPath << endl;


        string cmd = Form("filter(\"%s\", \"%s\")",
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
