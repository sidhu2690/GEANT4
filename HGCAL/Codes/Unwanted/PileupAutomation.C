#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

void pileupAutomation(
    const char* file = "/persistent/data1/ms21080/Data/Pileup/PileUp_Step2.txt",
    const char* outputPath = "/persistent/data1/ms21080/Data/Pileup/Pileup_Addition/",
    int nOutputEvents = 20000
) {
    
    gROOT->ProcessLine(".L createPileup.C+");
    
    ifstream f(file);
    string line;
    int count = 0;
    
    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        // Find " - " separator
        size_t dashPos = line.find(" - ");
        
        // Get input path (before " - ")
        string inputPath = line.substr(0, dashPos);
        
        // Get nPU string (after " - ")
        string nPU_string = line.substr(dashPos + 3);
        
        // Convert nPU string to integer
        int nPU;
        stringstream ss(nPU_string);
        ss >> nPU;
        
        // Get just the filename from full path
        size_t slashPos = inputPath.find_last_of('/');
        string inputFile = inputPath.substr(slashPos + 1);
        
        // Create output filename by inserting nPU info
        string outputFile = inputFile;
        string nPU_insert = Form("nPU_%d_", nPU);
        outputFile.insert(7, nPU_insert);  // Insert after "PileUp_"
        
        // Create full output path
        string fullOutputPath = string(outputPath) + "/" + outputFile;
        
        cout << "\n[" << ++count << "] Processing: nPU=" << nPU << endl;
        cout << "    Input : " << inputFile << endl;
        cout << "    Output: " << outputFile << endl;
        
        // Call createPileup function
        string command = Form("createPileup(\"%s\", \"%s\", %d, %d)",
                             inputPath.c_str(), 
                             fullOutputPath.c_str(), 
                             nPU, 
                             nOutputEvents);
        
        gROOT->ProcessLine(command.c_str());
        
        cout << "    Done!" << endl;
    }
    
    cout << "\n========================================" << endl;
    cout << "All jobs completed! Total: " << count << endl;
    cout << "========================================\n" << endl;
}
