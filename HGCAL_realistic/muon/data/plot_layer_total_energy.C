void plot_layer_total_energy() {
    // Open file and get tree
    TFile* f = TFile::Open("hgcal_output_processed.root");
    TTree* tree = (TTree*)f->Get("CellWiseSegmentation");
    
    cout << "=== Calculating Total Energy and Hit Multiplicity per Layer ===" << endl;
    
    // Arrays to store data
    std::vector<double> layers;
    std::vector<double> total_energy;
    std::vector<double> hit_multiplicity;
    
    int n_particles = 10000;
    
    // Calculate total energy and hit multiplicity for each layer
    for(int layer = 1; layer <= 47; layer++) {
        tree->Draw("edep", Form("layer==%d", layer), "goff");
        
        double sum = 0;
        int n_entries = tree->GetSelectedRows();
        double* values = tree->GetV1();
        
        for(int i = 0; i < n_entries; i++) {
            sum += values[i];
        }
        
        layers.push_back(layer);
        total_energy.push_back(sum / n_particles);
        hit_multiplicity.push_back((double)n_entries / n_particles);
        
        cout << "Layer " << layer << ": Avg Energy per Particle = " << sum / n_particles 
             << " MeV, Avg Hits per Particle = " << (double)n_entries / n_particles << endl;
    }
    
    // Set style
    gStyle->SetOptStat(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLineWidth(2);
    gStyle->SetTickLength(0.02, "XY");
    
    // Create canvas
    TCanvas* c1 = new TCanvas("c1", "Layer Analysis", 900, 700);
    c1->SetLeftMargin(0.15);
    c1->SetRightMargin(0.05);
    c1->SetTopMargin(0.08);
    c1->SetBottomMargin(0.12);
    
    // ==================================== 
    // PLOT 1: Total Energy per Layer per Particle
    // ====================================
    TGraph* g_energy = new TGraph(layers.size(), &layers[0], &total_energy[0]);
    g_energy->SetTitle("Energy Deposition Profile");
    g_energy->SetMarkerStyle(20);
    g_energy->SetMarkerSize(1.0);
    g_energy->SetMarkerColor(kBlack);
    g_energy->SetLineColor(kBlack);
    g_energy->SetLineWidth(2);
    g_energy->GetXaxis()->SetTitle("Layer Number");
    g_energy->GetYaxis()->SetTitle("Avg. Energy Deposited per Particle (MeV)");
    g_energy->GetXaxis()->SetTitleSize(0.05);
    g_energy->GetYaxis()->SetTitleSize(0.05);
    g_energy->GetXaxis()->SetLabelSize(0.045);
    g_energy->GetYaxis()->SetLabelSize(0.045);
    g_energy->GetXaxis()->SetTitleOffset(1.0);
    g_energy->GetYaxis()->SetTitleOffset(1.5);
    g_energy->GetXaxis()->SetLimits(0, 48);
    g_energy->GetXaxis()->SetNdivisions(510);
    g_energy->GetYaxis()->SetNdivisions(510);
    
    g_energy->Draw("APC");
    
    c1->Update();
    c1->SaveAs("layer_total_energy_sum.pdf(");
    
    // ==================================== 
    // PLOT 2: Hit Multiplicity per Layer per Particle
    // ====================================
    c1->Clear();
    
    TGraph* g_hits = new TGraph(layers.size(), &layers[0], &hit_multiplicity[0]);
    g_hits->SetTitle("DigiHit Multiplicity Profile");
    g_hits->SetMarkerStyle(21);
    g_hits->SetMarkerSize(1.0);
    g_hits->SetMarkerColor(kBlack);
    g_hits->SetLineColor(kBlack);
    g_hits->SetLineWidth(2);
    g_hits->GetXaxis()->SetTitle("Layer Number");
    g_hits->GetYaxis()->SetTitle("Average Number of DigiHits per Particle");
    g_hits->GetXaxis()->SetTitleSize(0.05);
    g_hits->GetYaxis()->SetTitleSize(0.05);
    g_hits->GetXaxis()->SetLabelSize(0.045);
    g_hits->GetYaxis()->SetLabelSize(0.045);
    g_hits->GetXaxis()->SetTitleOffset(1.0);
    g_hits->GetYaxis()->SetTitleOffset(1.5);
    g_hits->GetXaxis()->SetLimits(0, 48);
    g_hits->GetXaxis()->SetNdivisions(510);
    g_hits->GetYaxis()->SetNdivisions(510);
    
    g_hits->Draw("APC");
    
    c1->Update();
    c1->SaveAs("layer_total_energy_sum.pdf)");
    
    cout << "\nPDF saved as layer_total_energy_sum.pdf (2 pages)" << endl;
    cout << "Note: All values scaled per particle (total/10000)" << endl;
    
    f->Close();
}
