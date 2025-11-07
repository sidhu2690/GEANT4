#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <map>
#include <vector>
#include <fstream>
#include <set>

void layerwise_landau_fit() {
    TFile *f = TFile::Open("electron_root.root");
    TTree *t = (TTree*)f->Get("ParticleTracking");
    
    Int_t layer;
    Int_t eventID;  // NEW: To track which event each hit belongs to
    Double_t edep;
    t->SetBranchAddress("layer", &layer);
    t->SetBranchAddress("eventID", &eventID);  // CHANGE THIS if your branch has a different name
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
        Int_t iLayer = pair.first;
        validLayers.push_back(iLayer);
        
        TH1D *hLayer = new TH1D(Form("hLayer%d", iLayer), 
                                Form("Energy Distribution - Layer %d", iLayer),
                                100, 0, 2);
        
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
    std::map<Int_t, std::set<Int_t>> uniqueEventsPerLayer;  // NEW: Track unique events
    
    // Second loop: apply energy cut based on 75% of MPV
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        
        if (layer >= 0 && edep > 0) {
            auto it = mpvPerLayer.find(layer);
            if (it != mpvPerLayer.end()) {
                Double_t cutThreshold = 0.75 * it->second;
                
                if (edep > cutThreshold) {
                    filteredTotalEnergyPerLayer[layer] += edep;
                    filteredHitsPerLayer[layer]++;
                    uniqueEventsPerLayer[layer].insert(eventID);  // NEW: Track unique event
                }
            }
        }
    }
    
    // Create arrays for plotting
    Int_t nValidLayers = validLayers.size();
    Double_t *layerArray = new Double_t[nValidLayers];
    Double_t *filteredHitsArray = new Double_t[nValidLayers];
    Double_t *filteredEnergyArray = new Double_t[nValidLayers];
    Double_t *meanEnergyPerHitArray = new Double_t[nValidLayers];
    Double_t *meanEnergyPerEventArray = new Double_t[nValidLayers];  // NEW
    Double_t *uniqueEventsArray = new Double_t[nValidLayers];        // NEW
    
    for (Int_t i = 0; i < nValidLayers; i++) {
        Int_t iLayer = validLayers[i];
        layerArray[i] = iLayer;
        filteredHitsArray[i] = filteredHitsPerLayer[iLayer];
        filteredEnergyArray[i] = filteredTotalEnergyPerLayer[iLayer];
        uniqueEventsArray[i] = uniqueEventsPerLayer[iLayer].size();
        
        // Calculate mean energy per hit
        if (filteredHitsPerLayer[iLayer] > 0) {
            meanEnergyPerHitArray[i] = filteredEnergyArray[i] / filteredHitsArray[i];
        } else {
            meanEnergyPerHitArray[i] = 0;
        }
        
        // NEW: Calculate mean energy per EVENT
        if (uniqueEventsPerLayer[iLayer].size() > 0) {
            meanEnergyPerEventArray[i] = filteredEnergyArray[i] / uniqueEventsPerLayer[iLayer].size();
        } else {
            meanEnergyPerEventArray[i] = 0;
        }
    }
    
    // Save to CSV
    std::ofstream outFile("energy_vs_layer.csv");
    outFile << "Layer,TotalEnergy_MeV,NumHits,UniqueEvents,MeanEnergy_PerHit,MeanEnergy_PerEvent\n";
    for (Int_t i = 0; i < nValidLayers; i++) {
        outFile << layerArray[i] << "," 
                << filteredEnergyArray[i] << ","
                << filteredHitsArray[i] << ","
                << uniqueEventsArray[i] << ","
                << meanEnergyPerHitArray[i] << ","
                << meanEnergyPerEventArray[i] << "\n";
    }
    outFile.close();
    
    // Create canvas with 3 plots
    TCanvas *c2 = new TCanvas("c2", "Summary", 2100, 600);
    c2->Divide(3, 1);
    
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
    
    // Plot 2: Mean Energy per Hit
    c2->cd(2);
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0.08);
    gPad->SetGrid();
    
    TGraph *grMeanPerHit = new TGraph(nValidLayers, layerArray, meanEnergyPerHitArray);
    grMeanPerHit->SetTitle("Mean Energy per Hit;Layer Number;Mean Energy per Hit (MeV)");
    grMeanPerHit->SetMarkerStyle(21);
    grMeanPerHit->SetMarkerSize(1.2);
    grMeanPerHit->SetMarkerColor(kRed+1);
    grMeanPerHit->SetLineColor(kRed+1);
    grMeanPerHit->SetLineWidth(2);
    grMeanPerHit->GetXaxis()->SetTitleSize(0.045);
    grMeanPerHit->GetYaxis()->SetTitleSize(0.045);
    grMeanPerHit->GetXaxis()->SetLabelSize(0.04);
    grMeanPerHit->GetYaxis()->SetLabelSize(0.04);
    grMeanPerHit->GetYaxis()->SetTitleOffset(1.4);
    grMeanPerHit->Draw("APL");
    
    // Plot 3: Mean Energy per EVENT (NEW!)
    c2->cd(3);
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0.08);
    gPad->SetGrid();
    
    TGraph *grMeanPerEvent = new TGraph(nValidLayers, layerArray, meanEnergyPerEventArray);
    grMeanPerEvent->SetTitle("Mean Energy per Event;Layer Number;Mean Energy per Event (MeV)");
    grMeanPerEvent->SetMarkerStyle(22);
    grMeanPerEvent->SetMarkerSize(1.2);
    grMeanPerEvent->SetMarkerColor(kGreen+2);
    grMeanPerEvent->SetLineColor(kGreen+2);
    grMeanPerEvent->SetLineWidth(2);
    grMeanPerEvent->GetXaxis()->SetTitleSize(0.045);
    grMeanPerEvent->GetYaxis()->SetTitleSize(0.045);
    grMeanPerEvent->GetXaxis()->SetLabelSize(0.04);
    grMeanPerEvent->GetYaxis()->SetLabelSize(0.04);
    grMeanPerEvent->GetYaxis()->SetTitleOffset(1.4);
    grMeanPerEvent->Draw("APL");
    
    c2->Update();
    c2->SaveAs("summary_plots.pdf");
    
    // Cleanup
    delete[] layerArray;
    delete[] filteredHitsArray;
    delete[] filteredEnergyArray;
    delete[] meanEnergyPerHitArray;
    delete[] meanEnergyPerEventArray;
    delete[] uniqueEventsArray;
    delete c2;
    
    f->Close();
}
