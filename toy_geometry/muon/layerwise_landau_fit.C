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
    Int_t maxLayer = 46;
    
    std::map<Int_t, std::vector<Double_t>> edepPerLayer;
    std::map<Int_t, Double_t> totalEnergyPerLayer;
    
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        if (layer >= 0 && edep > 0) {  
            edepPerLayer[layer].push_back(edep);
            totalEnergyPerLayer[layer] += edep;
        }
    }
    
    std::map<Int_t, Double_t> mpvPerLayer;
    std::vector<Int_t> validLayers;
    
    for (Int_t iLayer = 0; iLayer <= maxLayer; iLayer++) {
        if (edepPerLayer.find(iLayer) == edepPerLayer.end()) {
            continue;
        }
        
        validLayers.push_back(iLayer);
        
        TH1D *hLayer = new TH1D(Form("hLayer%d", iLayer), 
                                Form("Energy Distribution - Layer %d", iLayer),
                                100, 0, 2);
        
        for (Double_t e : edepPerLayer[iLayer]) {
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
    
    std::map<Int_t, std::vector<Double_t>> filteredEdepPerLayer;
    std::map<Int_t, Double_t> filteredTotalEnergyPerLayer;
    std::map<Int_t, Int_t> filteredHitsPerLayer;
    
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        
        if (layer >= 0 && edep > 0) {
            if (mpvPerLayer.find(layer) != mpvPerLayer.end()) {
                Double_t cutThreshold = 0.75 * mpvPerLayer[layer];
                
                if (edep > cutThreshold) {
                    filteredEdepPerLayer[layer].push_back(edep);
                    filteredTotalEnergyPerLayer[layer] += edep;
                }
            }
        }
    }
    
    for (auto& pair : filteredEdepPerLayer) {
        filteredHitsPerLayer[pair.first] = pair.second.size();
    }
    
    Int_t nValidLayers = validLayers.size();
    Double_t *layerArray = new Double_t[nValidLayers];
    Double_t *filteredHitsArray = new Double_t[nValidLayers];
    Double_t *filteredEnergyArray = new Double_t[nValidLayers];
    
    for (Int_t i = 0; i < nValidLayers; i++) {
        Int_t iLayer = validLayers[i];
        layerArray[i] = iLayer;
        filteredHitsArray[i] = filteredHitsPerLayer[iLayer];
        filteredEnergyArray[i] = filteredTotalEnergyPerLayer[iLayer];
    }
    
    std::ofstream outFile("energy_vs_layer.csv");
    outFile << "Layer,Energy_MeV\n";
    for (Int_t i = 0; i < nValidLayers; i++) {
        outFile << layerArray[i] << "," << filteredEnergyArray[i] << "\n";
    }
    outFile.close();
    
    TCanvas *c2 = new TCanvas("c2", "Summary", 1400, 600);
    c2->Divide(2, 1);
    
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
    
    c2->cd(2);
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0.08);
    gPad->SetGrid();
    
    TGraph *grEnergy = new TGraph(nValidLayers, layerArray, filteredEnergyArray);
    grEnergy->SetTitle("Total Energy Deposited per Layer;Layer Number;Total Energy Deposited (MeV)");
    grEnergy->SetMarkerStyle(21);
    grEnergy->SetMarkerSize(1.2);
    grEnergy->SetMarkerColor(kRed+1);
    grEnergy->SetLineColor(kRed+1);
    grEnergy->SetLineWidth(2);
    grEnergy->GetXaxis()->SetTitleSize(0.045);
    grEnergy->GetYaxis()->SetTitleSize(0.045);
    grEnergy->GetXaxis()->SetLabelSize(0.04);
    grEnergy->GetYaxis()->SetLabelSize(0.04);
    grEnergy->GetYaxis()->SetTitleOffset(1.4);
    grEnergy->Draw("APL");
    
    c2->Update();
    c2->SaveAs("summary_plots.pdf");
    
    delete[] layerArray;
    delete[] filteredHitsArray;
    delete[] filteredEnergyArray;
    delete c2;
    
    f->Close();
}
