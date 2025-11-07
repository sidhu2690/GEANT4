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

MyDetectorConstruction::MyDetectorConstruction() {}
MyDetectorConstruction::~MyDetectorConstruction() {}

G4VPhysicalVolume* MyDetectorConstruction::Construct() {
    G4NistManager* nist = G4NistManager::Instance();

    // ---------------------
    // World volume: Large enough to contain all layers (using Box geometry)
    // ---------------------
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_Galactic");
    G4double worldSizeXY = 260.0*cm;  // Larger than max detector radius (256cm)
    G4double worldSizeZ = 520.0*cm;   // Larger than max z position (~513cm)
    
    G4Box* solidWorld = new G4Box("solidWorld", worldSizeXY, worldSizeXY, worldSizeZ);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

    G4VPhysicalVolume* physWorld = new G4PVPlacement(
        0,                      // rotation
        G4ThreeVector(0,0,0),   // position
        logicWorld,             // logical volume
        "physWorld",            // name
        0,                      // mother volume
        false,                  // boolean operation flag
        0,                      // copy number
        true                    // check overlaps
    );

    // ---------------------
    // Materials
    // ---------------------
    G4Material* siliconMat = nist->FindOrBuildMaterial("G4_Si");  // Silicon for layers
    G4Material* leadMat = nist->FindOrBuildMaterial("G4_Pb");     // Lead for absorbers

    // ---------------------
    // Layer data: z position (cm) and outer radius (cm)
    // ---------------------
    const int numLayers = 47;
    G4double layerZ[numLayers] = {
        322.155, 323.149, 325.212, 326.206, 328.269, 329.263, 331.326, 332.32, 334.383, 335.377,
        337.44, 338.434, 340.497, 341.491, 343.554, 344.548, 346.611, 347.605, 349.993, 350.987,
        353.375, 354.369, 356.757, 357.751, 360.139, 361.133, 367.976, 374.281, 380.586, 386.891,
        393.196, 399.501, 405.806, 411.916, 418.221, 424.526, 430.831, 439.056, 447.281, 455.506,
        463.731, 471.956, 480.181, 488.406, 496.631, 504.856, 513.081
    };
    
    G4double layerOuterRadius[numLayers] = {
        150, 142, 150, 142, 150, 142, 150, 142, 150, 150,
        150, 150, 150, 150, 150, 150, 150, 158, 158, 150,
        158, 150, 158, 158, 158, 158, 158, 164, 167, 169,
        167, 178, 183, 197, 206, 215, 215, 234, 245, 256,
        256, 256, 256, 256, 256, 256, 245
    };

    G4double innerRadius = 15.0*cm;     // Inner radius for all layers
    G4double layerThickness = 0.03*cm;   // Thickness of silicon layers (assumed 300 micrometer)
    G4double leadThickness = 5.6*mm;    // Thickness of lead absorbers
    G4double gap = 1.0*mm;              // Gap between silicon layer and lead absorber

    // ---------------------
    // Create layers and lead absorbers
    // ---------------------
    for (int i = 0; i < numLayers; i++) {
        G4double zPos = layerZ[i] * cm;
        G4double outerR = layerOuterRadius[i] * cm;
        
        // Create silicon layer
        G4String layerName = "solidLayer" + std::to_string(i);
        G4Tubs* solidLayer = new G4Tubs(layerName, innerRadius, outerR, layerThickness/2.0, 0.0, 2.0*CLHEP::pi);
        
        G4String logicLayerName = "logicLayer" + std::to_string(i);
        G4LogicalVolume* logicLayer = new G4LogicalVolume(solidLayer, siliconMat, logicLayerName);

	// G4VisAttributes* siliconVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0)); // cyan
	// siliconVis->SetForceSolid(true);
	// logicLayer->SetVisAttributes(siliconVis);
        
        G4String physLayerName = "physLayer" + std::to_string(i);
        new G4PVPlacement(
            0,                                    // rotation
            G4ThreeVector(0, 0, zPos),           // position
            logicLayer,                          // logical volume
            physLayerName,                       // name
            logicWorld,                          // mother volume
            false,                               // boolean operation flag
            i,                                   // copy number (layer number)
            false                                // check overlaps
        );
        
        // Create lead absorber (positioned 1mm after the silicon layer)
        G4double leadZPos = zPos + layerThickness/2.0 + gap + leadThickness/2.0;
        
        G4String leadName = "solidLead" + std::to_string(i);
        G4Tubs* solidLead = new G4Tubs(leadName, innerRadius, outerR, leadThickness/2.0, 0.0, 2.0*CLHEP::pi);
        
        G4String logicLeadName = "logicLead" + std::to_string(i);
        G4LogicalVolume* logicLead = new G4LogicalVolume(solidLead, leadMat, logicLeadName);

	// G4VisAttributes* leadVis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // gray
	// leadVis->SetForceSolid(true);
	// logicLead->SetVisAttributes(leadVis);
        
        G4String physLeadName = "physLead" + std::to_string(i);
        new G4PVPlacement(
            0,                                    // rotation
            G4ThreeVector(0, 0, leadZPos),       // position
            logicLead,                           // logical volume
            physLeadName,                        // name
            logicWorld,                          // mother volume
            false,                               // boolean operation flag
            i + 1000,                           // copy number (lead identifier: 1000 + layer)
            true                                // check overlaps
        );
    }

    // ---------------------
    // Register sensitive detector
    // ---------------------
    MySensitiveDetector* sensDet = new MySensitiveDetector("SensitiveDetector");
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);

    // Attach sensitive detector to all silicon layers
    for (int i = 0; i < numLayers; i++) {
        G4String logicLayerName = "logicLayer" + std::to_string(i);
        G4LogicalVolume* layerLogic =
            G4LogicalVolumeStore::GetInstance()->GetVolume(logicLayerName);

        if (layerLogic) {
            layerLogic->SetSensitiveDetector(sensDet);
        }
    }

    return physWorld;
}

