#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <iostream>
#include <fstream>
#include <map>

void fit_landau_distributions(const char* inputFile = "hgcal_output_processed.root") {
    
    // Set ROOT style - disable automatic stat boxes
    gStyle->SetOptFit(0);
    gStyle->SetOptStat(0);
    
    // Open input file
    TFile *fInput = TFile::Open(inputFile, "READ");

    // Get the CellWiseSegmentation tree
    TTree *cellTree = (TTree*)fInput->Get("CellWiseSegmentation");
    
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
    
    // Create maps to store histograms for each layer
    std::map<Int_t, TH1D*> h_eta_map;
    std::map<Int_t, TH1D*> h_phi_map;
    std::map<Int_t, TH1D*> h_theta_map;
    std::map<Int_t, TH1D*> h_edep_map;
    
    // Initialize histograms for each layer with restricted ranges
    std::cout << "Creating histograms..." << std::endl;
    for (Int_t iLayer = 1; iLayer <= nLayers; iLayer++) {
        // Eta: from 0 onwards (0 to 5)
        h_eta_map[iLayer] = new TH1D(Form("h_eta_layer%d", iLayer),
                                      Form("Eta Distribution - Layer %d;#eta;Entries", iLayer),
                                      100, 0, 5);
        
        h_phi_map[iLayer] = new TH1D(Form("h_phi_layer%d", iLayer),
                                      Form("Phi Distribution - Layer %d;#phi [deg];Entries", iLayer),
                                      100, -180, 180);
        
        h_theta_map[iLayer] = new TH1D(Form("h_theta_layer%d", iLayer),
                                        Form("Theta Distribution - Layer %d;#theta [deg];Entries", iLayer),
                                        100, 0, 180);
        
        // Edep: 0 to 1 MeV
        h_edep_map[iLayer] = new TH1D(Form("h_edep_layer%d", iLayer),
                                       Form("Energy Deposition - Layer %d;E_{dep} [MeV];Entries", iLayer),
                                       100, 0, 1);
    }
    
    // Fill histograms
    std::cout << "Filling histograms..." << std::endl;
    Long64_t nEntries = cellTree->GetEntries();
    for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++) {
        cellTree->GetEntry(iEntry);
        
        if (iEntry % 10000 == 0) {
            std::cout << "Processing entry " << iEntry << " / " << nEntries << std::endl;
        }
        
        if (layer >= 1 && layer <= nLayers) {
            h_eta_map[layer]->Fill(eta);
            h_phi_map[layer]->Fill(phi);
            h_theta_map[layer]->Fill(theta);
            h_edep_map[layer]->Fill(edep);
        }
    }
    
    // Create canvas
    TCanvas *c1 = new TCanvas("c1", "Distributions", 1400, 900);
    c1->SetGrid();
    
    // Open output text file for parameters
    std::ofstream outFile("distribution_parameters.txt");
    outFile << "==========================================================" << std::endl;
    outFile << "Distribution Parameters" << std::endl;
    outFile << "==========================================================" << std::endl;
    
    // ==================== Process Eta ====================
    std::cout << "\nPlotting Eta distributions..." << std::endl;
    outFile << "\n=== Eta Distributions ===" << std::endl;
    outFile << "Layer\tEntries\tMean\t\tStd" << std::endl;
    outFile << "----------------------------------------------------------" << std::endl;
    
    c1->Print("eta_distributions.pdf[");
    for (Int_t iLayer = 1; iLayer <= nLayers; iLayer++) {
        TH1D* h = h_eta_map[iLayer];
        if (h->GetEntries() > 0) {
            c1->Clear();
            c1->cd();
            c1->SetLogy(1);
            h->SetLineColor(kBlue+1);
            h->SetLineWidth(2);
            h->SetFillColor(kBlue-10);
            h->Draw("HIST");
            
            // Get histogram statistics
            Int_t entries = h->GetEntries();
            Double_t mean = h->GetMean();
            Double_t std = h->GetRMS();
            
            // Add text with statistics
            TLatex latex;
            latex.SetNDC();
            latex.SetTextSize(0.04);
            latex.SetTextColor(kBlack);
            latex.SetTextFont(42);
	    latex.DrawLatex(0.70, 0.85, Form("Layer %d", iLayer));
            latex.DrawLatex(0.70, 0.80, Form("Entries = %d", entries));
            latex.DrawLatex(0.70, 0.75, Form("Mean = %.4f", mean));
            latex.DrawLatex(0.70, 0.70, Form("Std = %.4f", std));	
            
            outFile << iLayer << "\t" << entries << "\t" << mean << "\t" << std << std::endl;
            
            c1->Update();
            c1->Print("eta_distributions.pdf");
        }
    }
    c1->Print("eta_distributions.pdf]");
    
    // ==================== Process Phi ====================
    std::cout << "\nPlotting Phi distributions..." << std::endl;
    outFile << "\n=== Phi Distributions ===" << std::endl;
    outFile << "Layer\tEntries\tMean\t\tStd" << std::endl;
    outFile << "----------------------------------------------------------" << std::endl;
    
    c1->Print("phi_distributions.pdf[");
    for (Int_t iLayer = 1; iLayer <= nLayers; iLayer++) {
        TH1D* h = h_phi_map[iLayer];
        if (h->GetEntries() > 0) {
            c1->Clear();
            c1->cd();
            c1->SetLogy(1);
            h->SetLineColor(kBlue+1);
            h->SetLineWidth(2);
            h->SetFillColor(kBlue-10);
            h->Draw("HIST");
            
            Int_t entries = h->GetEntries();
            Double_t mean = h->GetMean();
            Double_t std = h->GetRMS();
            
            TLatex latex;
            latex.SetNDC();
            latex.SetTextSize(0.04);
            latex.SetTextColor(kBlack);
            latex.SetTextFont(42);
	    latex.DrawLatex(0.70, 0.85, Form("Layer %d", iLayer));
            latex.DrawLatex(0.70, 0.80, Form("Entries = %d", entries));
            latex.DrawLatex(0.70, 0.75, Form("Mean = %.4f", mean));
            latex.DrawLatex(0.70, 0.70, Form("Std = %.4f", std));
            
            outFile << iLayer << "\t" << entries << "\t" << mean << "\t" << std << std::endl;
            
            c1->Update();
            c1->Print("phi_distributions.pdf");
        }
    }
    c1->Print("phi_distributions.pdf]");
    
    // ==================== Process Theta ====================
    std::cout << "\nPlotting Theta distributions..." << std::endl;
    outFile << "\n=== Theta Distributions ===" << std::endl;
    outFile << "Layer\tEntries\tMean\t\tStd" << std::endl;
    outFile << "----------------------------------------------------------" << std::endl;
    
    c1->Print("theta_distributions.pdf[");
    for (Int_t iLayer = 1; iLayer <= nLayers; iLayer++) {
        TH1D* h = h_theta_map[iLayer];
        if (h->GetEntries() > 0) {
            c1->Clear();
            c1->cd();
            c1->SetLogy(1);
            h->SetLineColor(kBlue+1);
            h->SetLineWidth(2);
            h->SetFillColor(kBlue-10);
            h->Draw("HIST");
            
            Int_t entries = h->GetEntries();
            Double_t mean = h->GetMean();
            Double_t std = h->GetRMS();
            
            TLatex latex;
            latex.SetNDC();
            latex.SetTextSize(0.04);
            latex.SetTextColor(kBlack);
            latex.SetTextFont(42);
	    latex.DrawLatex(0.70, 0.85, Form("Layer %d", iLayer));
            latex.DrawLatex(0.70, 0.80, Form("Entries = %d", entries));
            latex.DrawLatex(0.70, 0.75, Form("Mean = %.4f", mean));
            latex.DrawLatex(0.70, 0.70, Form("Std = %.4f", std));
            
            outFile << iLayer << "\t" << entries << "\t" << mean << "\t" << std << std::endl;
            
            c1->Update();
            c1->Print("theta_distributions.pdf");
        }
    }
    c1->Print("theta_distributions.pdf]");
    
    // ==================== Process Edep with Landau Fit ====================
    std::cout << "\nFitting Energy Deposition distributions..." << std::endl;
    outFile << "\n=== Energy Deposition Distributions (Landau Fit) ===" << std::endl;
    outFile << "Layer\tEntries\tMean\t\tStd\t\tMPV\t\tWidth\t\tChi2/NDF" << std::endl;
    outFile << "----------------------------------------------------------" << std::endl;
    
    c1->Print("edep_fits.pdf[");
    for (Int_t iLayer = 1; iLayer <= nLayers; iLayer++) {
        TH1D* h = h_edep_map[iLayer];
        if (h->GetEntries() > 10) {
            c1->Clear();
            c1->cd();
            c1->SetLogy(1);
            h->SetLineColor(kBlue+1);
            h->SetLineWidth(2);
            h->SetFillColor(kBlue-10);
            h->Draw("HIST");
            
            Int_t entries = h->GetEntries();
            Double_t mean = h->GetMean();
            Double_t std = h->GetRMS();
            Double_t fitMin = TMath::Max(0.01, mean - std);
            Double_t fitMax = TMath::Min(2.0, mean + 3*std);
            
            TF1 *fitFunc = new TF1(Form("landau_edep_%d", iLayer), "landau", fitMin, fitMax);
            fitFunc->SetParameters(h->GetMaximum() * h->GetBinWidth(1), mean, std*0.3);
            fitFunc->SetLineColor(kRed);
            fitFunc->SetLineWidth(3);
            h->Fit(fitFunc, "RQ");
            fitFunc->Draw("SAME");  // Explicitly overlay the fit
            
            Double_t mpv = fitFunc->GetParameter(1);
            Double_t width = fitFunc->GetParameter(2);
            Double_t chi2ndf = (fitFunc->GetNDF() > 0) ? fitFunc->GetChisquare() / fitFunc->GetNDF() : 0;
            
            TLatex latex;
            latex.SetNDC();
            latex.SetTextSize(0.04);
            latex.SetTextColor(kBlack);
            latex.SetTextFont(42);
	    latex.DrawLatex(0.70, 0.85, Form("Layer %d", iLayer));
            latex.DrawLatex(0.70, 0.80, Form("Entries = %d", entries));
            latex.DrawLatex(0.70, 0.75, Form("Mean = %.4f", mean));
            latex.DrawLatex(0.70, 0.70, Form("Std = %.4f", std));
            latex.DrawLatex(0.70, 0.65, Form("MPV = %.4f MeV", mpv));
            
            outFile << iLayer << "\t" << entries << "\t" << mean << "\t" << std 
                   << "\t" << mpv << "\t" << width << "\t" << chi2ndf << std::endl;
            
            c1->Update();
            c1->Print("edep_fits.pdf");
            
            delete fitFunc;
        }
    }
    c1->SetLogy(0);
    c1->Print("edep_fits.pdf]");
    
    outFile << "==========================================================" << std::endl;
    outFile.close();
    
    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Processing complete!" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "Output files created:" << std::endl;
    std::cout << "  - eta_distributions.pdf (47 pages, no fit)" << std::endl;
    std::cout << "  - phi_distributions.pdf (47 pages, no fit)" << std::endl;
    std::cout << "  - theta_distributions.pdf (47 pages, no fit)" << std::endl;
    std::cout << "  - edep_fits.pdf (47 pages, with Landau fit)" << std::endl;
    std::cout << "  - distribution_parameters.txt (all parameters)" << std::endl;
    std::cout << "==========================================================" << std::endl;
    
    // Cleanup
    for (auto& pair : h_eta_map) delete pair.second;
    for (auto& pair : h_phi_map) delete pair.second;
    for (auto& pair : h_theta_map) delete pair.second;
    for (auto& pair : h_edep_map) delete pair.second;
    
    delete c1;
    fInput->Close();
}
