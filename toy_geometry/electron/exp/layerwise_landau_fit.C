#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <map>
#include <vector>
#include <fstream>

void layerwise_landau_fit() {
    TFile *f = TFile::Open("hgcal_output0.root");
    TTree *t = (TTree*)f->Get("ParticleTracking");
    
    Int_t layer;
    Double_t edep;
    t->SetBranchAddress("layer", &layer);
    t->SetBranchAddress("energy_deposited_MeV", &edep);
    
    Long64_t nentries = t->GetEntries();
    
    std::map<Int_t, std::vector<Double_t>> edepPerLayer;
    
    // First loop: collect all energy depositions
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        if (layer >= 0 && edep > 0) {  
            edepPerLayer[layer].push_back(edep);
        }
    }
    
    std::map<Int_t, Double_t> mpvPerLayer;
    std::vector<Int_t> validLayers;
    
    // Fit Landau distribution for each layer to find MPV
    for (auto& pair : edepPerLayer) {
        Int_t iLayer = pair.first;  // Layer number
        validLayers.push_back(iLayer);
        
        TH1D *hLayer = new TH1D(Form("hLayer%d", iLayer), 
                                Form("Energy Distribution - Layer %d", iLayer),
                                100, 0, 2);
        
        // Fill histogram with energies from this layer
        for (Double_t e : pair.second) {
            hLayer->Fill(e);
        }
        
        TF1 *fit = new TF1(Form("fit%d", iLayer), "landau", 0, 2);
        Double_t maxBinCenter = hLayer->GetBinCenter(hLayer->GetMaximumBin());
        fit->SetParameters(hLayer->GetMaximum(), maxBinCenter, 0.1);
        hLayer->Fit(fit, "RQ");
        
        mpvPerLayer[iLayer] = fit->GetParameter(1);
        
        delete hLayer;
        delete fit;
    }
    
    std::map<Int_t, Double_t> filteredTotalEnergyPerLayer;
    std::map<Int_t, Int_t> filteredHitsPerLayer;
    
    // Second loop: apply energy cut based on 75% of MPV
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        
        if (layer >= 0 && edep > 0) {
            // Check if this layer has an MPV (it should if it had data)
            auto it = mpvPerLayer.find(layer);
            if (it != mpvPerLayer.end()) {
                Double_t cutThreshold = 0.75 * it->second;
                
                if (edep > cutThreshold) {
                    filteredTotalEnergyPerLayer[layer] += edep;
                    filteredHitsPerLayer[layer]++;
                }
            }
        }
    }
    
    // Create arrays for plotting
    Int_t nValidLayers = validLayers.size();
    Double_t *layerArray = new Double_t[nValidLayers];
    Double_t *filteredHitsArray = new Double_t[nValidLayers];
    Double_t *filteredEnergyArray = new Double_t[nValidLayers];
    Double_t *meanEnergyArray = new Double_t[nValidLayers];
    
    for (Int_t i = 0; i < nValidLayers; i++) {
        Int_t iLayer = validLayers[i];
        layerArray[i] = iLayer;
        filteredHitsArray[i] = filteredHitsPerLayer[iLayer];
        filteredEnergyArray[i] = filteredTotalEnergyPerLayer[iLayer];
        
        // Calculate mean energy per layer
        if (filteredHitsPerLayer[iLayer] > 0) {
            meanEnergyArray[i] = filteredEnergyArray[i] / filteredHitsArray[i];
        } else {
            meanEnergyArray[i] = 0;
        }
    }
    
    // Save to CSV
    std::ofstream outFile("energy_vs_layer.csv");
    outFile << "Layer,TotalEnergy_MeV,NumHits,MeanEnergy_MeV\n";
    for (Int_t i = 0; i < nValidLayers; i++) {
        outFile << layerArray[i] << "," 
                << filteredEnergyArray[i] << ","
                << filteredHitsArray[i] << ","
                << meanEnergyArray[i] << "\n";
    }
    outFile.close();
    
    // Create canvas with 2 plots
    TCanvas *c2 = new TCanvas("c2", "Summary", 1400, 600);
    c2->Divide(2, 1);
    
    // Plot 1: Number of Hits per Layer
    c2->cd(1);
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0.08);
    gPad->SetGrid();
    
    TGraph *grHits = new TGraph(nValidLayers, layerArray, filteredHitsArray);
    grHits->SetTitle("Number of Hits per Layer;Layer Number;Number of Hits");
    grHits->SetMarkerStyle(20);
    grHits->SetMarkerSize(1.2);
    grHits->SetMarkerColor(kBlue+1);
    grHits->SetLineColor(kBlue+1);
    grHits->SetLineWidth(2);
    grHits->GetXaxis()->SetTitleSize(0.045);
    grHits->GetYaxis()->SetTitleSize(0.045);
    grHits->GetXaxis()->SetLabelSize(0.04);
    grHits->GetYaxis()->SetLabelSize(0.04);
    grHits->GetYaxis()->SetTitleOffset(1.4);
    grHits->Draw("APL");
    
    // Plot 2: Mean Energy per Layer
    c2->cd(2);
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0.08);
    gPad->SetGrid();
    
    TGraph *grMeanEnergy = new TGraph(nValidLayers, layerArray, meanEnergyArray);
    grMeanEnergy->SetTitle("Mean Energy Deposited per Layer;Layer Number;Mean Energy per Hit (MeV)");
    grMeanEnergy->SetMarkerStyle(21);
    grMeanEnergy->SetMarkerSize(1.2);
    grMeanEnergy->SetMarkerColor(kRed+1);
    grMeanEnergy->SetLineColor(kRed+1);
    grMeanEnergy->SetLineWidth(2);
    grMeanEnergy->GetXaxis()->SetTitleSize(0.045);
    grMeanEnergy->GetYaxis()->SetTitleSize(0.045);
    grMeanEnergy->GetXaxis()->SetLabelSize(0.04);
    grMeanEnergy->GetYaxis()->SetLabelSize(0.04);
    grMeanEnergy->GetYaxis()->SetTitleOffset(1.4);
    grMeanEnergy->Draw("APL");
    
    c2->Update();
    c2->SaveAs("summary_plots.pdf");
    
    // Cleanup
    delete[] layerArray;
    delete[] filteredHitsArray;
    delete[] filteredEnergyArray;
    delete[] meanEnergyArray;
    delete c2;
    
    f->Close();
}
