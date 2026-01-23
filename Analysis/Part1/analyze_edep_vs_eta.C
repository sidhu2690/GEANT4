#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TGraph.h>
#include <TLegend.h>
#include <iostream>
#include <vector>

void analyze_edep_vs_eta(const char* inputFile = "hgcal_output_processed.root") {
    
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    
    TFile *fInput = TFile::Open(inputFile, "READ");
    TTree *cellTree = (TTree*)fInput->Get("CellWiseSegmentation");
    
    Double_t eta, edep;
    cellTree->SetBranchAddress("eta", &eta);
    cellTree->SetBranchAddress("edep", &edep);
    
    // Define eta ranges
    std::vector<std::pair<Double_t, Double_t>> etaRanges = {
        {1.5, 1.7}, {1.7, 1.9}, {1.9, 2.1}, {2.1, 2.3}, {2.3, 2.5},
        {2.5, 2.7}, {2.7, 2.9}, {2.9, 3.1}
    };
    
    Int_t nRanges = etaRanges.size();
    
    // Create histograms
    std::vector<TH1D*> histograms;
    for (Int_t i = 0; i < nRanges; i++) {
        TH1D *h = new TH1D(Form("h_%d", i),
                          Form("Energy Deposition (%.1f < #eta < %.1f);E_{dep} [MeV];Entries", 
                               etaRanges[i].first, etaRanges[i].second),
                          100, 0, 1);
        histograms.push_back(h);
    }
    
    // Fill histograms
    Long64_t nEntries = cellTree->GetEntries();
    for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++) {
        cellTree->GetEntry(iEntry);
        
        for (Int_t i = 0; i < nRanges; i++) {
            if (eta >= etaRanges[i].first && eta < etaRanges[i].second) {
                histograms[i]->Fill(edep);
                break;
            }
        }
    }
    
    TCanvas *c1 = new TCanvas("c1", "Energy Deposition", 1400, 900);
    
    std::vector<Double_t> etaCenters;
    std::vector<Double_t> mpvValues;
    
    c1->Print("edep_vs_eta.pdf[");
    
    // Fit each eta range
    for (Int_t i = 0; i < nRanges; i++) {
        c1->Clear();
        c1->cd();
        c1->SetLogy(1);
        
        TH1D *h = histograms[i];
        
        if (h->GetEntries() > 10) {
            h->SetLineColor(kBlue+1);
            h->SetLineWidth(2);
            //h->SetFillColor(kBlue-10);
            h->Draw("HIST");
            
            Int_t entries = h->GetEntries();
            Double_t mean = h->GetMean();
            Double_t std = h->GetRMS();
            
            // Landau fit
            Double_t fitMin = TMath::Max(0.01, mean - std);
            Double_t fitMax = TMath::Min(1.0, mean + 3*std);
            
            TF1 *fitFunc = new TF1(Form("landau_%d", i), "landau", fitMin, fitMax);
            fitFunc->SetParameters(h->GetMaximum() * h->GetBinWidth(1), mean, std*0.3);
            fitFunc->SetLineColor(kRed);
            fitFunc->SetLineWidth(3);
            h->Fit(fitFunc, "RQ");
            fitFunc->Draw("SAME");
            
            Double_t mpv = fitFunc->GetParameter(1);
            
            // Display text
            TLatex latex;
            latex.SetNDC();
            latex.SetTextSize(0.04);
            latex.SetTextColor(kBlack);
            latex.SetTextFont(42);
            latex.DrawLatex(0.70, 0.85, Form("#eta: %.1f - %.1f", etaRanges[i].first, etaRanges[i].second));
            latex.DrawLatex(0.70, 0.80, Form("Entries = %d", entries));
            latex.DrawLatex(0.70, 0.75, Form("Mean = %.4f", mean));
            latex.DrawLatex(0.70, 0.70, Form("Std = %.4f", std));
            latex.DrawLatex(0.70, 0.65, Form("MPV = %.4f MeV", mpv));
            
            // Store for summary plot
            Double_t etaCenter = (etaRanges[i].first + etaRanges[i].second) / 2.0;
            etaCenters.push_back(etaCenter);
            mpvValues.push_back(mpv);
            
            delete fitFunc;
        }
        
        c1->Update();
        c1->Print("edep_vs_eta.pdf");
    }
    
    // Overlay plot
    c1->Clear();
    c1->SetLogy(1);
    c1->cd();
    
    TLegend *leg = new TLegend(0.65, 0.45, 0.88, 0.88);
    leg->SetTextSize(0.03);
    
    Int_t colors[] = {kRed, kBlue, kGreen+2, kMagenta, kCyan+1, kOrange, kViolet, kSpring, kAzure+7, kPink+1};
    
    for (Int_t i = 0; i < nRanges; i++) {
        TH1D *h = (TH1D*)histograms[i]->Clone();
        h->SetLineColor(colors[i % 10]);
        h->SetLineWidth(2);
        h->SetStats(0);
        
        if (i == 0) {
            h->SetTitle("Energy Deposition Distribution;E_{dep} [MeV];Entries");
            h->Draw("HIST");
        } else {
            h->Draw("HIST SAME");
        }
        
        leg->AddEntry(h, Form("%.1f < #eta < %.1f", etaRanges[i].first, etaRanges[i].second), "l");
    }
    
    leg->Draw();
    c1->Update();
    c1->Print("edep_vs_eta.pdf");
    
    // Summary plot: MPV vs Eta center
    c1->Clear();
    c1->SetLogy(0);
    c1->cd();
    
    Int_t n = etaCenters.size();
    TGraph *gr = new TGraph(n);
    
    for (Int_t i = 0; i < n; i++) {
        gr->SetPoint(i, etaCenters[i], mpvValues[i]);
    }
    
    gr->SetTitle("MPV vs #eta;#eta (range center);MPV [MeV]");
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(1.5);
    gr->SetMarkerColor(kRed);
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->GetXaxis()->SetTitleSize(0.045);
    gr->GetXaxis()->SetLabelSize(0.04);
    gr->GetYaxis()->SetTitleSize(0.045);
    gr->GetYaxis()->SetLabelSize(0.04);
    
    c1->SetLeftMargin(0.13);  
    gr->Draw("ALP");
    c1->SetGrid();
    c1->Update();
    c1->Print("edep_vs_eta.pdf");
    
    c1->Print("edep_vs_eta.pdf]");
    
    // Cleanup
    for (auto h : histograms) delete h;
    delete leg;
    delete gr;
    delete c1;
    fInput->Close();
    
    std::cout << "Output: edep_vs_eta.pdf (" << (nRanges + 2) << " pages)" << std::endl;
}
