#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <iostream>
#include <map>
#include <tuple>
#include <cmath>
#include <vector>

using namespace std;

// Structure to hold aggregated Pixel cell data
struct Pixel_CellData {
    Int_t event_id;
    Int_t layer;
    Int_t i;
    Int_t j;
    Int_t ADC;
    Double_t xi;
    Double_t yi;
    Double_t zi;
    Double_t theta;
    Double_t phi;
    Double_t eta;
    Double_t edep;
};

// Structure to hold aggregated Eta_Phi cell data
struct Eta_Phi_CellData {
    Int_t event_id;
    Int_t layer;
    Int_t ieta;
    Int_t iphi;
    Int_t ADC;
    Double_t xi;
    Double_t yi;
    Double_t zi;
    Double_t theta;
    Double_t phi;
    Double_t eta;
    Double_t edep;
};

void cellwise_segmentation_v03(const char* inputFile = "Electron_Pt_025_Eta_185_Events_5_PU_000_Set01_Step1.root",
                            const char* outputFile = "Electron_Pt_025_Eta_185_Events_5_PU_000_Set01_Step2.root") {

    Int_t nEta_Bins  = 736,   nPhi_Bins = nEta_Bins;
    float Eta_Min    = 1.4,   Eta_Max   = 3.1,   Phi_Min = 0.,  Phi_Max = 360.;
    Double_t del_Eta = (Eta_Max - Eta_Min)/nEta_Bins;
    Double_t del_Phi = (Phi_Max - Phi_Min)/nPhi_Bins;

    // Open input file
    TFile *fInput = TFile::Open(inputFile, "READ");
    if (!fInput || fInput->IsZombie()) {
        std::cerr << "Error: Cannot open input file " << inputFile << std::endl;
        return;
    }

    TTree *genTree = (TTree*)fInput->Get("GeneratorInfo");
    TTree *trackTree = (TTree*)fInput->Get("ParticleTracking");

    if (!genTree || !trackTree) {
        std::cerr << "Error: Cannot find required trees" << std::endl;
        fInput->Close();
        return;
    }

    // CREATE OUTPUT FILE FIRST
    TFile *fOutput = new TFile(outputFile, "RECREATE");
    if (!fOutput || fOutput->IsZombie()) {
        std::cerr << "Error: Cannot create output file!" << std::endl;
        fInput->Close();
        delete fInput;
        return;
    }
    fOutput->cd();

    // Clone GeneratorInfo immediately and write
    std::cout << "Cloning GeneratorInfo tree..." << std::endl;
    TTree *genTreeClone = genTree->CloneTree(-1, "fast");
    genTreeClone->Write("", TObject::kOverwrite);
    Long64_t genEntries = genTreeClone->GetEntries();
    std::cout << "  Written GeneratorInfo: " << genEntries << " entries" << std::endl;
    delete genTreeClone;

    // Create output tree BEFORE processing
    TTree *Pixel_cellTree   = new TTree("Pixel_CellWiseSegmentation",   "Pixel Cell-wise Segmented Hit Data");
    TTree *Eta_Phi_cellTree = new TTree("Eta_Phi_CellWiseSegmentation", "Eta-Phi Cell-wise Segmented Hit Data");
    
    // CRITICAL: Set basket size and auto-flush
    Pixel_cellTree->SetAutoSave(0);         // Disable autosave
    Pixel_cellTree->SetAutoFlush(50000);    // Flush every 50k entries

    Eta_Phi_cellTree->SetAutoSave(0);       // Disable autosave
    Eta_Phi_cellTree->SetAutoFlush(50000);  // Flush every 50k entries
    
    Int_t    event_id, layer_out, i_out,  j_out,     ieta_out, iphi_out, adc_out;
    Double_t xi_out,   yi_out,    zi_out, theta_out, phi_out,  eta_out,  edep_out;

    //Pixel Cell Tree Structure
    Pixel_cellTree->Branch("event_id", &event_id,  "event_id/I");
    Pixel_cellTree->Branch("layer",    &layer_out, "layer/I");
    Pixel_cellTree->Branch("i",        &i_out,     "i/I");
    Pixel_cellTree->Branch("j",        &j_out,     "j/I");
    Pixel_cellTree->Branch("xi",       &xi_out,    "xi/D");
    Pixel_cellTree->Branch("yi",       &yi_out,    "yi/D");
    Pixel_cellTree->Branch("zi",       &zi_out,    "zi/D");
    Pixel_cellTree->Branch("theta",    &theta_out, "theta/D");
    Pixel_cellTree->Branch("phi",      &phi_out,   "phi/D");
    Pixel_cellTree->Branch("eta",      &eta_out,   "eta/D");
    Pixel_cellTree->Branch("edep",     &edep_out,  "edep/D");
    Pixel_cellTree->Branch("ADC",      &adc_out,   "ADC/I");


    //Eta_Phi Cell Tree Structure
    Eta_Phi_cellTree->Branch("event_id", &event_id,  "event_id/I");
    Eta_Phi_cellTree->Branch("layer",    &layer_out, "layer/I");
    Eta_Phi_cellTree->Branch("ieta",     &ieta_out,  "ieta/I");
    Eta_Phi_cellTree->Branch("iphi",     &iphi_out,  "iphi/I");
    Eta_Phi_cellTree->Branch("xi",       &xi_out,    "xi/D");
    Eta_Phi_cellTree->Branch("yi",       &yi_out,    "yi/D");
    Eta_Phi_cellTree->Branch("zi",       &zi_out,    "zi/D");
    Eta_Phi_cellTree->Branch("theta",    &theta_out, "theta/D");
    Eta_Phi_cellTree->Branch("phi",      &phi_out,   "phi/D");
    Eta_Phi_cellTree->Branch("eta",      &eta_out,   "eta/D");
    Eta_Phi_cellTree->Branch("edep",     &edep_out,  "edep/D");
    Eta_Phi_cellTree->Branch("ADC",      &adc_out,   "ADC/I");



    const Double_t cellSize = 7.0;

    // PROCESS IN CHUNKS - don't store everything in memory
    std::map<std::tuple<Int_t, Int_t, Int_t, Int_t>, Pixel_CellData>   Pixel_cellMap;
    std::map<std::tuple<Int_t, Int_t, Int_t, Int_t>, Eta_Phi_CellData> Eta_Phi_cellMap;
    
    TTreeReader reader(trackTree);
    TTreeReaderValue<Int_t> eventID(reader, "eventID");
    TTreeReaderValue<Int_t> layer(reader, "layer");
    TTreeReaderValue<Double_t> energy_deposited_MeV(reader, "energy_deposited_MeV");
    TTreeReaderValue<Double_t> x_enter_mm(reader, "x_enter_mm");
    TTreeReaderValue<Double_t> y_enter_mm(reader, "y_enter_mm");
    TTreeReaderValue<Double_t> z_enter_mm(reader, "z_enter_mm");

    Long64_t nEntries = trackTree->GetEntries();
    Long64_t processedCount      = 0,   filteredCount         = 0;
    Long64_t written_Pixel_Count = 0,   written_Eta_Phi_Count = 0;
    Int_t    currentEvent        = -1;

    std::cout << "Processing ParticleTracking tree..." << std::endl;

    while (reader.Next()) {
        processedCount++;

        if (processedCount % 50000 == 0) {
            std::cout << "Processed " << processedCount << " / " << nEntries 
                      << " (written: " << written_Pixel_Count << " cells)" << std::endl;
        }

        if (*layer <= 0 || *energy_deposited_MeV <= 0) {
            continue;
        }

        filteredCount++;

        // CHECK IF WE'VE MOVED TO A NEW EVENT
        if (currentEvent != -1 && *eventID != currentEvent) {
            // Write accumulated cells for previous event
            for (const auto& pair1 : Pixel_cellMap) {
                const Pixel_CellData& pixel_cell = pair1.second;
                event_id  = pixel_cell.event_id;
                layer_out = pixel_cell.layer;
                i_out     = pixel_cell.i;
                j_out     = pixel_cell.j;
                xi_out    = pixel_cell.xi;
                yi_out    = pixel_cell.yi;
                zi_out    = pixel_cell.zi;
                theta_out = pixel_cell.theta;
                phi_out   = pixel_cell.phi;
                eta_out   = pixel_cell.eta;
                edep_out  = pixel_cell.edep;
                adc_out   = pixel_cell.ADC;
                
                Pixel_cellTree->Fill();
                written_Pixel_Count++;
            }
            
            for (const auto& pair2 : Eta_Phi_cellMap) {
                const Eta_Phi_CellData& eta_phi_cell = pair2.second;
                event_id  = eta_phi_cell.event_id;
                layer_out = eta_phi_cell.layer;
                ieta_out  = eta_phi_cell.ieta;
                iphi_out  = eta_phi_cell.iphi;
                xi_out    = eta_phi_cell.xi;
                yi_out    = eta_phi_cell.yi;
                zi_out    = eta_phi_cell.zi;
                theta_out = eta_phi_cell.theta;
                phi_out   = eta_phi_cell.phi;
                eta_out   = eta_phi_cell.eta;
                edep_out  = eta_phi_cell.edep;
                adc_out   = eta_phi_cell.ADC;
                
                Eta_Phi_cellTree->Fill();
                written_Eta_Phi_Count++;
            }
            
            // Clear map to free memory
            Pixel_cellMap.clear();       Eta_Phi_cellMap.clear();
        }
        
        currentEvent = *eventID;

        // Calculate pixel cell indices
        Int_t i = (Int_t)std::round(*x_enter_mm / cellSize);
        Int_t j = (Int_t)std::round(*y_enter_mm / cellSize);
        Double_t xi = i * cellSize,  yi = j * cellSize,  zi = *z_enter_mm;

        // Calculate eta-phi cell indices
        Double_t ri        = std::sqrt(xi*xi + yi*yi + zi*zi);
        Double_t phi_rad   = std::atan2(yi, xi);
        Double_t phi_deg   = 180. + (phi_rad * 180.0 / M_PI);
        Double_t theta_rad = (ri > 0) ? std::acos(zi / ri) : 0.0;
        Double_t theta_deg = theta_rad * 180.0 / M_PI;
        Double_t eta       = (theta_rad > 0 && theta_rad < M_PI) ?
                             -std::log(std::tan(theta_rad / 2.0)) : 0.0;

        Int_t ieta = (Int_t)std::round((fabs(eta) - Eta_Min) / del_Eta);
        Int_t iphi = (Int_t)std::round((phi_deg - Phi_Min) /del_Phi);

        auto key1 = std::make_tuple(*eventID, *layer, i, j);
        auto it1  = Pixel_cellMap.find(key1);
        if (it1 == Pixel_cellMap.end()) {
            Pixel_CellData pixel_cell;
            pixel_cell.event_id = *eventID;
            pixel_cell.layer    = *layer;
            pixel_cell.i        = i;
            pixel_cell.j        = j;
            pixel_cell.xi       = xi;
            pixel_cell.yi       = yi;
            pixel_cell.zi       = zi;
            pixel_cell.edep     = *energy_deposited_MeV;
            pixel_cell.ADC      = std::round((1000./2.2) * (*energy_deposited_MeV));
            pixel_cell.theta    = theta_deg;
            pixel_cell.phi      = phi_deg;
            pixel_cell.eta      = eta;

            Pixel_cellMap[key1] = pixel_cell;
        } else {
            it1->second.edep += *energy_deposited_MeV;
            it1->second.ADC  += std::round((1000./2.2) * (*energy_deposited_MeV));
        }


        auto key2 = std::make_tuple(*eventID, *layer, ieta, iphi);
        auto it2 = Eta_Phi_cellMap.find(key2);
        if (it2 == Eta_Phi_cellMap.end()) {
            Eta_Phi_CellData eta_phi_cell;
            eta_phi_cell.event_id = *eventID;
            eta_phi_cell.layer    = *layer;
            eta_phi_cell.ieta     = ieta;
            eta_phi_cell.iphi     = iphi;
            eta_phi_cell.xi       = xi;
            eta_phi_cell.yi       = yi;
            eta_phi_cell.zi       = zi;
            eta_phi_cell.edep     = *energy_deposited_MeV;
            eta_phi_cell.ADC      = std::round((1000./2.2) * (*energy_deposited_MeV));
            eta_phi_cell.theta    = theta_deg;
            eta_phi_cell.phi      = phi_deg;
            eta_phi_cell.eta      = eta;

            Eta_Phi_cellMap[key2] = eta_phi_cell;
        } else {
            it2->second.edep += *energy_deposited_MeV;
            it2->second.ADC  += std::round((1000./2.2) * (*energy_deposited_MeV));
        }
    }

    // Write remaining cells from last event
    std::cout << "Writing final event cells..." << std::endl;
    for (const auto& pair1 : Pixel_cellMap) {
        const Pixel_CellData& pixel_cell = pair1.second;
        event_id  = pixel_cell.event_id;
        layer_out = pixel_cell.layer;
        i_out     = pixel_cell.i;
        j_out     = pixel_cell.j;
        xi_out    = pixel_cell.xi;
        yi_out    = pixel_cell.yi;
        zi_out    = pixel_cell.zi;
        theta_out = pixel_cell.theta;
        phi_out   = pixel_cell.phi;
        eta_out   = pixel_cell.eta;
        edep_out  = pixel_cell.edep;
        adc_out   = pixel_cell.ADC;
        
        Pixel_cellTree->Fill();
        written_Pixel_Count++;
    }

    for (const auto& pair2 : Eta_Phi_cellMap) {
        const Eta_Phi_CellData& eta_phi_cell = pair2.second;
        event_id  = eta_phi_cell.event_id;
        layer_out = eta_phi_cell.layer;
        ieta_out  = eta_phi_cell.ieta;
        iphi_out  = eta_phi_cell.iphi;
        xi_out    = eta_phi_cell.xi;
        yi_out    = eta_phi_cell.yi;
        zi_out    = eta_phi_cell.zi;
        theta_out = eta_phi_cell.theta;
        phi_out   = eta_phi_cell.phi;
        eta_out   = eta_phi_cell.eta;
        edep_out  = eta_phi_cell.edep;
        adc_out   = eta_phi_cell.ADC;
        
        Eta_Phi_cellTree->Fill();
        written_Eta_Phi_Count++;
    }


    std::cout << "\nFiltered entries (layer>0, edep>0): " << filteredCount << std::endl;
    std::cout << "Total cells written: Pixel = " << written_Pixel_Count;
    std::cout  << "   Eta_Phi = " << written_Eta_Phi_Count << std::endl;

    // FORCE FINAL WRITE
    std::cout << "Flushing tree to disk..." << std::endl;
    Pixel_cellTree->FlushBaskets();
    Pixel_cellTree->Write("", TObject::kOverwrite);
    Eta_Phi_cellTree->FlushBaskets();
    Eta_Phi_cellTree->Write("", TObject::kOverwrite);
    
    Long64_t pixel_cellEntries   = Pixel_cellTree->GetEntries();
    Long64_t eta_phi_cellEntries = Eta_Phi_cellTree->GetEntries();

    std::cout << "\n=== Output Summary ===" << std::endl;
    std::cout << "  1. GeneratorInfo: " << genEntries << " entries" << std::endl;
    std::cout << "  2. CellWiseSegmentation:   Pixel = " << pixel_cellEntries << " entries";
    std::cout << "   Eta_Phi = " << eta_phi_cellEntries << " entries" << std::endl;
    std::cout << "======================\n" << std::endl;

    // Cleanup
    delete Pixel_cellTree;    delete Eta_Phi_cellTree;
    fOutput->Close();         fInput->Close();
    delete fOutput;           delete fInput;
}
