#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4Material.hh"
#include "globals.hh"
#include <vector>

// Structure to hold material layer information
struct MaterialLayer {
    G4String name;
    G4Material* material;
    G4double zMin;          // mm
    G4double zMax;          // mm
    G4double outerRadius;   // mm
    G4int layerNumber;      // -1 for non-silicon layers
};

class MyDetectorConstruction : public G4VUserDetectorConstruction {
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField() override;
};

#endif
