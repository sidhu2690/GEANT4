#include "construction.hh"
#include "detector.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4NistManager.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include <vector>
#include <cmath>

MyDetectorConstruction::MyDetectorConstruction() {}
MyDetectorConstruction::~MyDetectorConstruction() {}

// Helper function to generate cell positions in annular region
std::vector<std::pair<G4double, G4double>> GenerateCellPositions(
    G4double cellSize, G4double innerRadius, G4double outerRadius)
{
    std::vector<std::pair<G4double, G4double>> positions;
    
    G4int numCellsAlongRadius = static_cast<G4int>(outerRadius / cellSize) + 1;
    
    for (G4int ix = -numCellsAlongRadius; ix <= numCellsAlongRadius; ix++) {
        for (G4int iy = -numCellsAlongRadius; iy <= numCellsAlongRadius; iy++) {
            G4double xPos = ix * cellSize;
            G4double yPos = iy * cellSize;
            G4double r = std::sqrt(xPos*xPos + yPos*yPos);
            
            // Check if position is within annular region
            if (r >= innerRadius && r <= outerRadius) {
                positions.push_back(std::make_pair(xPos, yPos));
            }
        }
    }
    
    return positions;
}

// Helper function to encode layer and cell indices into copy number
G4int EncodeCopyNumber(G4int layerID, G4int cellX, G4int cellY)
{
    // Encode: layerID * 1000000 + (cellX + 500) * 1000 + (cellY + 500)
    // Offset by 500 to handle negative indices
    return layerID * 1000000 + (cellX + 500) * 1000 + (cellY + 500);
}

