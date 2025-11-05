#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <map>
#include <set>

void analyze_hits_per_layer(const char* inputFile = "hgcal_output_processed.root") {
    
    gStyle->SetOptStat(0);
    
    // Open file and get tree
    TFile *fInput = TFile::Open(inputFile, "READ");
    TTree *cellTree = (TTree*)fInput->Get("CellWiseSegmentation");
    
    // Branch addresses
    Int_t event_id, layer, i_cell, j_cell;
    cellTree->SetBranchAddress("event_id", &event_id);
    cellTree->SetBranchAddress("layer", &layer);
    cellTree->SetBranchAddress("i", &i_cell);
    cellTree->SetBranchAddress("j", &j_cell);
    
    const Int_t nLayers = 47;
    const Int_t nEvents = 5;
    
    // Map to store unique cells: map[event][layer] = set of (i,j) pairs
    std::map<Int_t, std::map<Int_t, std::set<std::pair<Int_t, Int_t>>>> uniqueCells;
    
    // Read tree and collect unique cells
    Long64_t nEntries = cellTree->GetEntries();
    for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++) {
        cellTree->GetEntry(iEntry);
        
        if (event_id >= 0 && event_id < nEvents && layer >= 1 && layer <= nLayers) {
            uniqueCells[event_id][layer].insert(std::make_pair(i_cell, j_cell));
        }
    }
    
    // Create combined plot
    TCanvas *c = new TCanvas("c", "Unique Cells per Layer", 1600, 1000);
    c->Divide(3, 2);
    
    for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
        c->cd(iEvent + 1);
        gPad->SetLeftMargin(0.12);
        gPad->SetRightMargin(0.05);
        gPad->SetGrid();
        
        TH1D *h = new TH1D(Form("h_event%d", iEvent),
                          Form("Event %d;Layer Number;Unique Cells", iEvent),
                          nLayers, 0.5, nLayers + 0.5);
        
        Int_t totalCells = 0;
        for (Int_t iLayer = 1; iLayer <= nLayers; iLayer++) {
            Int_t cells = uniqueCells[iEvent][iLayer].size();
            h->SetBinContent(iLayer, cells);
            totalCells += cells;
        }
        
        h->SetLineColor(kRed+1);
        h->SetLineWidth(2);
        h->SetFillColor(kRed-9);
        h->GetXaxis()->SetTitleSize(0.05);
        h->GetXaxis()->SetLabelSize(0.045);
        h->GetYaxis()->SetTitleSize(0.05);
        h->GetYaxis()->SetLabelSize(0.045);
        h->SetMinimum(0);
        h->Draw("HIST");
        
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.50, 0.85, Form("Total: %d cells", totalCells));
    }
    
    c->Update();
    c->Print("unique_cells_per_layer.png");
    
    delete c;
    fInput->Close();
    
    std::cout << "Plot saved: unique_cells_per_layer.png" << std::endl;
}
