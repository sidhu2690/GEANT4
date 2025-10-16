void landau() {
    TFile *f = TFile::Open("hgcal_output0.root");
    TTree *t = (TTree*)f->Get("ParticleTracking");
    Int_t eventID, layer;
    Double_t edep;
    t->SetBranchAddress("eventID", &eventID);
    t->SetBranchAddress("layer", &layer);
    t->SetBranchAddress("energy_deposited_MeV", &edep);
    
    const Int_t nLayers = 47;
    
    // Store raw hits per layer
    std::map<int, std::vector<std::pair<int, Double_t>>> rawHits; // layer -> [(eventID, edep)]
    
    // Collect all raw hits with edep > 0
    Long64_t nentries = t->GetEntries();
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        if (layer >= 0 && layer < nLayers && edep > 0 && edep < 1) {
            rawHits[layer].push_back({eventID, edep});
        }
    }
    
    // Create output with larger canvas
    TCanvas *c = new TCanvas("c", "MIP Energy via Landau Fit", 1200, 1400);
    c->Print("energy_mip_landau_fit.pdf[");
    
    std::ofstream csvFile("layer_mip_landau.csv");
    csvFile << "Layer,MIP_MPV_MeV,Threshold_25pct,Landau_Width,Raw_Hits,Filtered_Hits,Num_Events,Mean_Energy,SD_Energy\n";
    
    // Process each layer
    for (int iLayer = 0; iLayer < nLayers; iLayer++) {
        auto layerIt = rawHits.find(iLayer);
        if (layerIt == rawHits.end() || layerIt->second.empty()) {
            csvFile << iLayer << ",0,0,0,0,0,0,0,0\n";
            continue;
        }
        
        auto &hits = layerIt->second;
        Int_t rawHitCount = hits.size();
        
        // Step 1: Create histogram of RAW hits (not summed by event)
        double maxE = 0;
        for (auto &hit : hits) {
            maxE = std::max(maxE, hit.second);
        }
        if (maxE <= 0) maxE = 10.0;
        
        TH1F *hRaw = new TH1F(Form("h_raw_layer%d", iLayer),
                              Form("Raw Hit Energy - Layer %d;Energy per Hit (MeV);Counts", iLayer),
                              100, 0, maxE * 1.1);
        
        for (auto &hit : hits) {
            hRaw->Fill(hit.second);
        }
        
        // Step 2: Fit Landau to raw hit distribution
        Int_t maxBin = hRaw->GetMaximumBin();
        Double_t peakPos = hRaw->GetBinCenter(maxBin);
        Double_t peakHeight = hRaw->GetBinContent(maxBin);
        
        Double_t fitMin = peakPos * 0.2;
        Double_t fitMax = peakPos * 4.0;
        if (fitMax > maxE) fitMax = maxE;
        
        TF1 *landau = new TF1(Form("landau_%d", iLayer), "landau", fitMin, fitMax);
        landau->SetParameters(peakHeight * peakPos * 0.15, peakPos, peakPos * 0.15);
        landau->SetParLimits(1, peakPos * 0.5, peakPos * 2.0);
        landau->SetParLimits(2, peakPos * 0.05, peakPos * 0.5);
        
        TFitResultPtr fitResult = hRaw->Fit(landau, "QRS");
        
        Double_t mpv = landau->GetParameter(1);
        Double_t width = landau->GetParameter(2);
        
        // Step 3: Calculate 25% threshold
        Double_t threshold = 0.75 * mpv;
        
        // Step 4: Filter hits above threshold and sum by event
        std::map<int, Double_t> energyPerEvent;
        Int_t filteredHitCount = 0;
        
        for (auto &hit : hits) {
            if (hit.second >= threshold) {
                energyPerEvent[hit.first] += hit.second;
                filteredHitCount++;
            }
        }
        
        // Step 5: Create histogram of summed energy per event
        double maxEventE = 0;
        for (auto &ev : energyPerEvent) {
            maxEventE = std::max(maxEventE, ev.second);
        }
        if (maxEventE <= 0) maxEventE = 10.0;
        
        TH1F *hEvent = new TH1F(Form("h_event_layer%d", iLayer),
                                Form("Energy per Event - Layer %d (Threshold: %.2f MeV);Total Energy per Event (MeV);Counts", 
                                     iLayer, threshold),
                                100, 0, maxEventE * 1.1);
        
        for (auto &ev : energyPerEvent) {
            hEvent->Fill(ev.second);
        }
        
        // Calculate mean and standard deviation of summed energy per event
        Double_t meanEnergy = hEvent->GetMean();
        Double_t sdEnergy = hEvent->GetStdDev();
        
        // Save to CSV
        csvFile << iLayer << "," << mpv << "," << threshold << "," << width << "," 
                << rawHitCount << "," << filteredHitCount << "," 
                << energyPerEvent.size() << "," << meanEnergy << "," << sdEnergy << "\n";
        
        // Step 6: Plot the summed distribution
        c->Clear();
        c->Divide(1, 2);
        
        // Top: Raw hit distribution with fit
        c->cd(1);
        gPad->SetLeftMargin(0.12);
        gPad->SetRightMargin(0.25);
        hRaw->SetLineColor(kBlack);
        hRaw->SetLineWidth(2);
        hRaw->SetFillColorAlpha(kAzure - 9, 0.4);
        hRaw->Draw("HIST");
        
        landau->SetLineColor(kRed + 1);
        landau->SetLineWidth(3);
        landau->Draw("SAME");
        
        TLine *mpvLine = new TLine(mpv, 0, mpv, hRaw->GetMaximum() * 0.8);
        mpvLine->SetLineColor(kRed + 2);
        mpvLine->SetLineWidth(2);
        mpvLine->SetLineStyle(2);
        mpvLine->Draw();
        
        TLine *threshLine = new TLine(threshold, 0, threshold, hRaw->GetMaximum() * 0.6);
        threshLine->SetLineColor(kOrange + 1);
        threshLine->SetLineWidth(2);
        threshLine->SetLineStyle(2);
        threshLine->Draw();
        
        // Legend positioned to avoid stats box (stats box is typically top right)
        TLegend *leg1 = new TLegend(0.50, 0.70, 0.70, 0.88);
        leg1->SetTextSize(0.03);
        leg1->AddEntry(hRaw, "Raw Hits", "f");
        leg1->AddEntry(landau, "Landau Fit", "l");
        leg1->AddEntry(threshLine, "25% Threshold", "l");
        leg1->Draw();
        
        // Add fit parameters text box on top plot
        TPaveText *fitParams1 = new TPaveText(0.50, 0.55, 0.70, 0.68, "NDC");
        fitParams1->SetFillColor(kWhite);
        fitParams1->SetBorderSize(1);
        fitParams1->SetTextAlign(12);
        fitParams1->SetTextSize(0.03);
        fitParams1->AddText(Form("MPV = %.3f MeV", mpv));
        fitParams1->AddText(Form("Width = %.3f MeV", width));
        fitParams1->AddText(Form("Threshold = %.3f MeV", threshold));
        fitParams1->Draw();
        
        // Bottom: Summed energy per event
        c->cd(2);
        gPad->SetLeftMargin(0.12);
        gPad->SetRightMargin(0.25);
        hEvent->SetLineColor(kBlack);
        hEvent->SetLineWidth(2);
        hEvent->SetFillColorAlpha(kGreen - 9, 0.4);
        hEvent->Draw("HIST");
        
        // Add parameters text box on bottom plot
        TPaveText *fitParams2 = new TPaveText(0.50, 0.70, 0.70, 0.88, "NDC");
        fitParams2->SetFillColor(kWhite);
        fitParams2->SetBorderSize(1);
        fitParams2->SetTextAlign(12);
        fitParams2->SetTextSize(0.03);
        fitParams2->AddText(Form("Events: %d", (int)energyPerEvent.size()));
        fitParams2->AddText(Form("Filtered Hits: %d/%d", filteredHitCount, rawHitCount));
        fitParams2->AddText(Form("Mean E = %.3f MeV", meanEnergy));
        fitParams2->AddText(Form("SD E = %.3f MeV", sdEnergy));
        fitParams2->Draw();
        
        c->Print("energy_mip_landau_fit.pdf");
        
        delete landau;
        delete hRaw;
        delete hEvent;
    }
    
    c->Print("energy_mip_landau_fit.pdf]");
    csvFile.close();
    f->Close();
    
    std::cout << "✅ Done: energy_mip_landau_fit.pdf and layer_mip_landau.csv" << std::endl;
}