G4VPhysicalVolume* MyDetectorConstruction::Construct() {
    G4NistManager* nist = G4NistManager::Instance();

    // World volume
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_Galactic");
    G4double worldSizeXY = 300.0*cm;
    G4double worldSizeZ = 550.0*cm;
    
    G4Box* solidWorld = new G4Box("solidWorld", worldSizeXY, worldSizeXY, worldSizeZ);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

    G4VPhysicalVolume* physWorld = new G4PVPlacement(
        0, G4ThreeVector(0,0,0), logicWorld, "physWorld", 0, false, 0, true
    );

    // ============================================
    // Materials
    // ============================================
    G4Material* siMat = nist->FindOrBuildMaterial("G4_Si");
    G4Material* cuMat = nist->FindOrBuildMaterial("G4_Cu");
    G4Material* pbMat = nist->FindOrBuildMaterial("G4_Pb");
    G4Material* airMat = nist->FindOrBuildMaterial("G4_AIR");
    
    // Stainless Steel (Fe-Ni alloy)
    G4Material* stainlessMat = new G4Material("StainlessSteel", 8.02*g/cm3, 2);
    stainlessMat->AddElement(nist->FindOrBuildElement("Fe"), 0.70);
    stainlessMat->AddElement(nist->FindOrBuildElement("Ni"), 0.30);
    
    // Kapton
    G4Material* kaptonMat = new G4Material("Kapton", 1.11*g/cm3, 4);
    kaptonMat->AddElement(nist->FindOrBuildElement("C"), 5);
    kaptonMat->AddElement(nist->FindOrBuildElement("H"), 4);
    kaptonMat->AddElement(nist->FindOrBuildElement("O"), 2);
    kaptonMat->AddElement(nist->FindOrBuildElement("N"), 1);
    
    // PCB
    G4Material* pcbMat = new G4Material("PCB", 1.7*g/cm3, 3);
    pcbMat->AddElement(nist->FindOrBuildElement("C"), 0.50);
    pcbMat->AddElement(nist->FindOrBuildElement("H"), 0.30);
    pcbMat->AddElement(nist->FindOrBuildElement("O"), 0.20);
    
    // Glue
    G4Material* glueMat = new G4Material("Glue", 1.3*g/cm3, 3);
    glueMat->AddElement(nist->FindOrBuildElement("C"), 0.60);
    glueMat->AddElement(nist->FindOrBuildElement("H"), 0.30);
    glueMat->AddElement(nist->FindOrBuildElement("O"), 0.10);

    // ============================================
    // Segmentation parameters
    // ============================================
    G4double cellSize = 50.0 * mm;  // 5 cm cells
    G4double innerRadius = 30.0 * cm;  // 300 mm

    // ============================================
    // Material structure (NON-SILICON layers)
    // ============================================
    struct MaterialLayer {
        G4String name;
        G4Material* material;
        G4double zMin;
        G4double zMax;
        G4double outerRadius;
        G4int layerID;  // For silicon layers
    };
    
    std::vector<MaterialLayer> nonSiliconLayers = {
        // Cassette 1, Layer 1
        {"Cu_c1l1_1", cuMat, 3210.6, 3210.7, 150.0, -1},
        {"SS_c1l1_1", stainlessMat, 3210.6, 3210.9, 150.0, -1},
        {"Pb_c1l1_1", pbMat, 3209.715, 3212.485, 150.0, -1},
        {"SS_c1l1_2", stainlessMat, 3213.77, 3214.07, 150.0, -1},
        {"Cu_c1l1_2", cuMat, 3214.17, 3214.27, 150.0, -1},
        {"Air_c1l1_1", airMat, 3214.2075, 3214.4325, 150.0, -1},
        {"PCB_c1l1_1", pcbMat, 3213.745, 3215.345, 150.0, -1},
        {"PCBHex_c1l1_1", pcbMat, 3219.075, 3220.675, 150.0, -1},
        
        // Cassette 34, Layer 47
        {"Kapton_c34l47_1", kaptonMat, 5131.635, 5131.735, 245.0, -1},
        {"BasePC_c34l47_1", pcbMat, 5132.26, 5133.26, 245.0, -1},
        {"CuCool_c34l47_1", cuMat, 5135.935, 5142.285, 245.0, -1}
    };
    
    // Silicon layers (will be segmented)
    std::vector<MaterialLayer> siliconLayers = {
        {"Si_c1l1", siMat, 3221.4, 3221.7, 150.0, 0},
        {"Si_c34l47", siMat, 5131.36, 5131.66, 245.0, 1}
        // Add all your silicon layers here with unique layerID
    };

    // ============================================
    // Create NON-SILICON layers (as cylinders)
    // ============================================
    int copyNum = 0;
    
    for (const auto& layer : nonSiliconLayers) {
        G4double zPos = (layer.zMin + layer.zMax) / 2.0 * mm;
        G4double thickness = (layer.zMax - layer.zMin) * mm;
        G4double outerR = layer.outerRadius * cm;
        
        if (outerR <= innerRadius) {
            outerR = innerRadius + 1.0*mm;
        }

        G4String solidName = layer.name + "_solid";
        G4Tubs* solid = new G4Tubs(solidName, innerRadius, outerR, thickness/2.0, 0.0, 2.0*CLHEP::pi);
        
        G4String logicName = layer.name + "_logic";
        G4LogicalVolume* logic = new G4LogicalVolume(solid, layer.material, logicName);
        
        G4String physName = layer.name + "_phys";
        new G4PVPlacement(0, G4ThreeVector(0, 0, zPos), logic, physName, logicWorld, false, copyNum, false);
        
        copyNum++;
    }

    G4cout << "Non-silicon layers created: " << copyNum << G4endl;

    // ============================================
    // Create SILICON layers (as segmented cells)
    // ============================================
    G4SDManager* sdManager = G4SDManager::GetSDMpointer();
    MySensitiveDetector* sensDet = new MySensitiveDetector("SensitiveDetector");
    sdManager->AddNewDetector(sensDet);
    
    int totalCells = 0;
    int siLayerCount = 0;
    
    for (const auto& siLayer : siliconLayers) {
        G4double zPos = (siLayer.zMin + siLayer.zMax) / 2.0 * mm;
        G4double thickness = (siLayer.zMax - siLayer.zMin) * mm;
        G4double outerR = siLayer.outerRadius * cm;
        
        // Generate cell positions for this layer
        std::vector<std::pair<G4double, G4double>> cellPositions = 
            GenerateCellPositions(cellSize, innerRadius, outerR);
        
        G4cout << "Layer " << siLayer.name << " at z=" << zPos/mm << " mm: " 
               << cellPositions.size() << " cells" << G4endl;
        
        // Create individual cell boxes
        for (const auto& pos : cellPositions) {
            G4double xCell = pos.first;
            G4double yCell = pos.second;
            
            // Calculate cell indices
            G4int cellX = static_cast<G4int>(std::round(xCell / cellSize));
            G4int cellY = static_cast<G4int>(std::round(yCell / cellSize));
            
            // Create cell box (half-sizes)
            G4Box* cellBox = new G4Box("SiCell_solid", 
                                       cellSize/2.0, cellSize/2.0, thickness/2.0);
            
            G4LogicalVolume* cellLogic = new G4LogicalVolume(cellBox, siMat, "SiCell_logic");
            
            // Encode layer and cell indices into copy number
            G4int cellCopyNum = EncodeCopyNumber(siLayer.layerID, cellX, cellY);
            
            // Place cell at position
            new G4PVPlacement(0, G4ThreeVector(xCell, yCell, zPos), 
                            cellLogic, "SiCell_phys", logicWorld, false, cellCopyNum, false);
            
            // Attach sensitive detector
            cellLogic->SetSensitiveDetector(sensDet);
            
            totalCells++;
        }
        
        siLayerCount++;
    }

    G4cout << "========================================" << G4endl;
    G4cout << "Detector Construction Summary:" << G4endl;
    G4cout << "Non-silicon layers: " << copyNum << G4endl;
    G4cout << "Silicon layers: " << siLayerCount << G4endl;
    G4cout << "Total silicon cells: " << totalCells << G4endl;
    G4cout << "Cell size: " << cellSize/mm << " mm" << G4endl;
    G4cout << "========================================" << G4endl;

    return physWorld;
}
