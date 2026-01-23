#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <vector>

void analyze_cellwise_data(const char* inputFile = "hgcal_output_processed.root") {
    
    // Set ROOT style
    gStyle->SetOptFit(0);
    gStyle->SetOptStat(0);
    
    // Open input file
    TFile *fInput = TFile::Open(inputFile, "READ");
    if (!fInput || fInput->IsZombie()) {
        std::cerr << "Error: Cannot open file " << inputFile << std::endl;
        return;
    }

    // Get the CellWiseSegmentation tree
    TTree *cellTree = (TTree*)fInput->Get("CellWiseSegmentation");
    if (!cellTree) {
        std::cerr << "Error: Cannot find CellWiseSegmentation tree" << std::endl;
        fInput->Close();
        return;
    }
    
    // Set up branch addresses
    Int_t event_id, layer, i_cell, j_cell;
    Double_t xi, yi, zi, theta, phi, eta, edep;
    
    cellTree->SetBranchAddress("event_id", &event_id);
    cellTree->SetBranchAddress("layer", &layer);
    cellTree->SetBranchAddress("i", &i_cell);
    cellTree->SetBranchAddress("j", &j_cell);
    cellTree->SetBranchAddress("xi", &xi);
    cellTree->SetBranchAddress("yi", &yi);
    cellTree->SetBranchAddress("zi", &zi);
    cellTree->SetBranchAddress("theta", &theta);
    cellTree->SetBranchAddress("phi", &phi);
    cellTree->SetBranchAddress("eta", &eta);
    cellTree->SetBranchAddress("edep", &edep);
    
    const Int_t nLayers = 47;
    
    // Data structures
    std::map<int, std::vector<Double_t>> hitsPerLayer; // layer -> [edep values]
    std::map<int, std::set<int>> eventsPerLayer; // layer -> set of unique event IDs
    
    // Global energy distribution histogram (log scale)
    TH1D *hEnergyTotal = new TH1D("hEnergyTotal", 
                                   "Energy Deposition Distribution;Energy Deposited (MeV);Entries",
                                   100, 0, 1);
    
    // Read all entries
    std::cout << "Processing entries..." << std::endl;
    Long64_t nEntries = cellTree->GetEntries();
    
    for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++) {
        cellTree->GetEntry(iEntry);
        
        if (iEntry % 100000 == 0) {
            std::cout << "Processing entry " << iEntry << " / " << nEntries << std::endl;
        }
        
        if (layer >= 1 && layer <= nLayers && edep > 0) {
            hitsPerLayer[layer].push_back(edep);
            eventsPerLayer[layer].insert(event_id);
            hEnergyTotal->Fill(edep);
        }
    }
    
    std::cout << "Analysis complete. Creating plots..." << std::endl;
    
    // Arrays for graphs
    Double_t layerNum[nLayers];
    Double_t meanEnergyPerLayer[nLayers];
    Double_t totalHitsPerLayer[nLayers];
    Int_t validLayers = 0;
    
    // Calculate mean energy and total hits per layer
    std::ofstream csvFile("cellwise_analysis.csv");
    csvFile << "Layer,Total_Hits,Num_Events,Mean_Energy_MeV,SD_Energy_MeV\n";
    
    for (int iLayer = 1; iLayer <= nLayers; iLayer++) {
        if (hitsPerLayer.find(iLayer) == hitsPerLayer.end() || hitsPerLayer[iLayer].empty()) {
            csvFile << iLayer << ",0,0,0,0\n";
            continue;
        }
        
        auto &hits = hitsPerLayer[iLayer];
        Int_t numHits = hits.size();
        Int_t numEvents = eventsPerLayer[iLayer].size();
        
        // Calculate mean and standard deviation
        Double_t sum = 0;
        Double_t sumSq = 0;
        for (auto &e : hits) {
            sum += e;
            sumSq += e * e;
        }
        
        Double_t mean = sum / numHits;
        Double_t variance = (sumSq / numHits) - (mean * mean);
        Double_t sd = (variance > 0) ? std::sqrt(variance) : 0;
        
        // Store for plotting
        layerNum[validLayers] = iLayer;
        totalHitsPerLayer[validLayers] = numHits;
        meanEnergyPerLayer[validLayers] = mean;
        validLayers++;
        
        csvFile << iLayer << "," << numHits << "," << numEvents << "," 
                << mean << "," << sd << "\n";
        
        std::cout << "Layer " << iLayer << ": " << numHits << " hits, mean E = " 
                  << mean << " MeV" << std::endl;
    }
    
    csvFile.close();
    
    // ==================== Plot 1: Global Energy Distribution (Log Scale) ====================
    TCanvas *c1 = new TCanvas("c1", "Energy Distribution", 800, 600);
    c1->cd();
    c1->SetLogy(1);
    c1->SetGrid();
    
    hEnergyTotal->SetLineColor(kBlue+1);
    hEnergyTotal->SetLineWidth(2);
    hEnergyTotal->SetFillColor(kBlue-10);
    hEnergyTotal->Draw("HIST");
    
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.04);
    latex.SetTextColor(kBlack);
    latex.SetTextFont(42);
    latex.DrawLatex(0.15, 0.85, Form("Total Entries = %d", (int)hEnergyTotal->GetEntries()));
    latex.DrawLatex(0.15, 0.80, Form("Mean = %.4f MeV", hEnergyTotal->GetMean()));
    latex.DrawLatex(0.15, 0.75, Form("RMS = %.4f MeV", hEnergyTotal->GetRMS()));
    
    c1->Update();
    c1->SaveAs("energy_distribution_total.pdf");
    c1->SaveAs("energy_distribution_total.png");
    
    // ==================== Plot 2: Mean Energy vs Layer ====================
    TCanvas *c2 = new TCanvas("c2", "Mean Energy vs Layer", 1000, 600);
    c2->cd();
    c2->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    
    TGraph *grEnergy = new TGraph(validLayers, layerNum, meanEnergyPerLayer);
    grEnergy->SetTitle("Mean Energy Deposited vs Layer;Layer Number;Mean Energy (MeV)");
    grEnergy->SetMarkerStyle(21);
    grEnergy->SetMarkerSize(1.0);
    grEnergy->SetMarkerColor(kRed + 1);
    grEnergy->SetLineColor(kRed + 1);
    grEnergy->SetLineWidth(2);
    grEnergy->Draw("APL");
    
    c2->Update();
    c2->SaveAs("mean_energy_vs_layer.pdf");
    c2->SaveAs("mean_energy_vs_layer.png");
    
    // ==================== Plot 3: Hit Multiplicity vs Layer ====================
    TCanvas *c3 = new TCanvas("c3", "Hit Multiplicity vs Layer", 1000, 600);
    c3->cd();
    c3->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    
    TGraph *grHits = new TGraph(validLayers, layerNum, totalHitsPerLayer);
    grHits->SetTitle("Hit Multiplicity vs Layer;Layer Number;Total Hits");
    grHits->SetMarkerStyle(20);
    grHits->SetMarkerSize(1.0);
    grHits->SetMarkerColor(kBlue + 1);
    grHits->SetLineColor(kBlue + 1);
    grHits->SetLineWidth(2);
    grHits->Draw("APL");
    
    c3->Update();
    c3->SaveAs("hit_multiplicity_vs_layer.pdf");
    c3->SaveAs("hit_multiplicity_vs_layer.png");
    
    // ==================== Combined Plot ====================
    TCanvas *c4 = new TCanvas("c4", "Combined Analysis", 1400, 1000);
    c4->Divide(2, 2);
    
    // Top left: Energy distribution
    c4->cd(1);
    gPad->SetLogy(1);
    gPad->SetGrid();
    hEnergyTotal->Draw("HIST");
    latex.DrawLatex(0.15, 0.85, Form("Entries = %d", (int)hEnergyTotal->GetEntries()));
    latex.DrawLatex(0.15, 0.80, Form("Mean = %.4f MeV", hEnergyTotal->GetMean()));
    
    // Top right: Mean energy vs layer
    c4->cd(2);
    gPad->SetGrid();
    grEnergy->Draw("APL");
    
    // Bottom left: Hit multiplicity vs layer
    c4->cd(3);
    gPad->SetGrid();
    grHits->Draw("APL");
    
    // Bottom right: Empty or can add another plot
    c4->cd(4);
    gPad->SetGrid();
    TLatex info;
    info.SetTextSize(0.04);
    info.SetTextAlign(22);
    info.DrawLatexNDC(0.5, 0.6, "CellWise Segmentation Analysis");
    info.DrawLatexNDC(0.5, 0.5, Form("Total Entries: %lld", nEntries));
    info.DrawLatexNDC(0.5, 0.4, Form("Layers: %d", validLayers));
    
    c4->Update();
    c4->SaveAs("combined_analysis.pdf");
    c4->SaveAs("combined_analysis.png");
    
    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Processing complete!" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "Output files created:" << std::endl;
    std::cout << "  - energy_distribution_total.pdf/png (log scale)" << std::endl;
    std::cout << "  - mean_energy_vs_layer.pdf/png" << std::endl;
    std::cout << "  - hit_multiplicity_vs_layer.pdf/png" << std::endl;
    std::cout << "  - combined_analysis.pdf/png (all plots)" << std::endl;
    std::cout << "  - cellwise_analysis.csv (data)" << std::endl;
    std::cout << "==========================================================" << std::endl;
    
    // Cleanup
    delete hEnergyTotal;
    delete grEnergy;
    delete grHits;
    delete c1;
    delete c2;
    delete c3;
    delete c4;
    
    fInput->Close();
}
