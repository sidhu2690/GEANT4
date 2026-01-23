#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <iostream>
#include <map>
#include <tuple>
#include <cmath>
#include <vector>

// Structure to hold aggregated cell data
struct CellData {
    Int_t event_id;
    Int_t layer;
    Int_t i;
    Int_t j;
    Double_t xi;
    Double_t yi;
    Double_t zi;
    Double_t theta;
    Double_t phi;
    Double_t eta;
    Double_t edep;
};

void cellwise_segmentation(const char* inputFile = "hgcal_output0.root", 
                            const char* outputFile = "hgcal_output_processed.root") {
    
    // Open input file
    TFile *fInput = TFile::Open(inputFile, "READ");


    
    // Get the trees
    TTree *genTree = (TTree*)fInput->Get("GeneratorInfo");
    TTree *trackTree = (TTree*)fInput->Get("ParticleTracking");
 
    
    // Create output file
    TFile *fOutput = new TFile(outputFile, "RECREATE");
    
    // Clone the first two trees
    TTree *genTreeClone = genTree->CloneTree(-1, "fast");
    genTreeClone->Write();
    
    TTree *trackTreeClone = trackTree->CloneTree(-1, "fast");
    trackTreeClone->Write();
    
    
    const Double_t cellSize = 7.0; // 7mm cell size
    
    // Map to aggregate hits by (event_id, layer, i, j)
    std::map<std::tuple<Int_t, Int_t, Int_t, Int_t>, CellData> cellMap;
    
    // Set up tree reader
    TTreeReader reader(trackTree);
    TTreeReaderValue<Int_t> eventID(reader, "eventID");
    TTreeReaderValue<Int_t> layer(reader, "layer");
    TTreeReaderValue<Double_t> energy_deposited_MeV(reader, "energy_deposited_MeV");
    TTreeReaderValue<Double_t> x_enter_mm(reader, "x_enter_mm");
    TTreeReaderValue<Double_t> y_enter_mm(reader, "y_enter_mm");
    TTreeReaderValue<Double_t> z_enter_mm(reader, "z_enter_mm");
    
    Long64_t nEntries = trackTree->GetEntries();
    Long64_t processedCount = 0;
    Long64_t filteredCount = 0;
    

    while (reader.Next()) {
        processedCount++;
        
        if (processedCount % 50000 == 0) {
            std::cout << "Processed " << processedCount << " / " << nEntries << " entries..." << std::endl;
        }
        
        // Filter: layer > 0 and energy deposited > 0
        if (*layer <= 0 || *energy_deposited_MeV <= 0) {
            continue;
        }
        
        filteredCount++;
        
        // Calculate cell indices
        Int_t i = (Int_t)std::round(*x_enter_mm / cellSize);
        Int_t j = (Int_t)std::round(*y_enter_mm / cellSize);
        
        // Calculate cell center coordinates
        Double_t xi = i * cellSize;
        Double_t yi = j * cellSize;
        Double_t zi = *z_enter_mm;
        
        // Create key for grouping
        auto key = std::make_tuple(*eventID, *layer, i, j);
        
        // Check if this cell already exists
        auto it = cellMap.find(key);
        if (it == cellMap.end()) {
            // New cell - create entry
            CellData cell;
            cell.event_id = *eventID;
            cell.layer = *layer;
            cell.i = i;
            cell.j = j;
            cell.xi = xi;
            cell.yi = yi;
            cell.zi = zi;
            cell.edep = *energy_deposited_MeV;
            
            // Calculate spherical coordinates
            Double_t ri = std::sqrt(xi*xi + yi*yi + zi*zi);
            Double_t phi_rad = std::atan2(yi, xi);
            Double_t phi_deg = phi_rad * 180.0 / M_PI;
            Double_t theta_rad = (ri > 0) ? std::acos(zi / ri) : 0.0;
            Double_t theta_deg = theta_rad * 180.0 / M_PI;
            Double_t eta = (theta_rad > 0 && theta_rad < M_PI) ? 
                          -std::log(std::tan(theta_rad / 2.0)) : 0.0;
            
            cell.theta = theta_deg;
            cell.phi = phi_deg;
            cell.eta = eta;
            
            cellMap[key] = cell;
        } else {
            it->second.edep += *energy_deposited_MeV;
        }
    }
    
    std::cout << "\nFiltered entries (layer>0, edep>0): " << filteredCount << std::endl;
    std::cout << "Unique cells: " << cellMap.size() << std::endl;
    
    // Create new tree for segmented data
    TTree *cellTree = new TTree("CellWiseSegmentation", "Cell-wise Segmented Hit Data");
    
    // Define branches for the new tree
    Int_t event_id, layer_out, i_out, j_out;
    Double_t xi_out, yi_out, zi_out, theta_out, phi_out, eta_out, edep_out;
    
    cellTree->Branch("event_id", &event_id, "event_id/I");
    cellTree->Branch("layer", &layer_out, "layer/I");
    cellTree->Branch("i", &i_out, "i/I");
    cellTree->Branch("j", &j_out, "j/I");
    cellTree->Branch("xi", &xi_out, "xi/D");
    cellTree->Branch("yi", &yi_out, "yi/D");
    cellTree->Branch("zi", &zi_out, "zi/D");
    cellTree->Branch("theta", &theta_out, "theta/D");
    cellTree->Branch("phi", &phi_out, "phi/D");
    cellTree->Branch("eta", &eta_out, "eta/D");
    cellTree->Branch("edep", &edep_out, "edep/D");
    
    // Fill the new tree
    std::cout << "\nFilling CellWiseSegmentation tree..." << std::endl;
    for (const auto& pair : cellMap) {
        const CellData& cell = pair.second;
        
        event_id = cell.event_id;
        layer_out = cell.layer;
        i_out = cell.i;
        j_out = cell.j;
        xi_out = cell.xi;
        yi_out = cell.yi;
        zi_out = cell.zi;
        theta_out = cell.theta;
        phi_out = cell.phi;
        eta_out = cell.eta;
        edep_out = cell.edep;
        
        cellTree->Fill();
    }
    
    // Write the new tree
    cellTree->Write();
    fOutput->Close();
    fInput->Close();
}
