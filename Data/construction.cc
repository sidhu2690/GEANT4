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

MyDetectorConstruction::MyDetectorConstruction() {}
MyDetectorConstruction::~MyDetectorConstruction() {}

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
    // Materials from CSV
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
    // Material structure from CSV
    // ============================================
    struct MaterialLayer {
        G4String name;
        G4Material* material;
        G4double zMin;
        G4double zMax;
        G4double outerRadius;
    };
    
    std::vector<MaterialLayer> layers = {
        // Cassette 1, Layer 1 (z=3221.55mm, R=150mm)
        {"Cu_c1l1_1", cuMat, 3210.6, 3210.7, 150.0},
        {"SS_c1l1_1", stainlessMat, 3210.6, 3210.9, 150.0},
        //{"Glue_c1l1_1", glueMat, 3211.025, 3211.075, 150.0},
        {"Pb_c1l1_1", pbMat, 3209.715, 3212.485, 150.0},
        //{"Glue_c1l1_2", glueMat, 3213.845, 3213.895, 150.0},
        {"SS_c1l1_2", stainlessMat, 3213.77, 3214.07, 150.0},
        {"Cu_c1l1_2", cuMat, 3214.17, 3214.27, 150.0},
        {"Air_c1l1_1", airMat, 3214.2075, 3214.4325, 150.0},
        {"PCB_c1l1_1", pcbMat, 3213.745, 3215.345, 150.0},
        //{"AirElec_c1l1_1", airMat, 3214.28, 3218.01, 150.0},
        {"PCBHex_c1l1_1", pcbMat, 3219.075, 3220.675, 150.0},
        //{"Glue_c1l1_3", glueMat, 3221.4375, 3221.5125, 150.0},
        {"Si_c1l1", siMat, 3221.4, 3221.7, 150.0},

        // Cassette 1, Layer 2 (z=3231.49mm, R=142mm)
        //{"Glue_c1l2_1", glueMat, 3231.5825, 3231.6475, 142.0},
        {"Kapton_c1l2_1", kaptonMat, 3231.7475, 3232.0125, 142.0},
        //{"Glue_c1l2_2", glueMat, 3231.9125, 3231.9775, 142.0},
        {"CuW_c1l2_1", cuMat, 3232.645, 3234.045, 142.0},
        {"CuCool_c1l2_1", cuMat, 3226.37, 3232.42, 142.0},
        {"CuW_c1l2_2", cuMat, 3230.095, 3231.495, 142.0},
        //{"Glue_c1l2_3", glueMat, 3230.8275, 3230.8925, 142.0},
        {"Kapton_c1l2_2", kaptonMat, 3230.9925, 3231.2575, 142.0},
        //{"Glue_c1l2_4", glueMat, 3231.1575, 3231.2225, 142.0},
        {"Si_c1l2", siMat, 3231.34, 3231.64, 142.0},
        //{"Glue_c1l2_5", glueMat, 3231.5275, 3231.6025, 142.0},
        {"PCBHex_c1l2_1", pcbMat, 3232.365, 3233.965, 142.0},
        //{"AirElec_c1l2_1", airMat, 3235.03, 3238.76, 142.0},
        {"PCB_c1l2_1", pcbMat, 3237.695, 3239.295, 142.0},
        {"Air_c1l2_1", airMat, 3238.6075, 3238.8325, 142.0},
        {"Cu_c1l2_1", cuMat, 3238.77, 3238.87, 142.0},
        {"SS_c1l2_1", stainlessMat, 3238.97, 3239.27, 142.0},
        //{"Glue_c1l2_6", glueMat, 3239.145, 3239.195, 142.0},
        {"Pb_c1l2_1", pbMat, 3241.655, 3246.625, 142.0},
        //{"Glue_c1l2_7", glueMat, 3244.165, 3244.215, 142.0},
        {"SS_c1l2_2", stainlessMat, 3244.34, 3244.64, 142.0},
        {"Cu_c1l2_2", cuMat, 3244.54, 3244.64, 142.0},
        {"Air_c1l2_2", airMat, 3244.7025, 3244.9275, 142.0},
        {"PCB_c1l2_2", pcbMat, 3245.615, 3247.215, 142.0},
        //{"AirElec_c1l2_2", airMat, 3248.28, 3252.01, 142.0},
        {"PCBHex_c1l2_2", pcbMat, 3250.945, 3252.545, 142.0},
        //{"Glue_c1l2_8", glueMat, 3251.7825, 3251.8575, 142.0},

        // Cassette 2, Layer 3 (z=3252.12mm, R=150mm)
        {"Si_c2l3", siMat, 3251.97, 3252.27, 150.0},
        //{"Glue_c2l3_1", glueMat, 3252.1525, 3252.2175, 150.0},
        {"Kapton_c2l3_1", kaptonMat, 3252.3175, 3252.5825, 150.0},
        //{"Glue_c2l3_2", glueMat, 3252.4825, 3252.5475, 150.0},
        {"CuW_c2l3_1", cuMat, 3253.215, 3254.615, 150.0},
        {"CuCool_c2l3_1", cuMat, 3256.94, 3262.99, 150.0},
        {"CuW_c2l3_2", cuMat, 3260.665, 3262.065, 150.0},
        //{"Glue_c2l3_3", glueMat, 3261.3975, 3261.4625, 150.0},
        {"Kapton_c2l3_2", kaptonMat, 3261.5625, 3261.8275, 150.0},
        //{"Glue_c2l3_4", glueMat, 3261.7275, 3261.7925, 150.0},

        // Cassette 2, Layer 4 (z=3262.06mm, R=142mm)
        {"Si_c2l4", siMat, 3261.91, 3262.21, 142.0},
        //{"Glue_c2l4_1", glueMat, 3262.0975, 3262.1725, 142.0},
        {"PCBHex_c2l4_1", pcbMat, 3262.935, 3264.535, 142.0},
        //{"AirElec_c2l4_1", airMat, 3265.6, 3269.33, 142.0},
        {"PCB_c2l4_1", pcbMat, 3268.265, 3269.865, 142.0},
        {"Air_c2l4_1", airMat, 3269.1775, 3269.4025, 142.0},
        {"Cu_c2l4_1", cuMat, 3269.34, 3269.44, 142.0},
        {"SS_c2l4_1", stainlessMat, 3269.54, 3269.84, 142.0},
        //{"Glue_c2l4_2", glueMat, 3269.715, 3269.765, 142.0},
        {"Pb_c2l4_1", pbMat, 3272.225, 3277.195, 142.0},
        //{"Glue_c2l4_3", glueMat, 3274.735, 3274.785, 142.0},
        {"SS_c2l4_2", stainlessMat, 3274.91, 3275.21, 142.0},
        {"Cu_c2l4_2", cuMat, 3275.11, 3275.21, 142.0},
        {"Air_c2l4_2", airMat, 3275.2725, 3275.4975, 142.0},
        {"PCB_c2l4_2", pcbMat, 3276.185, 3277.785, 142.0},
        //{"AirElec_c2l4_2", airMat, 3278.85, 3282.58, 142.0},
        {"PCBHex_c2l4_2", pcbMat, 3281.515, 3283.115, 142.0},
        //{"Glue_c2l4_4", glueMat, 3282.3525, 3282.4275, 142.0},

        // Cassette 3, Layer 5 (z=3282.69mm, R=150mm)
        {"Si_c3l5", siMat, 3282.54, 3282.84, 150.0},
        //{"Glue_c3l5_1", glueMat, 3282.7225, 3282.7875, 150.0},
        {"Kapton_c3l5_1", kaptonMat, 3282.8875, 3283.1525, 150.0},
        //{"Glue_c3l5_2", glueMat, 3283.0525, 3283.1175, 150.0},
        {"CuW_c3l5_1", cuMat, 3283.785, 3285.185, 150.0},
        {"CuCool_c3l5_1", cuMat, 3287.51, 3293.56, 150.0},
        {"CuW_c3l5_2", cuMat, 3291.235, 3292.635, 150.0},
        //{"Glue_c3l5_3", glueMat, 3291.9675, 3292.0325, 150.0},
        {"Kapton_c3l5_2", kaptonMat, 3292.1325, 3292.3975, 150.0},
        //{"Glue_c3l5_4", glueMat, 3292.2975, 3292.3625, 150.0},

        // Cassette 3, Layer 6 (z=3292.63mm, R=150mm)
        {"Si_c3l6", siMat, 3292.48, 3292.78, 150.0},
        //{"Glue_c3l6_1", glueMat, 3292.6675, 3292.7425, 150.0},
        {"PCBHex_c3l6_1", pcbMat, 3293.505, 3295.105, 150.0},
        //{"AirElec_c3l6_1", airMat, 3296.17, 3299.9, 150.0},
        {"PCB_c3l6_1", pcbMat, 3298.835, 3300.435, 150.0},
        {"Air_c3l6_1", airMat, 3299.7475, 3299.9725, 150.0},
        {"Cu_c3l6_1", cuMat, 3299.91, 3300.01, 150.0},
        {"SS_c3l6_1", stainlessMat, 3300.11, 3300.41, 150.0},
        //{"Glue_c3l6_2", glueMat, 3300.285, 3300.335, 150.0},
        {"Pb_c3l6_1", pbMat, 3302.795, 3307.765, 150.0},
        //{"Glue_c3l6_3", glueMat, 3305.305, 3305.355, 150.0},
        {"SS_c3l6_2", stainlessMat, 3305.48, 3305.78, 150.0},
        {"Cu_c3l6_2", cuMat, 3305.68, 3305.78, 150.0},
        {"Air_c3l6_2", airMat, 3305.8425, 3306.0675, 150.0},
        {"PCB_c3l6_2", pcbMat, 3306.755, 3308.355, 150.0},
        //{"AirElec_c3l6_2", airMat, 3309.42, 3313.15, 150.0},
        {"PCBHex_c3l6_2", pcbMat, 3312.085, 3313.685, 150.0},
        //{"Glue_c3l6_4", glueMat, 3312.9225, 3312.9975, 150.0},

        // Cassette 4, Layer 7 (z=3313.26mm, R=150mm)
        {"Si_c4l7", siMat, 3313.11, 3313.41, 150.0},
        //{"Glue_c4l7_1", glueMat, 3313.2925, 3313.3575, 150.0},
        {"Kapton_c4l7_1", kaptonMat, 3313.4575, 3313.7225, 150.0},
        //{"Glue_c4l7_2", glueMat, 3313.6225, 3313.6875, 150.0},
        {"CuW_c4l7_1", cuMat, 3314.355, 3315.755, 150.0},
        {"CuCool_c4l7_1", cuMat, 3318.08, 3324.13, 150.0},
        {"CuW_c4l7_2", cuMat, 3321.805, 3323.205, 150.0},
        //{"Glue_c4l7_3", glueMat, 3322.5375, 3322.6025, 150.0},
        {"Kapton_c4l7_2", kaptonMat, 3322.7025, 3322.9675, 150.0},
        //{"Glue_c4l7_4", glueMat, 3322.8675, 3322.9325, 150.0},

        // Cassette 4, Layer 8 (z=3323.2mm, R=142mm)
        {"Si_c4l8", siMat, 3323.05, 3323.35, 142.0},
        //{"Glue_c4l8_1", glueMat, 3323.2375, 3323.3125, 142.0},
        {"PCBHex_c4l8_1", pcbMat, 3324.075, 3325.675, 142.0},
        //{"AirElec_c4l8_1", airMat, 3326.74, 3330.47, 142.0},
        {"PCB_c4l8_1", pcbMat, 3329.405, 3331.005, 142.0},
        {"Air_c4l8_1", airMat, 3330.3175, 3330.5425, 142.0},
        {"Cu_c4l8_1", cuMat, 3330.48, 3330.58, 142.0},
        {"SS_c4l8_1", stainlessMat, 3330.68, 3330.98, 142.0},
        //{"Glue_c4l8_2", glueMat, 3330.855, 3330.905, 142.0},
        {"Pb_c4l8_1", pbMat, 3333.365, 3338.335, 142.0},
        //{"Glue_c4l8_3", glueMat, 3335.875, 3335.925, 142.0},
        {"SS_c4l8_2", stainlessMat, 3336.05, 3336.35, 142.0},
        {"Cu_c4l8_2", cuMat, 3336.25, 3336.35, 142.0},
        {"Air_c4l8_2", airMat, 3336.4125, 3336.6375, 142.0},
        {"PCB_c4l8_2", pcbMat, 3337.325, 3338.925, 142.0},
        //{"AirElec_c4l8_2", airMat, 3339.99, 3343.72, 142.0},
        {"PCBHex_c4l8_2", pcbMat, 3342.655, 3344.255, 142.0},
        //{"Glue_c4l8_4", glueMat, 3343.4925, 3343.5675, 142.0},

        // Cassette 5, Layer 9 (z=3343.83mm, R=150mm)
        {"Si_c5l9", siMat, 3343.68, 3343.98, 150.0},
        //{"Glue_c5l9_1", glueMat, 3343.8625, 3343.9275, 150.0},
        {"Kapton_c5l9_1", kaptonMat, 3344.0275, 3344.2925, 150.0},
        //{"Glue_c5l9_2", glueMat, 3344.1925, 3344.2575, 150.0},
        {"CuW_c5l9_1", cuMat, 3344.925, 3346.325, 150.0},
        {"CuCool_c5l9_1", cuMat, 3348.65, 3354.7, 150.0},
        {"CuW_c5l9_2", cuMat, 3352.375, 3353.775, 150.0},
        //{"Glue_c5l9_3", glueMat, 3353.1075, 3353.1725, 150.0},
        {"Kapton_c5l9_2", kaptonMat, 3353.2725, 3353.5375, 150.0},
        //{"Glue_c5l9_4", glueMat, 3353.4375, 3353.5025, 150.0},

        // Cassette 5, Layer 10 (z=3353.77mm, R=150mm)
        {"Si_c5l10", siMat, 3353.62, 3353.92, 150.0},
        //{"Glue_c5l10_1", glueMat, 3353.8075, 3353.8825, 150.0},
        {"PCBHex_c5l10_1", pcbMat, 3354.645, 3356.245, 150.0},
        //{"AirElec_c5l10_1", airMat, 3357.31, 3361.04, 150.0},
        {"PCB_c5l10_1", pcbMat, 3359.975, 3361.575, 150.0},
        {"Air_c5l10_1", airMat, 3360.8875, 3361.1125, 150.0},
        {"Cu_c5l10_1", cuMat, 3361.05, 3361.15, 150.0},
        {"SS_c5l10_1", stainlessMat, 3361.25, 3361.55, 150.0},
        //{"Glue_c5l10_2", glueMat, 3361.425, 3361.475, 150.0},
        {"Pb_c5l10_1", pbMat, 3363.935, 3368.905, 150.0},
        //{"Glue_c5l10_3", glueMat, 3366.445, 3366.495, 150.0},
        {"SS_c5l10_2", stainlessMat, 3366.62, 3366.92, 150.0},
        {"Cu_c5l10_2", cuMat, 3366.82, 3366.92, 150.0},
        {"Air_c5l10_2", airMat, 3366.9825, 3367.2075, 150.0},
        {"PCB_c5l10_2", pcbMat, 3367.895, 3369.495, 150.0},
        //{"AirElec_c5l10_2", airMat, 3370.56, 3374.29, 150.0},
        {"PCBHex_c5l10_2", pcbMat, 3373.225, 3374.825, 150.0},
        //{"Glue_c5l10_4", glueMat, 3374.0625, 3374.1375, 150.0},

        // Cassette 6, Layer 11 (z=3374.4mm, R=150mm)
        {"Si_c6l11", siMat, 3374.25, 3374.55, 150.0},
        //{"Glue_c6l11_1", glueMat, 3374.4325, 3374.4975, 150.0},
        {"Kapton_c6l11_1", kaptonMat, 3374.5975, 3374.8625, 150.0},
        //{"Glue_c6l11_2", glueMat, 3374.7625, 3374.8275, 150.0},
        {"CuW_c6l11_1", cuMat, 3375.495, 3376.895, 150.0},
        {"CuCool_c6l11_1", cuMat, 3379.22, 3385.27, 150.0},
        {"CuW_c6l11_2", cuMat, 3382.945, 3384.345, 150.0},
        //{"Glue_c6l11_3", glueMat, 3383.6775, 3383.7425, 150.0},
        {"Kapton_c6l11_2", kaptonMat, 3383.8425, 3384.1075, 150.0},
        //{"Glue_c6l11_4", glueMat, 3384.0075, 3384.0725, 150.0},

        // Cassette 6, Layer 12 (z=3384.34mm, R=150mm)
        {"Si_c6l12", siMat, 3384.19, 3384.49, 150.0},
        //{"Glue_c6l12_1", glueMat, 3384.3775, 3384.4525, 150.0},
        {"PCBHex_c6l12_1", pcbMat, 3385.215, 3386.815, 150.0},
        //{"AirElec_c6l12_1", airMat, 3387.88, 3391.61, 150.0},
        {"PCB_c6l12_1", pcbMat, 3390.545, 3392.145, 150.0},
        {"Air_c6l12_1", airMat, 3391.4575, 3391.6825, 150.0},
        {"Cu_c6l12_1", cuMat, 3391.62, 3391.72, 150.0},
        {"SS_c6l12_1", stainlessMat, 3391.82, 3392.12, 150.0},
        //{"Glue_c6l12_2", glueMat, 3391.995, 3392.045, 150.0},
        {"Pb_c6l12_1", pbMat, 3394.505, 3399.475, 150.0},
        //{"Glue_c6l12_3", glueMat, 3397.015, 3397.065, 150.0},
        {"SS_c6l12_2", stainlessMat, 3397.19, 3397.49, 150.0},
        {"Cu_c6l12_2", cuMat, 3397.39, 3397.49, 150.0},
        {"Air_c6l12_2", airMat, 3397.5525, 3397.7775, 150.0},
        {"PCB_c6l12_2", pcbMat, 3398.465, 3400.065, 150.0},
        //{"AirElec_c6l12_2", airMat, 3401.13, 3404.86, 150.0},
        {"PCBHex_c6l12_2", pcbMat, 3403.795, 3405.395, 150.0},
        //{"Glue_c6l12_4", glueMat, 3404.6325, 3404.7075, 150.0},

        // Cassette 7, Layer 13 (z=3404.97mm, R=150mm)
        {"Si_c7l13", siMat, 3404.82, 3405.12, 150.0},
        //{"Glue_c7l13_1", glueMat, 3405.0025, 3405.0675, 150.0},
        {"Kapton_c7l13_1", kaptonMat, 3405.1675, 3405.4325, 150.0},
        //{"Glue_c7l13_2", glueMat, 3405.3325, 3405.3975, 150.0},
        {"CuW_c7l13_1", cuMat, 3406.065, 3407.465, 150.0},
        {"CuCool_c7l13_1", cuMat, 3409.79, 3415.84, 150.0},
        {"CuW_c7l13_2", cuMat, 3413.515, 3414.915, 150.0},
        //{"Glue_c7l13_3", glueMat, 3414.2475, 3414.3125, 150.0},
        {"Kapton_c7l13_2", kaptonMat, 3414.4125, 3414.6775, 150.0},
        //{"Glue_c7l13_4", glueMat, 3414.5775, 3414.6425, 150.0},

        // Cassette 7, Layer 14 (z=3414.91mm, R=150mm)
        {"Si_c7l14", siMat, 3414.76, 3415.06, 150.0},
        //{"Glue_c7l14_1", glueMat, 3414.9475, 3415.0225, 150.0},
        {"PCBHex_c7l14_1", pcbMat, 3415.785, 3417.385, 150.0},
        //{"AirElec_c7l14_1", airMat, 3418.45, 3422.18, 150.0},
        {"PCB_c7l14_1", pcbMat, 3421.115, 3422.715, 150.0},
        {"Air_c7l14_1", airMat, 3422.0275, 3422.2525, 150.0},
        {"Cu_c7l14_1", cuMat, 3422.19, 3422.29, 150.0},
        {"SS_c7l14_1", stainlessMat, 3422.39, 3422.69, 150.0},
        //{"Glue_c7l14_2", glueMat, 3422.565, 3422.615, 150.0},
        {"Pb_c7l14_1", pbMat, 3425.075, 3430.045, 150.0},
        //{"Glue_c7l14_3", glueMat, 3427.585, 3427.635, 150.0},
        {"SS_c7l14_2", stainlessMat, 3427.76, 3428.06, 150.0},
        {"Cu_c7l14_2", cuMat, 3427.96, 3428.06, 150.0},
        {"Air_c7l14_2", airMat, 3428.1225, 3428.3475, 150.0},
        {"PCB_c7l14_2", pcbMat, 3429.035, 3430.635, 150.0},
        //{"AirElec_c7l14_2", airMat, 3431.7, 3435.43, 150.0},
        {"PCBHex_c7l14_2", pcbMat, 3434.365, 3435.965, 150.0},
        //{"Glue_c7l14_4", glueMat, 3435.2025, 3435.2775, 150.0},

        // Cassette 8, Layer 15 (z=3435.54mm, R=150mm)
        {"Si_c8l15", siMat, 3435.39, 3435.69, 150.0},
        //{"Glue_c8l15_1", glueMat, 3435.5725, 3435.6375, 150.0},
        {"Kapton_c8l15_1", kaptonMat, 3435.7375, 3436.0025, 150.0},
        //{"Glue_c8l15_2", glueMat, 3435.9025, 3435.9675, 150.0},
        {"CuW_c8l15_1", cuMat, 3436.635, 3438.035, 150.0},
        {"CuCool_c8l15_1", cuMat, 3440.36, 3446.41, 150.0},
        {"CuW_c8l15_2", cuMat, 3444.085, 3445.485, 150.0},
        //{"Glue_c8l15_3", glueMat, 3444.8175, 3444.8825, 150.0},
        {"Kapton_c8l15_2", kaptonMat, 3444.9825, 3445.2475, 150.0},
        //{"Glue_c8l15_4", glueMat, 3445.1475, 3445.2125, 150.0},

        // Cassette 8, Layer 16 (z=3445.48mm, R=150mm)
        {"Si_c8l16", siMat, 3445.33, 3445.63, 150.0},
        //{"Glue_c8l16_1", glueMat, 3445.5175, 3445.5925, 150.0},
        {"PCBHex_c8l16_1", pcbMat, 3446.355, 3447.955, 150.0},
        //{"AirElec_c8l16_1", airMat, 3449.02, 3452.75, 150.0},
        {"PCB_c8l16_1", pcbMat, 3451.685, 3453.285, 150.0},
        {"Air_c8l16_1", airMat, 3452.5975, 3452.8225, 150.0},
        {"Cu_c8l16_1", cuMat, 3452.76, 3452.86, 150.0},
        {"SS_c8l16_1", stainlessMat, 3452.96, 3453.26, 150.0},
        //{"Glue_c8l16_2", glueMat, 3453.135, 3453.185, 150.0},
        {"Pb_c8l16_1", pbMat, 3455.645, 3460.615, 150.0},
        //{"Glue_c8l16_3", glueMat, 3458.155, 3458.205, 150.0},
        {"SS_c8l16_2", stainlessMat, 3458.33, 3458.63, 150.0},
        {"Cu_c8l16_2", cuMat, 3458.53, 3458.63, 150.0},
        {"Air_c8l16_2", airMat, 3458.6925, 3458.9175, 150.0},
        {"PCB_c8l16_2", pcbMat, 3459.605, 3461.205, 150.0},
        //{"AirElec_c8l16_2", airMat, 3462.27, 3466.0, 150.0},
        {"PCBHex_c8l16_2", pcbMat, 3464.935, 3466.535, 150.0},
        //{"Glue_c8l16_4", glueMat, 3465.7725, 3465.8475, 150.0},

        // Cassette 9, Layer 17 (z=3466.11mm, R=158mm)
        {"Si_c9l17", siMat, 3465.96, 3466.26, 158.0},
        //{"Glue_c9l17_1", glueMat, 3466.1425, 3466.2075, 158.0},
        {"Kapton_c9l17_1", kaptonMat, 3466.3075, 3466.5725, 158.0},
        //{"Glue_c9l17_2", glueMat, 3466.4725, 3466.5375, 158.0},
        {"CuW_c9l17_1", cuMat, 3467.205, 3468.605, 158.0},
        {"CuCool_c9l17_1", cuMat, 3470.93, 3476.98, 158.0},
        {"CuW_c9l17_2", cuMat, 3474.655, 3476.055, 158.0},
        //{"Glue_c9l17_3", glueMat, 3475.3875, 3475.4525, 158.0},
        {"Kapton_c9l17_2", kaptonMat, 3475.5525, 3475.8175, 158.0},
        //{"Glue_c9l17_4", glueMat, 3475.7175, 3475.7825, 158.0},

        // Cassette 9, Layer 18 (z=3476.05mm, R=158mm)
        {"Si_c9l18", siMat, 3475.9, 3476.2, 158.0},
        //{"Glue_c9l18_1", glueMat, 3476.0875, 3476.1625, 158.0},
        {"PCBHex_c9l18_1", pcbMat, 3476.925, 3478.525, 158.0},
        //{"AirElec_c9l18_1", airMat, 3479.59, 3483.32, 158.0},
        {"PCB_c9l18_1", pcbMat, 3482.255, 3483.855, 158.0},
        {"Air_c9l18_1", airMat, 3483.1675, 3483.3925, 158.0},
        {"Cu_c9l18_1", cuMat, 3483.33, 3483.43, 158.0},
        {"SS_c9l18_1", stainlessMat, 3483.53, 3483.83, 158.0},
        //{"Glue_c9l18_2", glueMat, 3483.705, 3483.755, 158.0},
        {"Pb_c9l18_1", pbMat, 3487.84, 3496.06, 158.0},
        //{"Glue_c9l18_3", glueMat, 3491.975, 3492.025, 158.0},
        {"SS_c9l18_2", stainlessMat, 3492.15, 3492.45, 158.0},
        {"Cu_c9l18_2", cuMat, 3492.35, 3492.45, 158.0},
        {"Air_c9l18_2", airMat, 3492.5125, 3492.7375, 158.0},
        {"PCB_c9l18_2", pcbMat, 3493.425, 3495.025, 158.0},
        //{"AirElec_c9l18_2", airMat, 3496.09, 3499.82, 158.0},
        {"PCBHex_c9l18_2", pcbMat, 3498.755, 3500.355, 158.0},
        //{"Glue_c9l18_4", glueMat, 3499.5925, 3499.6675, 158.0},

        // Cassette 10, Layer 19 (z=3499.93mm, R=158mm)
        {"Si_c10l19", siMat, 3499.78, 3500.08, 158.0},
        //{"Glue_c10l19_1", glueMat, 3499.9625, 3500.0275, 158.0},
        {"Kapton_c10l19_1", kaptonMat, 3500.1275, 3500.3925, 158.0},
        //{"Glue_c10l19_2", glueMat, 3500.2925, 3500.3575, 158.0},
        {"CuW_c10l19_1", cuMat, 3501.025, 3502.425, 158.0},
        {"CuCool_c10l19_1", cuMat, 3504.75, 3510.8, 158.0},
        {"CuW_c10l19_2", cuMat, 3508.475, 3509.875, 158.0},
        //{"Glue_c10l19_3", glueMat, 3509.2075, 3509.2725, 158.0},
        {"Kapton_c10l19_2", kaptonMat, 3509.3725, 3509.6375, 158.0},
        //{"Glue_c10l19_4", glueMat, 3509.5375, 3509.6025, 158.0},

        // Cassette 10, Layer 20 (z=3509.87mm, R=158mm)
        {"Si_c10l20", siMat, 3509.72, 3510.02, 158.0},
        //{"Glue_c10l20_1", glueMat, 3509.9075, 3509.9825, 158.0},
        {"PCBHex_c10l20_1", pcbMat, 3510.745, 3512.345, 158.0},
        //{"AirElec_c10l20_1", airMat, 3513.41, 3517.14, 158.0},
        {"PCB_c10l20_1", pcbMat, 3516.075, 3517.675, 158.0},
        {"Air_c10l20_1", airMat, 3516.9875, 3517.2125, 158.0},
        {"Cu_c10l20_1", cuMat, 3517.15, 3517.25, 158.0},
        {"SS_c10l20_1", stainlessMat, 3517.35, 3517.65, 158.0},
        //{"Glue_c10l20_2", glueMat, 3517.525, 3517.575, 158.0},
        {"Pb_c10l20_1", pbMat, 3521.66, 3529.88, 158.0},
        //{"Glue_c10l20_3", glueMat, 3525.795, 3525.845, 158.0},
        {"SS_c10l20_2", stainlessMat, 3525.97, 3526.27, 158.0},
        {"Cu_c10l20_2", cuMat, 3526.17, 3526.27, 158.0},
        {"Air_c10l20_2", airMat, 3526.3325, 3526.5575, 158.0},
        {"PCB_c10l20_2", pcbMat, 3527.245, 3528.845, 158.0},
        //{"AirElec_c10l20_2", airMat, 3529.91, 3533.64, 158.0},
        {"PCBHex_c10l20_2", pcbMat, 3532.575, 3534.175, 158.0},
        //{"Glue_c10l20_4", glueMat, 3533.4125, 3533.4875, 158.0},

        // Cassette 11, Layer 21 (z=3533.75mm, R=158mm)
        {"Si_c11l21", siMat, 3533.6, 3533.9, 158.0},
        //{"Glue_c11l21_1", glueMat, 3533.7825, 3533.8475, 158.0},
        {"Kapton_c11l21_1", kaptonMat, 3533.9475, 3534.2125, 158.0},
        //{"Glue_c11l21_2", glueMat, 3534.1125, 3534.1775, 158.0},
        {"CuW_c11l21_1", cuMat, 3534.845, 3536.245, 158.0},
        {"CuCool_c11l21_1", cuMat, 3538.57, 3544.62, 158.0},
        {"CuW_c11l21_2", cuMat, 3542.295, 3543.695, 158.0},
        //{"Glue_c11l21_3", glueMat, 3543.0275, 3543.0925, 158.0},
        {"Kapton_c11l21_2", kaptonMat, 3543.1925, 3543.4575, 158.0},
        //{"Glue_c11l21_4", glueMat, 3543.3575, 3543.4225, 158.0},

        // Cassette 11, Layer 22 (z=3543.69mm, R=158mm)
        {"Si_c11l22", siMat, 3543.54, 3543.84, 158.0},
        //{"Glue_c11l22_1", glueMat, 3543.7275, 3543.8025, 158.0},
        {"PCBHex_c11l22_1", pcbMat, 3544.565, 3546.165, 158.0},
        //{"AirElec_c11l22_1", airMat, 3547.23, 3550.96, 158.0},
        {"PCB_c11l22_1", pcbMat, 3549.895, 3551.495, 158.0},
        {"Air_c11l22_1", airMat, 3550.8075, 3551.0325, 158.0},
        {"Cu_c11l22_1", cuMat, 3550.97, 3551.07, 158.0},
        {"SS_c11l22_1", stainlessMat, 3551.17, 3551.47, 158.0},
        //{"Glue_c11l22_2", glueMat, 3551.345, 3551.395, 158.0},
        {"Pb_c11l22_1", pbMat, 3555.48, 3563.7, 158.0},
        //{"Glue_c11l22_3", glueMat, 3559.615, 3559.665, 158.0},
        {"SS_c11l22_2", stainlessMat, 3559.79, 3560.09, 158.0},
        {"Cu_c11l22_2", cuMat, 3559.99, 3560.09, 158.0},
        {"Air_c11l22_2", airMat, 3560.1525, 3560.3775, 158.0},
        {"PCB_c11l22_2", pcbMat, 3561.065, 3562.665, 158.0},
        //{"AirElec_c11l22_2", airMat, 3563.73, 3567.46, 158.0},
        {"PCBHex_c11l22_2", pcbMat, 3566.395, 3567.995, 158.0},
        //{"Glue_c11l22_4", glueMat, 3567.2325, 3567.3075, 158.0},

        // Cassette 12, Layer 23 (z=3567.57mm, R=158mm)
        {"Si_c12l23", siMat, 3567.42, 3567.72, 158.0},
        //{"Glue_c12l23_1", glueMat, 3567.6025, 3567.6675, 158.0},
        {"Kapton_c12l23_1", kaptonMat, 3567.7675, 3568.0325, 158.0},
        //{"Glue_c12l23_2", glueMat, 3567.9325, 3567.9975, 158.0},
        {"CuW_c12l23_1", cuMat, 3568.665, 3570.065, 158.0},
        {"CuCool_c12l23_1", cuMat, 3572.39, 3578.44, 158.0},
        {"CuW_c12l23_2", cuMat, 3576.115, 3577.515, 158.0},
        //{"Glue_c12l23_3", glueMat, 3576.8475, 3576.9125, 158.0},
        {"Kapton_c12l23_2", kaptonMat, 3577.0125, 3577.2775, 158.0},
        //{"Glue_c12l23_4", glueMat, 3577.1775, 3577.2425, 158.0},

        // Cassette 12, Layer 24 (z=3577.51mm, R=158mm)
        {"Si_c12l24", siMat, 3577.36, 3577.66, 158.0},
        //{"Glue_c12l24_1", glueMat, 3577.5475, 3577.6225, 158.0},
        {"PCBHex_c12l24_1", pcbMat, 3578.385, 3579.985, 158.0},
        //{"AirElec_c12l24_1", airMat, 3581.05, 3584.78, 158.0},
        {"PCB_c12l24_1", pcbMat, 3583.715, 3585.315, 158.0},
        {"Air_c12l24_1", airMat, 3584.6275, 3584.8525, 158.0},
        {"Cu_c12l24_1", cuMat, 3584.79, 3584.89, 158.0},
        {"SS_c12l24_1", stainlessMat, 3584.99, 3585.29, 158.0},
        //{"Glue_c12l24_2", glueMat, 3585.165, 3585.215, 158.0},
        {"Pb_c12l24_1", pbMat, 3589.3, 3597.52, 158.0},
        //{"Glue_c12l24_3", glueMat, 3593.435, 3593.485, 158.0},
        {"SS_c12l24_2", stainlessMat, 3593.61, 3593.91, 158.0},
        {"Cu_c12l24_2", cuMat, 3593.81, 3593.91, 158.0},
        {"Air_c12l24_2", airMat, 3593.9725, 3594.1975, 158.0},
        {"PCB_c12l24_2", pcbMat, 3594.885, 3596.485, 158.0},
        //{"AirElec_c12l24_2", airMat, 3597.55, 3601.28, 158.0},
        {"PCBHex_c12l24_2", pcbMat, 3600.215, 3601.815, 158.0},
        //{"Glue_c12l24_4", glueMat, 3601.0525, 3601.1275, 158.0},

        // Cassette 13, Layer 25 (z=3601.39mm, R=158mm)
        {"Si_c13l25", siMat, 3601.24, 3601.54, 158.0},
        //{"Glue_c13l25_1", glueMat, 3601.4225, 3601.4875, 158.0},
        {"Kapton_c13l25_1", kaptonMat, 3601.5875, 3601.8525, 158.0},
        //{"Glue_c13l25_2", glueMat, 3601.7525, 3601.8175, 158.0},
        {"CuW_c13l25_1", cuMat, 3602.485, 3603.885, 158.0},
        {"CuCool_c13l25_1", cuMat, 3606.21, 3612.26, 158.0},
        {"CuW_c13l25_2", cuMat, 3609.935, 3611.335, 158.0},
        //{"Glue_c13l25_3", glueMat, 3610.6675, 3610.7325, 158.0},
        {"Kapton_c13l25_2", kaptonMat, 3610.8325, 3611.0975, 158.0},
        //{"Glue_c13l25_4", glueMat, 3610.9975, 3611.0625, 158.0},

        // Cassette 13, Layer 26 (z=3611.33mm, R=158mm)
        {"Si_c13l26", siMat, 3611.18, 3611.48, 158.0},
        //{"Glue_c13l26_1", glueMat, 3611.3675, 3611.4425, 158.0},
        {"PCBHex_c13l26_1", pcbMat, 3612.205, 3613.805, 158.0},
        //{"AirElec_c13l26_1", airMat, 3614.87, 3618.6, 158.0},
        {"PCB_c13l26_1", pcbMat, 3617.535, 3619.135, 158.0},
        {"Air_c13l26_1", airMat, 3618.4475, 3618.6725, 158.0},
        {"SS_c13l26_1", stainlessMat, 3619.06, 3620.06, 158.0},
        {"AbsorberPlate_HE_1", stainlessMat, 3642.06, 3687.06, 158.0},
        {"Air_HE_1", airMat, 3666.56, 3670.56, 158.0},
        {"SS_HE_1", stainlessMat, 3669.81, 3672.31, 158.0},
        {"Air_HE_2", airMat, 3671.26, 3671.66, 158.0},
        {"PCB_HE_1", pcbMat, 3672.26, 3673.86, 158.0},
        //{"AirElec_HE_1", airMat, 3674.7975, 3678.2725, 158.0},
        {"PCBHex_HE_1", pcbMat, 3677.335, 3678.935, 158.0},
        //{"Glue_HE_1", glueMat, 3678.1725, 3678.2475, 158.0},

        // Cassette 14, Layer 27 (z=3678.51mm, R=158mm)
        {"Si_c14l27", siMat, 3678.36, 3678.66, 158.0},
        //{"Glue_c14l27_1", glueMat, 3678.5475, 3678.6225, 158.0},
        {"Kapton_c14l27_1", kaptonMat, 3678.635, 3678.735, 158.0},
        //{"Glue_c14l27_2", glueMat, 3678.7225, 3678.7975, 158.0},
        {"BasePC_c14l27_1", pcbMat, 3679.26, 3680.26, 158.0},
        {"CuCool_c14l27_1", cuMat, 3682.935, 3689.285, 158.0},
        {"AbsorberPlate_c14l27_1", stainlessMat, 3706.86, 3748.36, 158.0},
        {"Air_c14l27_1", airMat, 3729.61, 3733.61, 158.0},
        {"SS_c14l27_1", stainlessMat, 3732.86, 3735.36, 158.0},
        {"Air_c14l27_2", airMat, 3734.31, 3734.71, 158.0},
        {"PCB_c14l27_1", pcbMat, 3735.31, 3736.91, 158.0},
        //{"AirElec_c14l27_1", airMat, 3737.8475, 3741.3225, 158.0},
        {"PCBHex_c14l27_1", pcbMat, 3740.385, 3741.985, 158.0},
        //{"Glue_c14l27_3", glueMat, 3741.2225, 3741.2975, 158.0},

        // Cassette 15, Layer 28 (z=3741.56mm, R=164mm)
        {"Si_c15l28", siMat, 3741.41, 3741.71, 164.0},
        //{"Glue_c15l28_1", glueMat, 3741.5975, 3741.6725, 164.0},
        {"Kapton_c15l28_1", kaptonMat, 3741.685, 3741.785, 164.0},
        //{"Glue_c15l28_2", glueMat, 3741.7725, 3741.8475, 164.0},
        {"BasePC_c15l28_1", pcbMat, 3742.31, 3743.31, 164.0},
        {"CuCool_c15l28_1", cuMat, 3745.985, 3752.335, 164.0},
        {"AbsorberPlate_c15l28_1", stainlessMat, 3769.91, 3811.41, 164.0},
        {"Air_c15l28_1", airMat, 3792.66, 3796.66, 164.0},
        {"SS_c15l28_1", stainlessMat, 3795.91, 3798.41, 164.0},
        {"Air_c15l28_2", airMat, 3797.36, 3797.76, 164.0},
        {"PCB_c15l28_1", pcbMat, 3798.36, 3799.96, 164.0},
        //{"AirElec_c15l28_1", airMat, 3800.8975, 3804.3725, 164.0},
        {"PCBHex_c15l28_1", pcbMat, 3803.435, 3805.035, 164.0},
        //{"Glue_c15l28_2", glueMat, 3804.2725, 3804.3475, 164.0},

        // Cassette 16, Layer 29 (z=3804.61mm, R=167mm)
        {"Si_c16l29", siMat, 3804.46, 3804.76, 167.0},
        //{"Glue_c16l29_1", glueMat, 3804.6475, 3804.7225, 167.0},
        {"Kapton_c16l29_1", kaptonMat, 3804.735, 3804.835, 167.0},
        //{"Glue_c16l29_2", glueMat, 3804.8225, 3804.8975, 167.0},
        {"BasePC_c16l29_1", pcbMat, 3805.36, 3806.36, 167.0},
        {"CuCool_c16l29_1", cuMat, 3809.035, 3815.385, 167.0},
        {"AbsorberPlate_c16l29_1", stainlessMat, 3832.96, 3874.46, 167.0},
        {"Air_c16l29_1", airMat, 3855.71, 3859.71, 167.0},
        {"SS_c16l29_1", stainlessMat, 3858.96, 3861.46, 167.0},
        {"Air_c16l29_2", airMat, 3860.41, 3860.81, 167.0},
        {"PCB_c16l29_1", pcbMat, 3861.41, 3863.01, 167.0},
        //{"AirElec_c16l29_1", airMat, 3863.9475, 3867.4225, 167.0},
        {"PCBHex_c16l29_1", pcbMat, 3866.485, 3868.085, 167.0},
        //{"Glue_c16l29_3", glueMat, 3867.3225, 3867.3975, 167.0},

        // Cassette 17, Layer 30 (z=3867.66mm, R=169mm)
        {"Si_c17l30", siMat, 3867.51, 3867.81, 169.0},
        //{"Glue_c17l30_1", glueMat, 3867.6975, 3867.7725, 169.0},
        {"Kapton_c17l30_1", kaptonMat, 3867.785, 3867.885, 169.0},
        //{"Glue_c17l30_2", glueMat, 3867.8725, 3867.9475, 169.0},
        {"BasePC_c17l30_1", pcbMat, 3868.41, 3869.41, 169.0},
        {"CuCool_c17l30_1", cuMat, 3872.085, 3878.435, 169.0},
        {"AbsorberPlate_c17l30_1", stainlessMat, 3896.01, 3937.51, 169.0},
        {"Air_c17l30_1", airMat, 3918.76, 3922.76, 169.0},
        {"SS_c17l30_1", stainlessMat, 3922.01, 3924.51, 169.0},
        {"Air_c17l30_2", airMat, 3923.46, 3923.86, 169.0},
        {"PCB_c17l30_1", pcbMat, 3924.46, 3926.06, 169.0},
        //{"AirElec_c17l30_1", airMat, 3926.9975, 3930.4725, 169.0},
        {"PCBHex_c17l30_1", pcbMat, 3929.535, 3931.135, 169.0},
        //{"Glue_c17l30_2", glueMat, 3930.3725, 3930.4475, 169.0},

        // Cassette 18, Layer 31 (z=3930.71mm, R=167mm)
        {"Si_c18l31", siMat, 3930.56, 3930.86, 167.0},
        //{"Glue_c18l31_1", glueMat, 3930.7475, 3930.8225, 167.0},
        {"Kapton_c18l31_1", kaptonMat, 3930.835, 3930.935, 167.0},
        //{"Glue_c18l31_2", glueMat, 3930.9225, 3930.9975, 167.0},
        {"BasePC_c18l31_1", pcbMat, 3931.46, 3932.46, 167.0},
        {"CuCool_c18l31_1", cuMat, 3935.135, 3941.485, 167.0},
        {"AbsorberPlate_c18l31_1", stainlessMat, 3959.06, 4000.56, 167.0},
        {"Air_c18l31_1", airMat, 3981.81, 3985.81, 167.0},
        {"SS_c18l31_1", stainlessMat, 3985.06, 3987.56, 167.0},
        {"Air_c18l31_2", airMat, 3986.51, 3986.91, 167.0},
        {"PCB_c18l31_1", pcbMat, 3987.51, 3989.11, 167.0},
        //{"AirElec_c18l31_1", airMat, 3990.0475, 3993.5225, 167.0},
        {"PCBHex_c18l31_1", pcbMat, 3992.585, 3994.185, 167.0},
        //{"Glue_c18l31_2", glueMat, 3993.4225, 3993.4975, 167.0},

        // Cassette 19, Layer 32 (z=3993.76mm, R=178mm)
        {"Si_c19l32", siMat, 3993.61, 3993.91, 178.0},
        //{"Glue_c19l32_1", glueMat, 3993.7975, 3993.8725, 178.0},
        {"Kapton_c19l32_1", kaptonMat, 3993.885, 3993.985, 178.0},
        //{"Glue_c19l32_2", glueMat, 3993.9725, 3994.0475, 178.0},
        {"BasePC_c19l32_1", pcbMat, 3994.51, 3995.51, 178.0},
        {"CuCool_c19l32_1", cuMat, 3998.185, 4004.535, 178.0},
        {"AbsorberPlate_c19l32_1", stainlessMat, 4022.11, 4063.61, 178.0},
        {"Air_c19l32_1", airMat, 4044.86, 4048.86, 178.0},
        {"SS_c19l32_1", stainlessMat, 4048.11, 4050.61, 178.0},
        {"Air_c19l32_2", airMat, 4049.56, 4049.96, 178.0},
        {"PCB_c19l32_1", pcbMat, 4050.56, 4052.16, 178.0},
        //{"AirElec_c19l32_1", airMat, 4053.0975, 4056.5725, 178.0},
        {"PCBHex_c19l32_1", pcbMat, 4055.635, 4057.235, 178.0},
        //{"Glue_c19l32_2", glueMat, 4056.4725, 4056.5475, 178.0},

        // Cassette 20, Layer 33 (z=4056.81mm, R=206mm)
        {"Si_c20l33", siMat, 4056.66, 4056.96, 206.0},
        //{"Glue_c20l33_1", glueMat, 4056.8475, 4056.9225, 206.0},
        {"Kapton_c20l33_1", kaptonMat, 4056.935, 4057.035, 206.0},
        //{"Glue_c20l33_2", glueMat, 4057.0225, 4057.0975, 206.0},
        {"BasePC_c20l33_1", pcbMat, 4057.56, 4058.56, 206.0},
        {"CuCool_c20l33_1", cuMat, 4061.235, 4067.585, 206.0},
        {"AbsorberPlate_c20l33_1", stainlessMat, 4085.16, 4126.66, 206.0},
        {"Air_c20l33_1", airMat, 4107.91, 4111.91, 206.0},
        {"SS_c20l33_1", stainlessMat, 4111.16, 4113.66, 206.0},
        {"Air_c20l33_2", airMat, 4112.61, 4113.01, 206.0},
        {"PCB_c20l33_1", pcbMat, 4113.61, 4115.21, 206.0},
        //{"AirElec_c20l33_1", airMat, 4116.1475, 4119.6225, 206.0},
        {"PCBHex_c20l33_1", pcbMat, 4118.685, 4120.285, 206.0},
        //{"Glue_c20l33_2", glueMat, 4119.5225, 4119.5975, 206.0},

        // Cassette 21, Layer 34 (z=4119.86mm, R=215mm)
        {"Si_c21l34", siMat, 4119.71, 4120.01, 215.0},
        //{"Glue_c21l34_1", glueMat, 4119.8975, 4119.9725, 215.0},
        {"Kapton_c21l34_1", kaptonMat, 4119.985, 4120.085, 215.0},
        //{"Glue_c21l34_2", glueMat, 4120.0725, 4120.1475, 215.0},
        {"BasePC_c21l34_1", pcbMat, 4120.61, 4121.61, 215.0},
        {"CuCool_c21l34_1", cuMat, 4124.285, 4130.635, 215.0},
        {"AbsorberPlate_c21l34_1", stainlessMat, 4148.21, 4189.71, 215.0},
        {"Air_c21l34_1", airMat, 4170.96, 4174.96, 215.0},
        {"SS_c21l34_1", stainlessMat, 4174.21, 4176.71, 215.0},
        {"Air_c21l34_2", airMat, 4175.66, 4176.06, 215.0},
        {"PCB_c21l34_1", pcbMat, 4176.66, 4178.26, 215.0},
        //{"AirElec_c21l34_1", airMat, 4179.1975, 4182.6725, 215.0},
        {"PCBHex_c21l34_1", pcbMat, 4181.735, 4183.335, 215.0},
        //{"Glue_c21l34_2", glueMat, 4182.5725, 4182.6475, 215.0},

        // Cassette 22, Layer 35 (z=4182.91mm, R=215mm)
        {"Si_c22l35", siMat, 4182.76, 4183.06, 215.0},
        //{"Glue_c22l35_1", glueMat, 4182.9475, 4183.0225, 215.0},
        {"Kapton_c22l35_1", kaptonMat, 4183.035, 4183.135, 215.0},
        //{"Glue_c22l35_2", glueMat, 4183.1225, 4183.1975, 215.0},
        {"BasePC_c22l35_1", pcbMat, 4183.66, 4184.66, 215.0},
        {"CuCool_c22l35_1", cuMat, 4187.335, 4193.685, 215.0},
        {"AbsorberPlate_c22l35_1", stainlessMat, 4211.26, 4252.76, 215.0},
        {"Air_c22l35_1", airMat, 4234.01, 4238.01, 215.0},
        {"SS_c22l35_1", stainlessMat, 4237.26, 4239.76, 215.0},
        {"Air_c22l35_2", airMat, 4238.71, 4239.11, 215.0},
        {"PCB_c22l35_1", pcbMat, 4239.71, 4241.31, 215.0},
        //{"AirElec_c22l35_1", airMat, 4242.2475, 4245.7225, 215.0},
        {"PCBHex_c22l35_1", pcbMat, 4244.785, 4246.385, 215.0},
        //{"Glue_c22l35_2", glueMat, 4245.6225, 4245.6975, 215.0},

        // Cassette 23, Layer 36 (z=4245.96mm, R=234mm)
        {"Si_c23l36", siMat, 4245.81, 4246.11, 234.0},
        //{"Glue_c23l36_1", glueMat, 4245.9975, 4246.0725, 234.0},
        {"Kapton_c23l36_1", kaptonMat, 4246.085, 4246.185, 234.0},
        //{"Glue_c23l36_2", glueMat, 4246.1725, 4246.2475, 234.0},
        {"BasePC_c23l36_1", pcbMat, 4246.71, 4247.71, 234.0},
        {"CuCool_c23l36_1", cuMat, 4250.385, 4256.735, 234.0},
        {"AbsorberPlate_c23l36_1", stainlessMat, 4274.31, 4315.81, 234.0},
        {"Air_c23l36_1", airMat, 4297.06, 4301.06, 234.0},
        {"SS_c23l36_1", stainlessMat, 4300.31, 4302.81, 234.0},
        {"Air_c23l36_2", airMat, 4301.76, 4302.16, 234.0},
        {"PCB_c23l36_1", pcbMat, 4302.76, 4304.36, 234.0},
        //{"AirElec_c23l36_1", airMat, 4305.2975, 4308.7725, 234.0},
        {"PCBHex_c23l36_1", pcbMat, 4307.835, 4309.435, 234.0},
        //{"Glue_c23l36_2", glueMat, 4308.6725, 4308.7475, 234.0},

        // Cassette 24, Layer 37 (z=4309.01mm, R=245mm)
        {"Si_c24l37", siMat, 4308.86, 4309.16, 245.0},
        //{"Glue_c24l37_1", glueMat, 4309.0475, 4309.1225, 245.0},
        {"Kapton_c24l37_1", kaptonMat, 4309.135, 4309.235, 245.0},
        //{"Glue_c24l37_2", glueMat, 4309.2225, 4309.2975, 245.0},
        {"BasePC_c24l37_1", pcbMat, 4309.76, 4310.76, 245.0},
        {"CuCool_c24l37_1", cuMat, 4313.435, 4319.785, 245.0},
        {"AbsorberPlate_c24l37_1", stainlessMat, 4346.96, 4407.66, 245.0},
        {"Air_c24l37_1", airMat, 4379.31, 4383.31, 245.0},
        {"SS_c24l37_1", stainlessMat, 4382.56, 4385.06, 245.0},
        {"Air_c24l37_2", airMat, 4384.01, 4384.41, 245.0},
        {"PCB_c24l37_1", pcbMat, 4385.01, 4386.61, 245.0},
        //{"AirElec_c24l37_1", airMat, 4387.5475, 4391.0225, 245.0},
        {"PCBHex_c24l37_1", pcbMat, 4390.085, 4391.685, 245.0},
        //{"Glue_c24l37_3", glueMat, 4390.9225, 4390.9975, 245.0},

        // Cassette 25, Layer 38 (z=4391.26mm, R=256mm)
        {"Si_c25l38", siMat, 4391.11, 4391.41, 256.0},
        //{"Glue_c25l38_1", glueMat, 4391.2975, 4391.3725, 256.0},
        {"Kapton_c25l38_1", kaptonMat, 4391.385, 4391.485, 256.0},
        //{"Glue_c25l38_2", glueMat, 4391.4725, 4391.5475, 256.0},
        {"BasePC_c25l38_1", pcbMat, 4392.01, 4393.01, 256.0},
        {"CuCool_c25l38_1", cuMat, 4395.685, 4402.035, 256.0},
        {"AbsorberPlate_c25l38_1", stainlessMat, 4429.21, 4489.91, 256.0},
        {"Air_c25l38_1", airMat, 4461.56, 4465.56, 256.0},
        {"SS_c25l38_1", stainlessMat, 4464.81, 4467.31, 256.0},
        {"Air_c25l38_2", airMat, 4466.26, 4466.66, 256.0},
        {"PCB_c25l38_1", pcbMat, 4467.26, 4468.86, 256.0},
        //{"AirElec_c25l38_1", airMat, 4469.7975, 4473.2725, 256.0},
        {"PCBHex_c25l38_1", pcbMat, 4472.335, 4473.935, 256.0},
        //{"Glue_c25l38_2", glueMat, 4473.1725, 4473.2475, 256.0},

        // Cassette 26, Layer 39 (z=4473.51mm, R=256mm)
        {"Si_c26l39", siMat, 4473.36, 4473.66, 256.0},
        //{"Glue_c26l39_1", glueMat, 4473.5475, 4473.6225, 256.0},
        {"Kapton_c26l39_1", kaptonMat, 4473.635, 4473.735, 256.0},
        //{"Glue_c26l39_2", glueMat, 4473.7225, 4473.7975, 256.0},
        {"BasePC_c26l39_1", pcbMat, 4474.26, 4475.26, 256.0},
        {"CuCool_c26l39_1", cuMat, 4477.935, 4484.285, 256.0},
        {"AbsorberPlate_c26l39_1", stainlessMat, 4511.46, 4572.16, 256.0},
        {"Air_c26l39_1", airMat, 4543.81, 4547.81, 256.0},
        {"SS_c26l39_1", stainlessMat, 4547.06, 4549.56, 256.0},
        {"Air_c26l39_2", airMat, 4548.51, 4548.91, 256.0},
        {"PCB_c26l39_1", pcbMat, 4549.51, 4551.11, 256.0},
        //{"AirElec_c26l39_1", airMat, 4552.0475, 4555.5225, 256.0},
        {"PCBHex_c26l39_1", pcbMat, 4554.585, 4556.185, 256.0},
        //{"Glue_c26l39_2", glueMat, 4555.4225, 4555.4975, 256.0},

        // Cassette 27, Layer 40 (z=4555.76mm, R=256mm)
        {"Si_c27l40", siMat, 4555.61, 4555.91, 256.0},
        //{"Glue_c27l40_1", glueMat, 4555.7975, 4555.8725, 256.0},
        {"Kapton_c27l40_1", kaptonMat, 4555.885, 4555.985, 256.0},
        //{"Glue_c27l40_2", glueMat, 4555.9725, 4556.0475, 256.0},
        {"BasePC_c27l40_1", pcbMat, 4556.51, 4557.51, 256.0},
        {"CuCool_c27l40_1", cuMat, 4560.185, 4566.535, 256.0},
        {"AbsorberPlate_c27l40_1", stainlessMat, 4593.71, 4654.41, 256.0},
        {"Air_c27l40_1", airMat, 4626.06, 4630.06, 256.0},
        {"SS_c27l40_1", stainlessMat, 4629.31, 4631.81, 256.0},
        {"Air_c27l40_2", airMat, 4630.76, 4631.16, 256.0},
        {"PCB_c27l40_1", pcbMat, 4631.76, 4633.36, 256.0},
        //{"AirElec_c27l40_1", airMat, 4634.2975, 4637.7725, 256.0},
        {"PCBHex_c27l40_1", pcbMat, 4636.835, 4638.435, 256.0},
        //{"Glue_c27l40_2", glueMat, 4637.6725, 4637.7475, 256.0},

        // Cassette 28, Layer 41 (z=4638.01mm, R=256mm)
        {"Si_c28l41", siMat, 4637.86, 4638.16, 256.0},
        //{"Glue_c28l41_1", glueMat, 4638.0475, 4638.1225, 256.0},
        {"Kapton_c28l41_1", kaptonMat, 4638.135, 4638.235, 256.0},
        //{"Glue_c28l41_2", glueMat, 4638.2225, 4638.2975, 256.0},
        {"BasePC_c28l41_1", pcbMat, 4638.76, 4639.76, 256.0},
        {"CuCool_c28l41_1", cuMat, 4642.435, 4648.785, 256.0},
        {"AbsorberPlate_c28l41_1", stainlessMat, 4675.96, 4736.66, 256.0},
        {"Air_c28l41_1", airMat, 4708.31, 4712.31, 256.0},
        {"SS_c28l41_1", stainlessMat, 4711.56, 4714.06, 256.0},
        {"Air_c28l41_2", airMat, 4713.01, 4713.41, 256.0},
        {"PCB_c28l41_1", pcbMat, 4714.01, 4715.61, 256.0},
        //{"AirElec_c28l41_1", airMat, 4716.5475, 4720.0225, 256.0},
        {"PCBHex_c28l41_1", pcbMat, 4719.085, 4720.685, 256.0},
        //{"Glue_c28l41_2", glueMat, 4719.9225, 4719.9975, 256.0},

        // Cassette 29, Layer 42 (z=4720.26mm, R=256mm)
        {"Si_c29l42", siMat, 4720.11, 4720.41, 256.0},
        //{"Glue_c29l42_1", glueMat, 4720.2975, 4720.3725, 256.0},
        {"Kapton_c29l42_1", kaptonMat, 4720.385, 4720.485, 256.0},
        //{"Glue_c29l42_2", glueMat, 4720.4725, 4720.5475, 256.0},
        {"BasePC_c29l42_1", pcbMat, 4721.01, 4722.01, 256.0},
        {"CuCool_c29l42_1", cuMat, 4724.685, 4731.035, 256.0},
        {"AbsorberPlate_c29l42_1", stainlessMat, 4758.21, 4818.91, 256.0},
        {"Air_c29l42_1", airMat, 4790.56, 4794.56, 256.0},
        {"SS_c29l42_1", stainlessMat, 4793.81, 4796.31, 256.0},
        {"Air_c29l42_2", airMat, 4795.26, 4795.66, 256.0},
        {"PCB_c29l42_1", pcbMat, 4796.26, 4797.86, 256.0},
        //{"AirElec_c29l42_1", airMat, 4798.7975, 4802.2725, 256.0},
        {"PCBHex_c29l42_1", pcbMat, 4801.335, 4802.935, 256.0},
        //{"Glue_c29l42_2", glueMat, 4802.1725, 4802.2475, 256.0},

        // Cassette 30, Layer 43 (z=4802.51mm, R=256mm)
        {"Si_c30l43", siMat, 4802.36, 4802.66, 256.0},
        //{"Glue_c30l43_1", glueMat, 4802.5475, 4802.6225, 256.0},
        {"Kapton_c30l43_1", kaptonMat, 4802.635, 4802.735, 256.0},
        //{"Glue_c30l43_2", glueMat, 4802.7225, 4802.7975, 256.0},
        {"BasePC_c30l43_1", pcbMat, 4803.26, 4804.26, 256.0},
        {"CuCool_c30l43_1", cuMat, 4806.935, 4813.285, 256.0},
        {"AbsorberPlate_c30l43_1", stainlessMat, 4840.46, 4901.16, 256.0},
        {"Air_c30l43_1", airMat, 4872.81, 4876.81, 256.0},
        {"SS_c30l43_1", stainlessMat, 4876.06, 4878.56, 256.0},
        {"Air_c30l43_2", airMat, 4877.51, 4877.91, 256.0},
        {"PCB_c30l43_1", pcbMat, 4878.51, 4880.11, 256.0},
        //{"AirElec_c30l43_1", airMat, 4881.0475, 4884.5225, 256.0},
        {"PCBHex_c30l43_1", pcbMat, 4883.585, 4885.185, 256.0},
        //{"Glue_c30l43_2", glueMat, 4884.4225, 4884.4975, 256.0},

        // Cassette 31, Layer 44 (z=4884.76mm, R=256mm)
        {"Si_c31l44", siMat, 4884.61, 4884.91, 256.0},
        //{"Glue_c31l44_1", glueMat, 4884.7975, 4884.8725, 256.0},
        {"Kapton_c31l44_1", kaptonMat, 4884.885, 4884.985, 256.0},
        //{"Glue_c31l44_2", glueMat, 4884.9725, 4885.0475, 256.0},
        {"BasePC_c31l44_1", pcbMat, 4885.51, 4886.51, 256.0},
        {"CuCool_c31l44_1", cuMat, 4889.185, 4895.535, 256.0},
        {"AbsorberPlate_c31l44_1", stainlessMat, 4922.71, 4983.41, 256.0},
        {"Air_c31l44_1", airMat, 4955.06, 4959.06, 256.0},
        {"SS_c31l44_1", stainlessMat, 4958.31, 4960.81, 256.0},
        {"Air_c31l44_2", airMat, 4959.76, 4960.16, 256.0},
        {"PCB_c31l44_1", pcbMat, 4960.76, 4962.36, 256.0},
        //{"AirElec_c31l44_1", airMat, 4963.2975, 4966.7725, 256.0},
        {"PCBHex_c31l44_1", pcbMat, 4965.835, 4967.435, 256.0},
        //{"Glue_c31l44_2", glueMat, 4966.6725, 4966.7475, 256.0},

        // Cassette 32, Layer 45 (z=4967.01mm, R=256mm)
        {"Si_c32l45", siMat, 4966.86, 4967.16, 256.0},
        //{"Glue_c32l45_1", glueMat, 4967.0475, 4967.1225, 256.0},
        {"Kapton_c32l45_1", kaptonMat, 4967.135, 4967.235, 256.0},
        //{"Glue_c32l45_2", glueMat, 4967.2225, 4967.2975, 256.0},
        {"BasePC_c32l45_1", pcbMat, 4967.76, 4968.76, 256.0},
        {"CuCool_c32l45_1", cuMat, 4971.435, 4977.785, 256.0},
        {"AbsorberPlate_c32l45_1", stainlessMat, 5004.96, 5065.66, 256.0},
        {"Air_c32l45_1", airMat, 5037.31, 5041.31, 256.0},
        {"SS_c32l45_1", stainlessMat, 5040.56, 5043.06, 256.0},
        {"Air_c32l45_2", airMat, 5042.01, 5042.41, 256.0},
        {"PCB_c32l45_1", pcbMat, 5043.01, 5044.61, 256.0},
        //{"AirElec_c32l45_1", airMat, 5045.5475, 5049.0225, 256.0},
        {"PCBHex_c32l45_1", pcbMat, 5048.085, 5049.685, 256.0},
        //{"Glue_c32l45_2", glueMat, 5048.9225, 5048.9975, 256.0},

        // Cassette 33, Layer 46 (z=5049.26mm, R=256mm)
        {"Si_c33l46", siMat, 5049.11, 5049.41, 256.0},
        //{"Glue_c33l46_1", glueMat, 5049.2975, 5049.3725, 256.0},
        {"Kapton_c33l46_1", kaptonMat, 5049.385, 5049.485, 256.0},
        //{"Glue_c33l46_2", glueMat, 5049.4725, 5049.5475, 256.0},
        {"BasePC_c33l46_1", pcbMat, 5050.01, 5051.01, 256.0},
        {"CuCool_c33l46_1", cuMat, 5053.685, 5060.035, 256.0},
        {"AbsorberPlate_c33l46_1", stainlessMat, 5087.21, 5147.91, 256.0},
        {"Air_c33l46_1", airMat, 5119.56, 5123.56, 256.0},
        {"SS_c33l46_1", stainlessMat, 5122.81, 5125.31, 256.0},
        {"Air_c33l46_2", airMat, 5124.26, 5124.66, 256.0},
        {"PCB_c33l46_1", pcbMat, 5125.26, 5126.86, 256.0},
        //{"AirElec_c33l46_1", airMat, 5127.7975, 5131.2725, 256.0},
        {"PCBHex_c33l46_1", pcbMat, 5130.335, 5131.935, 256.0},
        //{"Glue_c33l46_2", glueMat, 5131.1725, 5131.2475, 256.0},

        // Cassette 34, Layer 47 (z=5131.51mm, R=245mm)
        {"Si_c34l47", siMat, 5131.36, 5131.66, 245.0},
        //{"Glue_c34l47_1", glueMat, 5131.5475, 5131.6225, 245.0},
        {"Kapton_c34l47_1", kaptonMat, 5131.635, 5131.735, 245.0},
        //{"Glue_c34l47_2", glueMat, 5131.7225, 5131.7975, 245.0},
        {"BasePC_c34l47_1", pcbMat, 5132.26, 5133.26, 245.0},
        {"CuCool_c34l47_1", cuMat, 5135.935, 5142.285, 245.0}
    };

    G4double innerRadius = 15.0*cm;
    G4SDManager* sdManager = G4SDManager::GetSDMpointer();
    MySensitiveDetector* sensDet = nullptr;
    int copyNum = 0;
    int siLayerCount = 0;

    for (const auto& layer : layers) {
        G4double zPos = (layer.zMin + layer.zMax) / 2.0 * mm;
        G4double thickness = (layer.zMax - layer.zMin) * mm;
        G4double outerR = layer.outerRadius * mm;
        
        // Skip if thickness too small
        if (thickness < 0.001*mm) continue;

        // Ensure outer radius is greater than inner radius
        if (outerR <= innerRadius) {
            // Give a small radial thickness (1mm) for materials at the same nominal radius
            outerR = innerRadius + 1.0*mm;
        }

        G4String solidName = layer.name + "_solid";
        G4Tubs* solid = new G4Tubs(solidName, innerRadius, outerR, thickness/2.0, 0.0, 2.0*CLHEP::pi);
        
        G4String logicName = layer.name + "_logic";
        G4LogicalVolume* logic = new G4LogicalVolume(solid, layer.material, logicName);

        // Optional: Set visualization attributes
        /*
        if (layer.name.find("Si") != std::string::npos) {
            G4VisAttributes* siVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0)); // cyan
            siVis->SetForceSolid(true);
            logic->SetVisAttributes(siVis);
        }
        */

        G4String physName = layer.name + "_phys";
        new G4PVPlacement(0, G4ThreeVector(0, 0, zPos), logic, physName, logicWorld, false, copyNum, false);
        
        // Attach sensitive detector only to silicon layers
        if (layer.name.find("Si_") != std::string::npos) {
            if (!sensDet) {
                sensDet = new MySensitiveDetector("SensitiveDetector");
                sdManager->AddNewDetector(sensDet);
            }
            logic->SetSensitiveDetector(sensDet);
            siLayerCount++;
        }
        
        copyNum++;
    }

    G4cout << "Total material layers created: " << copyNum << G4endl;
    G4cout << "Silicon layers with sensitive detector: " << siLayerCount << G4endl;

    return physWorld;
}
