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
    // World volume
    // ---------------------
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_Galactic");
    G4double worldSizeXY = 300.0*cm;
    G4double worldSizeZ = 600.0*cm;
    
    G4Box* solidWorld = new G4Box("solidWorld", worldSizeXY, worldSizeXY, worldSizeZ);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

    G4VPhysicalVolume* physWorld = new G4PVPlacement(
        0,
        G4ThreeVector(0,0,0),
        logicWorld,
        "physWorld",
        0,
        false,
        0,
        true
    );

    // ---------------------
    // Materials
    // ---------------------
    G4Material* siMat = nist->FindOrBuildMaterial("G4_Si");
    G4Material* cuMat = nist->FindOrBuildMaterial("G4_Cu");
    G4Material* pbMat = nist->FindOrBuildMaterial("G4_Pb");
    G4Material* airMat = nist->FindOrBuildMaterial("G4_AIR");
    
    G4Material* stainlessMat = new G4Material("StainlessSteel", 8.02*g/cm3, 2);
    stainlessMat->AddElement(nist->FindOrBuildElement("Fe"), 0.70);
    stainlessMat->AddElement(nist->FindOrBuildElement("Ni"), 0.30);
    
    G4Material* kaptonMat = new G4Material("Kapton", 1.11*g/cm3, 4);
    kaptonMat->AddElement(nist->FindOrBuildElement("C"), 5);
    kaptonMat->AddElement(nist->FindOrBuildElement("H"), 4);
    kaptonMat->AddElement(nist->FindOrBuildElement("O"), 2);
    kaptonMat->AddElement(nist->FindOrBuildElement("N"), 1);
    
    G4Material* pcbMat = new G4Material("PCB", 1.7*g/cm3, 3);
    pcbMat->AddElement(nist->FindOrBuildElement("C"), 0.50);
    pcbMat->AddElement(nist->FindOrBuildElement("H"), 0.30);
    pcbMat->AddElement(nist->FindOrBuildElement("O"), 0.20);
    
    G4Material* glueMat = new G4Material("Glue", 1.3*g/cm3, 3);
    glueMat->AddElement(nist->FindOrBuildElement("C"), 0.60);
    glueMat->AddElement(nist->FindOrBuildElement("H"), 0.30);
    glueMat->AddElement(nist->FindOrBuildElement("O"), 0.10);

    // ---------------------
    // Silicon layers 
    // ---------------------
    G4double innerRadius = 15.0*cm;

    std::vector<MaterialLayer> siliconLayers = {
        {"Si_l1",   siMat, 3221.4, 3221.7, 150.0, 1},
        {"Si_l2",   siMat, 3231.34, 3231.64, 142.0, 2},
        {"Si_l3",   siMat, 3251.97, 3252.27, 150.0, 3},
        {"Si_l4",   siMat, 3261.91, 3262.21, 142.0, 4},
        {"Si_l5",   siMat, 3282.54, 3282.84, 150.0, 5},
        {"Si_l6",   siMat, 3292.48, 3292.78, 150.0, 6},
        {"Si_l7",   siMat, 3313.11, 3313.41, 150.0, 7},
        {"Si_l8",   siMat, 3323.05, 3323.35, 142.0, 8},
        {"Si_l9",   siMat, 3343.68, 3343.98, 150.0, 9},
        {"Si_l10",  siMat, 3353.62, 3353.92, 150.0, 10},
        {"Si_l11",  siMat, 3374.25, 3374.55, 150.0, 11},
        {"Si_l12",  siMat, 3384.19, 3384.49, 150.0, 12},
        {"Si_l13",  siMat, 3404.82, 3405.12, 150.0, 13},
        {"Si_l14",  siMat, 3414.76, 3415.06, 150.0, 14},
        {"Si_l15",  siMat, 3435.39, 3435.69, 150.0, 15},
        {"Si_l16",  siMat, 3445.33, 3445.63, 150.0, 16},
        {"Si_l17",  siMat, 3465.96, 3466.26, 158.0, 17},
        {"Si_l18",  siMat, 3475.9, 3476.2, 158.0, 18},
        {"Si_l19", siMat, 3499.78, 3500.08, 158.0, 19},
        {"Si_l20", siMat, 3509.72, 3510.02, 158.0, 20},
        {"Si_l21", siMat, 3533.6, 3533.9, 158.0, 21},
        {"Si_l22", siMat, 3543.54, 3543.84, 158.0, 22},
        {"Si_l23", siMat, 3567.42, 3567.72, 158.0, 23},
        {"Si_l24", siMat, 3577.36, 3577.66, 158.0, 24},
        {"Si_l25", siMat, 3601.24, 3601.54, 158.0, 25},
        {"Si_l26", siMat, 3611.18, 3611.48, 158.0, 26},
        {"Si_l27", siMat, 3678.36, 3678.66, 158.0, 27},
        {"Si_l28", siMat, 3741.41, 3741.71, 164.0, 28},
        {"Si_l29", siMat, 3804.46, 3804.76, 167.0, 29},
        {"Si_l30", siMat, 3867.51, 3867.81, 169.0, 30},
        {"Si_l31", siMat, 3930.56, 3930.86, 167.0, 31},
        {"Si_l32", siMat, 3993.61, 3993.91, 178.0, 32},
        {"Si_l33", siMat, 4056.66, 4056.96, 206.0, 33},
        {"Si_l34", siMat, 4119.71, 4120.01, 215.0, 34},
        {"Si_l35", siMat, 4182.76, 4183.06, 215.0, 35},
        {"Si_l36", siMat, 4245.81, 4246.11, 234.0, 36},
        {"Si_l37", siMat, 4308.86, 4309.16, 245.0, 37},
        {"Si_l38", siMat, 4391.11, 4391.41, 256.0, 38},
        {"Si_l39", siMat, 4473.36, 4473.66, 256.0, 39},
        {"Si_l40", siMat, 4555.61, 4555.91, 256.0, 40},
        {"Si_l41", siMat, 4637.86, 4638.16, 256.0, 41},
        {"Si_l42", siMat, 4720.11, 4720.41, 256.0, 42},
        {"Si_l43", siMat, 4802.36, 4802.66, 256.0, 43},
        {"Si_l44", siMat, 4884.61, 4884.91, 256.0, 44},
        {"Si_l45", siMat, 4966.86, 4967.16, 256.0, 45},
        {"Si_l46", siMat, 5049.11, 5049.41, 256.0, 46},
        {"Si_l47", siMat, 5131.36, 5131.66, 245.0, 47}
    };


    // ---------------------
    // Non-silicon layers 
    // ---------------------
    std::vector<MaterialLayer> nonSiliconLayers = {
   	{"Cu_l0_pre1", cuMat, 3210.5, 3210.6, 150, -1},
    	{"StainlessSteel_l0_pre2", stainlessMat, 3210.6, 3210.9, 150, -1},
    	//{"glue_l0_pre3", glueMat, 3210.9, 3210.95, 150, -1},
    	{"Lead_l0_pre4", pbMat, 3210.95, 3213.72, 150, -1},
    	//{"glue_l0_pre5", glueMat, 3213.72, 3213.77, 150, -1},
    	{"StainlessSteel_l0_pre6", stainlessMat, 3213.77, 3214.07, 150, -1},
    	{"Cu_l0_pre7", cuMat, 3214.07, 3214.17, 150, -1},
    	//{"Air_l0_pre8", airMat, 3214.17, 3214.395, 150, -1},
    	{"PCB_l0_pre9", pcbMat, 3214.395, 3215.995, 150, -1},
    	//{"Airelectronics_l0_pre10", airMat, 3215.995, 3219.725, 150, -1},
    	{"PCBHexaboard_l0_pre11", pcbMat, 3219.725, 3221.325, 150, -1},
    	//{"glue_l0_pre12", glueMat, 3221.325, 3221.4, 150, -1},

    	//{"glue_l1_post1", glueMat, 3221.7, 3221.765, 150, -1},
    	{"kapton_l1_pre1", kaptonMat, 3221.765, 3222.03, 150, -1},
    	//{"glue_l1_pre2", glueMat, 3222.03, 3222.095, 150, -1},
    	{"CuW_l1_pre3", cuMat, 3222.095, 3223.495, 150, -1},
    	{"CoolingplateCu_l1_pre4", cuMat, 3223.495, 3229.545, 150, -1},
    	{"CuW_l1_pre5", cuMat, 3229.545, 3230.945, 150, -1},
    	//{"glue_l1_pre6", glueMat, 3230.945, 3231.01, 150, -1},
    	{"kapton_l1_pre7", kaptonMat, 3231.01, 3231.275, 150, -1},
    	//{"glue_l1_pre8", glueMat, 3231.275, 3231.34, 150, -1},

    	//{"glue_l2_post1", glueMat, 3231.64, 3231.715, 142, -1},
    	{"PCBHexaboard_l2_pre1", pcbMat, 3231.715, 3233.315, 142, -1},
    	//{"Airelectronics_l2_pre2", airMat, 3233.315, 3237.045, 142, -1},
    	{"PCB_l2_pre3", pcbMat, 3237.045, 3238.645, 142, -1},
    	//{"Air_l2_pre4", airMat, 3238.645, 3238.87, 142, -1},
    	{"Cu_l2_pre5", cuMat, 3238.87, 3238.97, 142, -1},
    	{"StainlessSteel_l2_pre6", stainlessMat, 3238.97, 3239.27, 142, -1},
    	//{"glue_l2_pre7", glueMat, 3239.27, 3239.32, 142, -1},
    	{"Lead_l2_pre8", pbMat, 3239.32, 3244.29, 142, -1},
    	//{"glue_l2_pre9", glueMat, 3244.29, 3244.34, 142, -1},
    	{"StainlessSteel_l2_pre10", stainlessMat, 3244.34, 3244.64, 142, -1},
    	{"Cu_l2_pre11", cuMat, 3244.64, 3244.74, 142, -1},
    	//{"Air_l2_pre12", airMat, 3244.74, 3244.965, 142, -1},
    	{"PCB_l2_pre13", pcbMat, 3244.965, 3246.565, 142, -1},
    	//{"Airelectronics_l2_pre14", airMat, 3246.565, 3250.295, 142, -1},
    	{"PCBHexaboard_l2_pre15", pcbMat, 3250.295, 3251.895, 142, -1},
    	//{"glue_l2_pre16", glueMat, 3251.895, 3251.97, 142, -1},
	
    	//{"glue_l3_post1", glueMat, 3252.27, 3252.335, 150, -1},
    	{"kapton_l3_pre1", kaptonMat, 3252.335, 3252.6, 150, -1},
    	//{"glue_l3_pre2", glueMat, 3252.6, 3252.665, 150, -1},
    	{"CuW_l3_pre3", cuMat, 3252.665, 3254.065, 150, -1},
    	{"CoolingplateCu_l3_pre4", cuMat, 3254.065, 3260.115, 150, -1},
    	{"CuW_l3_pre5", cuMat, 3260.115, 3261.515, 150, -1},
    	//{"glue_l3_pre6", glueMat, 3261.515, 3261.58, 150, -1},
    	{"kapton_l3_pre7", kaptonMat, 3261.58, 3261.845, 150, -1},
    	//{"glue_l3_pre8", glueMat, 3261.845, 3261.91, 150, -1},
	
    	//{"glue_l4_post1", glueMat, 3262.21, 3262.285, 142, -1},
    	{"PCBHexaboard_l4_pre1", pcbMat, 3262.285, 3263.885, 142, -1},
    	//{"Airelectronics_l4_pre2", airMat, 3263.885, 3267.615, 142, -1},
    	{"PCB_l4_pre3", pcbMat, 3267.615, 3269.215, 142, -1},
    	//{"Air_l4_pre4", airMat, 3269.215, 3269.44, 142, -1},
    	{"Cu_l4_pre5", cuMat, 3269.44, 3269.54, 142, -1},
    	{"StainlessSteel_l4_pre6", stainlessMat, 3269.54, 3269.84, 142, -1},
    	//{"glue_l4_pre7", glueMat, 3269.84, 3269.89, 142, -1},
    	{"Lead_l4_pre8", pbMat, 3269.89, 3274.86, 142, -1},
    	//{"glue_l4_pre9", glueMat, 3274.86, 3274.91, 142, -1},
    	{"StainlessSteel_l4_pre10", stainlessMat, 3274.91, 3275.21, 142, -1},
    	{"Cu_l4_pre11", cuMat, 3275.21, 3275.31, 142, -1},
    	//{"Air_l4_pre12", airMat, 3275.31, 3275.535, 142, -1},
    	{"PCB_l4_pre13", pcbMat, 3275.535, 3277.135, 142, -1},
    	//{"Airelectronics_l4_pre14", airMat, 3277.135, 3280.865, 142, -1},
    	{"PCBHexaboard_l4_pre15", pcbMat, 3280.865, 3282.465, 142, -1},
    	//{"glue_l4_pre16", glueMat, 3282.465, 3282.54, 142, -1},

    	//{"glue_l5_post1", glueMat, 3282.84, 3282.905, 150, -1},
    	{"kapton_l5_pre1", kaptonMat, 3282.905, 3283.17, 150, -1},
    	//{"glue_l5_pre2", glueMat, 3283.17, 3283.235, 150, -1},
    	{"CuW_l5_pre3", cuMat, 3283.235, 3284.635, 150, -1},
    	{"CoolingplateCu_l5_pre4", cuMat, 3284.635, 3290.685, 150, -1},
    	{"CuW_l5_pre5", cuMat, 3290.685, 3292.085, 150, -1},
    	//{"glue_l5_pre6", glueMat, 3292.085, 3292.15, 150, -1},
    	{"kapton_l5_pre7", kaptonMat, 3292.15, 3292.415, 150, -1},
    	//{"glue_l5_pre8", glueMat, 3292.415, 3292.48, 150, -1},

    	//{"glue_l6_post1", glueMat, 3292.78, 3292.855, 150, -1},
    	{"PCBHexaboard_l6_pre1", pcbMat, 3292.855, 3294.455, 150, -1},
    	//{"Airelectronics_l6_pre2", airMat, 3294.455, 3298.185, 150, -1},
    	{"PCB_l6_pre3", pcbMat, 3298.185, 3299.785, 150, -1},
    	//{"Air_l6_pre4", airMat, 3299.785, 3300.01, 150, -1},
    	{"Cu_l6_pre5", cuMat, 3300.01, 3300.11, 150, -1},
    	{"StainlessSteel_l6_pre6", stainlessMat, 3300.11, 3300.41, 150, -1},
    	//{"glue_l6_pre7", glueMat, 3300.41, 3300.46, 150, -1},
    	{"Lead_l6_pre8", pbMat, 3300.46, 3305.43, 150, -1},
    	//{"glue_l6_pre9", glueMat, 3305.43, 3305.48, 150, -1},
    	{"StainlessSteel_l6_pre10", stainlessMat, 3305.48, 3305.78, 150, -1},
    	{"Cu_l6_pre11", cuMat, 3305.78, 3305.88, 150, -1},
    	//{"Air_l6_pre12", airMat, 3305.88, 3306.105, 150, -1},
    	{"PCB_l6_pre13", pcbMat, 3306.105, 3307.705, 150, -1},
    	//{"Airelectronics_l6_pre14", airMat, 3307.705, 3311.435, 150, -1},
    	{"PCBHexaboard_l6_pre15", pcbMat, 3311.435, 3313.035, 150, -1},
    	//{"glue_l6_pre16", glueMat, 3313.035, 3313.11, 150, -1},

    	//{"glue_l7_post1", glueMat, 3313.41, 3313.475, 150, -1},
    	{"kapton_l7_pre1", kaptonMat, 3313.475, 3313.74, 150, -1},
    	//{"glue_l7_pre2", glueMat, 3313.74, 3313.805, 150, -1},
    	{"CuW_l7_pre3", cuMat, 3313.805, 3315.205, 150, -1},
    	{"CoolingplateCu_l7_pre4", cuMat, 3315.205, 3321.255, 150, -1},
    	{"CuW_l7_pre5", cuMat, 3321.255, 3322.655, 150, -1},
    	//{"glue_l7_pre6", glueMat, 3322.655, 3322.72, 150, -1},
    	{"kapton_l7_pre7", kaptonMat, 3322.72, 3322.985, 150, -1},
    	//{"glue_l7_pre8", glueMat, 3322.985, 3323.05, 150, -1},

    	//{"glue_l8_post1", glueMat, 3323.35, 3323.425, 142, -1},
    	{"PCBHexaboard_l8_pre1", pcbMat, 3323.425, 3325.025, 142, -1},
    	//{"Airelectronics_l8_pre2", airMat, 3325.025, 3328.755, 142, -1},
    	{"PCB_l8_pre3", pcbMat, 3328.755, 3330.355, 142, -1},
    	//{"Air_l8_pre4", airMat, 3330.355, 3330.58, 142, -1},
    	{"Cu_l8_pre5", cuMat, 3330.58, 3330.68, 142, -1},
    	{"StainlessSteel_l8_pre6", stainlessMat, 3330.68, 3330.98, 142, -1},
    	//{"glue_l8_pre7", glueMat, 3330.98, 3331.03, 142, -1},
    	{"Lead_l8_pre8", pbMat, 3331.03, 3336.0, 142, -1},
    	//{"glue_l8_pre9", glueMat, 3336.0, 3336.05, 142, -1},
    	{"StainlessSteel_l8_pre10", stainlessMat, 3336.05, 3336.35, 142, -1},
    	{"Cu_l8_pre11", cuMat, 3336.35, 3336.45, 142, -1},
    	//{"Air_l8_pre12", airMat, 3336.45, 3336.675, 142, -1},
    	{"PCB_l8_pre13", pcbMat, 3336.675, 3338.275, 142, -1},
    	//{"Airelectronics_l8_pre14", airMat, 3338.275, 3342.005, 142, -1},
    	{"PCBHexaboard_l8_pre15", pcbMat, 3342.005, 3343.605, 142, -1},
    	//{"glue_l8_pre16", glueMat, 3343.605, 3343.68, 142, -1},

    	//{"glue_l9_post1", glueMat, 3343.98, 3344.045, 142, -1},
    	{"kapton_l9_pre1", kaptonMat, 3344.045, 3344.31, 142, -1},
   	//{"glue_l9_pre2", glueMat, 3344.31, 3344.375, 142, -1},
    	{"CuW_l9_pre3", cuMat, 3344.375, 3345.775, 142, -1},
    	{"CoolingplateCu_l9_pre4", cuMat, 3345.775, 3351.825, 142, -1},
    	{"CuW_l9_pre5", cuMat, 3351.825, 3353.225, 142, -1},
    	//{"glue_l9_pre6", glueMat, 3353.225, 3353.29, 142, -1},
    	{"kapton_l9_pre7", kaptonMat, 3353.29, 3353.555, 142, -1},
    	//{"glue_l9_pre8", glueMat, 3353.555, 3353.62, 142, -1},

    	//{"glue_l10_post1", glueMat, 3353.92, 3353.995, 142, -1},
    	{"PCBHexaboard_l10_pre1", pcbMat, 3353.995, 3355.595, 142, -1},
    	//{"Airelectronics_l10_pre2", airMat, 3355.595, 3359.325, 142, -1},
    	{"PCB_l10_pre3", pcbMat, 3359.325, 3360.925, 142, -1},
    	//{"Air_l10_pre4", airMat, 3360.925, 3361.15, 142, -1},
    	{"Cu_l10_pre5", cuMat, 3361.15, 3361.25, 142, -1},
    	{"StainlessSteel_l10_pre6", stainlessMat, 3361.25, 3361.55, 142, -1},
    	//{"glue_l10_pre7", glueMat, 3361.55, 3361.6, 142, -1},
    	{"Lead_l10_pre8", pbMat, 3361.6, 3366.57, 142, -1},
    	//{"glue_l10_pre9", glueMat, 3366.57, 3366.62, 142, -1},
    	{"StainlessSteel_l10_pre10", stainlessMat, 3366.62, 3366.92, 142, -1},
    	{"Cu_l10_pre11", cuMat, 3366.92, 3367.02, 142, -1},
    	//{"Air_l10_pre12", airMat, 3367.02, 3367.245, 142, -1},
    	{"PCB_l10_pre13", pcbMat, 3367.245, 3368.845, 142, -1},
    	//{"Airelectronics_l10_pre14", airMat, 3368.845, 3372.575, 142, -1},
    	{"PCBHexaboard_l10_pre15", pcbMat, 3372.575, 3374.175, 142, -1},
    	//{"glue_l10_pre16", glueMat, 3374.175, 3374.25, 142, -1},

    	//{"glue_l11_post1", glueMat, 3374.55, 3374.615, 150, -1},
    	{"kapton_l11_pre1", kaptonMat, 3374.615, 3374.88, 150, -1},
    	//{"glue_l11_pre2", glueMat, 3374.88, 3374.945, 150, -1},
    	{"CuW_l11_pre3", cuMat, 3374.945, 3376.345, 150, -1},
    	{"CoolingplateCu_l11_pre4", cuMat, 3376.345, 3382.395, 150, -1},
    	{"CuW_l11_pre5", cuMat, 3382.395, 3383.795, 150, -1},
    	//{"glue_l11_pre6", glueMat, 3383.795, 3383.86, 150, -1},
    	{"kapton_l11_pre7", kaptonMat, 3383.86, 3384.125, 150, -1},
    	//{"glue_l11_pre8", glueMat, 3384.125, 3384.19, 150, -1},

    	//{"glue_l12_post1", glueMat, 3384.49, 3384.565, 150, -1},
    	{"PCBHexaboard_l12_pre1", pcbMat, 3384.565, 3386.165, 150, -1},
    	//{"Airelectronics_l12_pre2", airMat, 3386.165, 3389.895, 150, -1},
    	{"PCB_l12_pre3", pcbMat, 3389.895, 3391.495, 150, -1},
    	//{"Air_l12_pre4", airMat, 3391.495, 3391.72, 150, -1},
    	{"Cu_l12_pre5", cuMat, 3391.72, 3391.82, 150, -1},
    	{"StainlessSteel_l12_pre6", stainlessMat, 3391.82, 3392.12, 150, -1},
    	//{"glue_l12_pre7", glueMat, 3392.12, 3392.17, 150, -1},
    	{"Lead_l12_pre8", pbMat, 3392.17, 3397.14, 150, -1},
    	//{"glue_l12_pre9", glueMat, 3397.14, 3397.19, 150, -1},
    	{"StainlessSteel_l12_pre10", stainlessMat, 3397.19, 3397.49, 150, -1},
    	{"Cu_l12_pre11", cuMat, 3397.49, 3397.59, 150, -1},
    	//{"Air_l12_pre12", airMat, 3397.59, 3397.815, 150, -1},
    	{"PCB_l12_pre13", pcbMat, 3397.815, 3399.415, 150, -1},
    	//{"Airelectronics_l12_pre14", airMat, 3399.415, 3403.145, 150, -1},
    	{"PCBHexaboard_l12_pre15", pcbMat, 3403.145, 3404.745, 150, -1},
    	//{"glue_l12_pre16", glueMat, 3404.745, 3404.82, 150, -1},

    	//{"glue_l13_post1", glueMat, 3405.12, 3405.185, 150, -1},
    	{"kapton_l13_pre1", kaptonMat, 3405.185, 3405.45, 150, -1},
    	//{"glue_l13_pre2", glueMat, 3405.45, 3405.515, 150, -1},
    	{"CuW_l13_pre3", cuMat, 3405.515, 3406.915, 150, -1},
    	{"CoolingplateCu_l13_pre4", cuMat, 3406.915, 3412.965, 150, -1},
    	{"CuW_l13_pre5", cuMat, 3412.965, 3414.365, 150, -1},
    	//{"glue_l13_pre6", glueMat, 3414.365, 3414.43, 150, -1},
    	{"kapton_l13_pre7", kaptonMat, 3414.43, 3414.695, 150, -1},
    	//{"glue_l13_pre8", glueMat, 3414.695, 3414.76, 150, -1},

    	//{"glue_l14_post1", glueMat, 3415.06, 3415.135, 150, -1},
    	{"PCBHexaboard_l14_pre1", pcbMat, 3415.135, 3416.735, 150, -1},
    	//{"Airelectronics_l14_pre2", airMat, 3416.735, 3420.465, 150, -1},
    	{"PCB_l14_pre3", pcbMat, 3420.465, 3422.065, 150, -1},
    	//{"Air_l14_pre4", airMat, 3422.065, 3422.29, 150, -1},
    	{"Cu_l14_pre5", cuMat, 3422.29, 3422.39, 150, -1},
    	{"StainlessSteel_l14_pre6", stainlessMat, 3422.39, 3422.69, 150, -1},
    	//{"glue_l14_pre7", glueMat, 3422.69, 3422.74, 150, -1},
    	{"Lead_l14_pre8", pbMat, 3422.74, 3427.71, 150, -1},
    	//{"glue_l14_pre9", glueMat, 3427.71, 3427.76, 150, -1},
    	{"StainlessSteel_l14_pre10", stainlessMat, 3427.76, 3428.06, 150, -1},
    	{"Cu_l14_pre11", cuMat, 3428.06, 3428.16, 150, -1},
    	//{"Air_l14_pre12", airMat, 3428.16, 3428.385, 150, -1},
    	{"PCB_l14_pre13", pcbMat, 3428.385, 3429.985, 150, -1},
    	//{"Airelectronics_l14_pre14", airMat, 3429.985, 3433.715, 150, -1},
    	{"PCBHexaboard_l14_pre15", pcbMat, 3433.715, 3435.315, 150, -1},
    	//{"glue_l14_pre16", glueMat, 3435.315, 3435.39, 150, -1},

    	//{"glue_l15_post1", glueMat, 3435.69, 3435.755, 150, -1},
    	{"kapton_l15_pre1", kaptonMat, 3435.755, 3436.02, 150, -1},
    	//{"glue_l15_pre2", glueMat, 3436.02, 3436.085, 150, -1},
    	{"CuW_l15_pre3", cuMat, 3436.085, 3437.485, 150, -1},
    	{"CoolingplateCu_l15_pre4", cuMat, 3437.485, 3443.535, 150, -1},
    	{"CuW_l15_pre5", cuMat, 3443.535, 3444.935, 150, -1},
    	//{"glue_l15_pre6", glueMat, 3444.935, 3445.0, 150, -1},
    	{"kapton_l15_pre7", kaptonMat, 3445.0, 3445.265, 150, -1},
    	//{"glue_l15_pre8", glueMat, 3445.265, 3445.33, 150, -1},

    	//{"glue_l16_post1", glueMat, 3445.63, 3445.705, 150, -1},
    	{"PCBHexaboard_l16_pre1", pcbMat, 3445.705, 3447.305, 150, -1},
    	//{"Airelectronics_l16_pre2", airMat, 3447.305, 3451.035, 150, -1},
    	{"PCB_l16_pre3", pcbMat, 3451.035, 3452.635, 150, -1},
    	//{"Air_l16_pre4", airMat, 3452.635, 3452.86, 150, -1},
    	{"Cu_l16_pre5", cuMat, 3452.86, 3452.96, 150, -1},
    	{"StainlessSteel_l16_pre6", stainlessMat, 3452.96, 3453.26, 150, -1},
    	//{"glue_l16_pre7", glueMat, 3453.26, 3453.31, 150, -1},
    	{"Lead_l16_pre8", pbMat, 3453.31, 3458.28, 150, -1},
    	//{"glue_l16_pre9", glueMat, 3458.28, 3458.33, 150, -1},
    	{"StainlessSteel_l16_pre10", stainlessMat, 3458.33, 3458.63, 150, -1},
    	{"Cu_l16_pre11", cuMat, 3458.63, 3458.73, 150, -1},
    	//{"Air_l16_pre12", airMat, 3458.73, 3458.955, 150, -1},
    	{"PCB_l16_pre13", pcbMat, 3458.955, 3460.555, 150, -1},
    	//{"Airelectronics_l16_pre14", airMat, 3460.555, 3464.285, 150, -1},
    	{"PCBHexaboard_l16_pre15", pcbMat, 3464.285, 3465.885, 150, -1},
    	//{"glue_l16_pre16", glueMat, 3465.885, 3465.96, 150, -1},

    	//{"glue_l17_post1", glueMat, 3466.26, 3466.325, 150, -1},
    	{"kapton_l17_pre1", kaptonMat, 3466.325, 3466.59, 150, -1},
    	//{"glue_l17_pre2", glueMat, 3466.59, 3466.655, 150, -1},
    	{"CuW_l17_pre3", cuMat, 3466.655, 3468.055, 150, -1},
    	{"CoolingplateCu_l17_pre4", cuMat, 3468.055, 3474.105, 150, -1},
    	{"CuW_l17_pre5", cuMat, 3474.105, 3475.505, 150, -1},
    	//{"glue_l17_pre6", glueMat, 3475.505, 3475.57, 150, -1},
    	{"kapton_l17_pre7", kaptonMat, 3475.57, 3475.835, 150, -1},
    	//{"glue_l17_pre8", glueMat, 3475.835, 3475.9, 150, -1},

    	//{"glue_l18_post1", glueMat, 3476.2, 3476.275, 158, -1},
    	{"PCBHexaboard_l18_pre1", pcbMat, 3476.275, 3477.875, 158, -1},
    	//{"Airelectronics_l18_pre2", airMat, 3477.875, 3481.605, 158, -1},
    	{"PCB_l18_pre3", pcbMat, 3481.605, 3483.205, 158, -1},
    	//{"Air_l18_pre4", airMat, 3483.205, 3483.43, 158, -1},
    	{"Cu_l18_pre5", cuMat, 3483.43, 3483.53, 158, -1},
    	{"StainlessSteel_l18_pre6", stainlessMat, 3483.53, 3483.83, 158, -1},
    	//{"glue_l18_pre7", glueMat, 3483.83, 3483.88, 158, -1},
    	{"Lead_l18_pre8", pbMat, 3483.88, 3492.1, 158, -1},
    	//{"glue_l18_pre9", glueMat, 3492.1, 3492.15, 158, -1},
    	{"StainlessSteel_l18_pre10", stainlessMat, 3492.15, 3492.45, 158, -1},
    	{"Cu_l18_pre11", cuMat, 3492.45, 3492.55, 158, -1},
    	//{"Air_l18_pre12", airMat, 3492.55, 3492.775, 158, -1},
    	{"PCB_l18_pre13", pcbMat, 3492.775, 3494.375, 158, -1},
    	//{"Airelectronics_l18_pre14", airMat, 3494.375, 3498.105, 158, -1},
    	{"PCBHexaboard_l18_pre15", pcbMat, 3498.105, 3499.705, 158, -1},
    	//{"glue_l18_pre16", glueMat, 3499.705, 3499.78, 158, -1},

    	//{"glue_l19_post1", glueMat, 3500.08, 3500.145, 158, -1},
    	{"kapton_l19_pre1", kaptonMat, 3500.145, 3500.41, 158, -1},
    	//{"glue_l19_pre2", glueMat, 3500.41, 3500.475, 158, -1},
    	{"CuW_l19_pre3", cuMat, 3500.475, 3501.875, 158, -1},
    	{"CoolingplateCu_l19_pre4", cuMat, 3501.875, 3507.925, 158, -1},
    	{"CuW_l19_pre5", cuMat, 3507.925, 3509.325, 158, -1},
    	//{"glue_l19_pre6", glueMat, 3509.325, 3509.39, 158, -1},
    	{"kapton_l19_pre7", kaptonMat, 3509.39, 3509.655, 158, -1},
    	//{"glue_l19_pre8", glueMat, 3509.655, 3509.72, 158, -1},

    	//{"glue_l20_post1", glueMat, 3510.02, 3510.095, 150, -1},
    	{"PCBHexaboard_l20_pre1", pcbMat, 3510.095, 3511.695, 150, -1},
    	//{"Airelectronics_l20_pre2", airMat, 3511.695, 3515.425, 150, -1},
    	{"PCB_l20_pre3", pcbMat, 3515.425, 3517.025, 150, -1},
    	//{"Air_l20_pre4", airMat, 3517.025, 3517.25, 150, -1},
    	{"Cu_l20_pre5", cuMat, 3517.25, 3517.35, 150, -1},
    	{"StainlessSteel_l20_pre6", stainlessMat, 3517.35, 3517.65, 150, -1},
    	//{"glue_l20_pre7", glueMat, 3517.65, 3517.7, 150, -1},
    	{"Lead_l20_pre8", pbMat, 3517.7, 3525.92, 150, -1},
    	//{"glue_l20_pre9", glueMat, 3525.92, 3525.97, 150, -1},
    	{"StainlessSteel_l20_pre10", stainlessMat, 3525.97, 3526.27, 150, -1},
    	{"Cu_l20_pre11", cuMat, 3526.27, 3526.37, 150, -1},
    	//{"Air_l20_pre12", airMat, 3526.37, 3526.595, 150, -1},
    	{"PCB_l20_pre13", pcbMat, 3526.595, 3528.195, 150, -1},
    	//{"Airelectronics_l20_pre14", airMat, 3528.195, 3531.925, 150, -1},
    	{"PCBHexaboard_l20_pre15", pcbMat, 3531.925, 3533.525, 150, -1},
    	//{"glue_l20_pre16", glueMat, 3533.525, 3533.6, 150, -1},

    	//{"glue_l21_post1", glueMat, 3533.9, 3533.965, 158, -1},
    	{"kapton_l21_pre1", kaptonMat, 3533.965, 3534.23, 158, -1},
    	//{"glue_l21_pre2", glueMat, 3534.23, 3534.295, 158, -1},
    	{"CuW_l21_pre3", cuMat, 3534.295, 3535.695, 158, -1},
    	{"CoolingplateCu_l21_pre4", cuMat, 3535.695, 3541.745, 158, -1},
    	{"CuW_l21_pre5", cuMat, 3541.745, 3543.145, 158, -1},
    	//{"glue_l21_pre6", glueMat, 3543.145, 3543.21, 158, -1},
    	{"kapton_l21_pre7", kaptonMat, 3543.21, 3543.475, 158, -1},
    	//{"glue_l21_pre8", glueMat, 3543.475, 3543.54, 158, -1},

    	//{"glue_l22_post1", glueMat, 3543.84, 3543.915, 150, -1},
    	{"PCBHexaboard_l22_pre1", pcbMat, 3543.915, 3545.515, 150, -1},
    	//{"Airelectronics_l22_pre2", airMat, 3545.515, 3549.245, 150, -1},
    	{"PCB_l22_pre3", pcbMat, 3549.245, 3550.845, 150, -1},
    	//{"Air_l22_pre4", airMat, 3550.845, 3551.07, 150, -1},
    	{"Cu_l22_pre5", cuMat, 3551.07, 3551.17, 150, -1},
    	{"StainlessSteel_l22_pre6", stainlessMat, 3551.17, 3551.47, 150, -1},
    	//{"glue_l22_pre7", glueMat, 3551.47, 3551.52, 150, -1},
    	{"Lead_l22_pre8", pbMat, 3551.52, 3559.74, 150, -1},
    	//{"glue_l22_pre9", glueMat, 3559.74, 3559.79, 150, -1},
    	{"StainlessSteel_l22_pre10", stainlessMat, 3559.79, 3560.09, 150, -1},
    	{"Cu_l22_pre11", cuMat, 3560.09, 3560.19, 150, -1},
    	//{"Air_l22_pre12", airMat, 3560.19, 3560.415, 150, -1},
    	{"PCB_l22_pre13", pcbMat, 3560.415, 3562.015, 150, -1},
    	//{"Airelectronics_l22_pre14", airMat, 3562.015, 3565.745, 150, -1},
    	{"PCBHexaboard_l22_pre15", pcbMat, 3565.745, 3567.345, 150, -1},
    	//{"glue_l22_pre16", glueMat, 3567.345, 3567.42, 150, -1},

    	//{"glue_l23_post1", glueMat, 3567.72, 3567.785, 158, -1},
    	{"kapton_l23_pre1", kaptonMat, 3567.785, 3568.05, 158, -1},
    	//{"glue_l23_pre2", glueMat, 3568.05, 3568.115, 158, -1},
    	{"CuW_l23_pre3", cuMat, 3568.115, 3569.515, 158, -1},
    	{"CoolingplateCu_l23_pre4", cuMat, 3569.515, 3575.565, 158, -1},
    	{"CuW_l23_pre5", cuMat, 3575.565, 3576.965, 158, -1},
    	//{"glue_l23_pre6", glueMat, 3576.965, 3577.03, 158, -1},
    	{"kapton_l23_pre7", kaptonMat, 3577.03, 3577.295, 158, -1},
    	//{"glue_l23_pre8", glueMat, 3577.295, 3577.36, 158, -1},

    	//{"glue_l24_post1", glueMat, 3577.66, 3577.735, 158, -1},
    	{"PCBHexaboard_l24_pre1", pcbMat, 3577.735, 3579.335, 158, -1},
    	//{"Airelectronics_l24_pre2", airMat, 3579.335, 3583.065, 158, -1},
    	{"PCB_l24_pre3", pcbMat, 3583.065, 3584.665, 158, -1},
    	//{"Air_l24_pre4", airMat, 3584.665, 3584.89, 158, -1},
    	{"Cu_l24_pre5", cuMat, 3584.89, 3584.99, 158, -1},
    	{"StainlessSteel_l24_pre6", stainlessMat, 3584.99, 3585.29, 158, -1},
    	//{"glue_l24_pre7", glueMat, 3585.29, 3585.34, 158, -1},
    	{"Lead_l24_pre8", pbMat, 3585.34, 3593.56, 158, -1},
    	//{"glue_l24_pre9", glueMat, 3593.56, 3593.61, 158, -1},
    	{"StainlessSteel_l24_pre10", stainlessMat, 3593.61, 3593.91, 158, -1},
    	{"Cu_l24_pre11", cuMat, 3593.91, 3594.01, 158, -1},
    	//{"Air_l24_pre12", airMat, 3594.01, 3594.235, 158, -1},
    	{"PCB_l24_pre13", pcbMat, 3594.235, 3595.835, 158, -1},
    	//{"Airelectronics_l24_pre14", airMat, 3595.835, 3599.565, 158, -1},
    	{"PCBHexaboard_l24_pre15", pcbMat, 3599.565, 3601.165, 158, -1},
    	//{"glue_l24_pre16", glueMat, 3601.165, 3601.24, 158, -1},

    	//{"glue_l25_post1", glueMat, 3601.54, 3601.605, 158, -1},
    	{"kapton_l25_pre1", kaptonMat, 3601.605, 3601.87, 158, -1},
    	//{"glue_l25_pre2", glueMat, 3601.87, 3601.935, 158, -1},
    	{"CuW_l25_pre3", cuMat, 3601.935, 3603.335, 158, -1},
    	{"CoolingplateCu_l25_pre4", cuMat, 3603.335, 3609.385, 158, -1},
    	{"CuW_l25_pre5", cuMat, 3609.385, 3610.785, 158, -1},
    	//{"glue_l25_pre6", glueMat, 3610.785, 3610.85, 158, -1},
    	{"kapton_l25_pre7", kaptonMat, 3610.85, 3611.115, 158, -1},
    	//{"glue_l25_pre8", glueMat, 3611.115, 3611.18, 158, -1},

    	//{"glue_l26_post1", glueMat, 3611.48, 3611.555, 158, -1},
    	{"PCBHexaboard_l26_pre1", pcbMat, 3611.555, 3613.155, 158, -1},
    	//{"Airelectronics_l26_pre2", airMat, 3613.155, 3616.885, 158, -1},
    	{"PCB_l26_pre3", pcbMat, 3616.885, 3618.485, 158, -1},
    	//{"Air_l26_pre4", airMat, 3618.485, 3618.71, 158, -1},
    	{"StainlessSteel_l26_pre5", stainlessMat, 3618.71, 3619.71, 158, -1},
    	{"StainlessSteel_l26_pre6", stainlessMat, 3619.71, 3664.71, 158, -1},
    	//{"Air_l26_pre7", airMat, 3664.71, 3668.71, 158, -1},
    	{"StainlessSteel_l26_pre8", stainlessMat, 3668.71, 3671.21, 158, -1},
    	//{"Air_l26_pre9", airMat, 3671.21, 3671.61, 158, -1},
    	{"PCB_l26_pre10", pcbMat, 3671.61, 3673.21, 158, -1},
    	//{"Airelectronics_l26_pre11", airMat, 3673.21, 3676.685, 158, -1},
    	{"PCBHexaboard_l26_pre12", pcbMat, 3676.685, 3678.285, 158, -1},
    	//{"glue_l26_pre13", glueMat, 3678.285, 3678.36, 158, -1},

    	//{"glue_l27_post1", glueMat, 3678.66, 3678.735, 158, -1},
    	{"kapton_l27_pre1", kaptonMat, 3678.735, 3678.835, 158, -1},
    	//{"glue_l27_pre2", glueMat, 3678.835, 3678.91, 158, -1},
    	{"PCB_l27_pre3", pcbMat, 3678.91, 3679.91, 158, -1},
    	{"CoolingplateCu_l27_pre4", cuMat, 3679.91, 3686.26, 158, -1},
    	{"Absorberplate_l27_pre5", stainlessMat, 3686.26, 3727.76, 158, -1},
    	//{"Air_l27_pre6", airMat, 3727.76, 3731.76, 158, -1},
    	{"StainlessSteel_l27_pre7", stainlessMat, 3731.76, 3734.26, 158, -1},
    	//{"Air_l27_pre8", airMat, 3734.26, 3734.66, 158, -1},
    	{"PCB_l27_pre9", pcbMat, 3734.66, 3736.26, 158, -1},
    	//{"Airelectronics_l27_pre10", airMat, 3736.26, 3739.735, 158, -1},
    	{"PCBHexaboard_l27_pre11", pcbMat, 3739.735, 3741.335, 158, -1},
    	//{"glue_l27_pre12", glueMat, 3741.335, 3741.41, 158, -1},

    	//{"glue_l28_post1", glueMat, 3741.71, 3741.785, 164, -1},
    	{"kapton_l28_pre1", kaptonMat, 3741.785, 3741.885, 164, -1},
    	//{"glue_l28_pre2", glueMat, 3741.885, 3741.96, 164, -1},
    	{"PCB_l28_pre3", pcbMat, 3741.96, 3742.96, 164, -1},
    	{"CoolingplateCu_l28_pre4", cuMat, 3742.96, 3749.31, 164, -1},
    	{"ABsorberplate_l28_pre5", stainlessMat, 3749.31, 3790.81, 164, -1},
    	//{"Air_l28_pre6", airMat, 3790.81, 3794.81, 164, -1},
    	{"StainlessSteel_l28_pre7", stainlessMat, 3794.81, 3797.31, 164, -1},
    	//{"Air_l28_pre8", airMat, 3797.31, 3797.71, 164, -1},
    	{"PCB_l28_pre9", pcbMat, 3797.71, 3799.31, 164, -1},
    	//{"Airelectronics_l28_pre10", airMat, 3799.31, 3802.785, 164, -1},
    	{"PCBHexaboard_l28_pre11", pcbMat, 3802.785, 3804.385, 164, -1},
    	//{"glue_l28_pre12", glueMat, 3804.385, 3804.46, 164, -1},

    	//{"glue_l29_post1", glueMat, 3804.76, 3804.835, 167, -1},
    	{"kapton_l29_pre1", kaptonMat, 3804.835, 3804.935, 167, -1},
    	//{"glue_l29_pre2", glueMat, 3804.935, 3805.01, 167, -1},
    	{"PCB_l29_pre3", pcbMat, 3805.01, 3806.01, 167, -1},
    	{"CoolingplateCu_l29_pre4", cuMat, 3806.01, 3812.36, 167, -1},
    	{"Absorberplate_l29_pre5", stainlessMat, 3812.36, 3853.86, 167, -1},
    	//{"Air_l29_pre6", airMat, 3853.86, 3857.86, 167, -1},
    	{"StainlessSteel_l29_pre7", stainlessMat, 3857.86, 3860.36, 167, -1},
    	//{"Air_l29_pre8", airMat, 3860.36, 3860.76, 167, -1},
    	{"PCB_l29_pre9", pcbMat, 3860.76, 3862.36, 167, -1},
    	//{"Airelectronics_l29_pre10", airMat, 3862.36, 3865.835, 167, -1},
    	{"PCBHexaboard_l29_pre11", pcbMat, 3865.835, 3867.435, 167, -1},
    	//{"glue_l29_pre12", glueMat, 3867.435, 3867.51, 167, -1},

    	//{"glue_l30_post1", glueMat, 3867.81, 3867.885, 169, -1},
    	{"kapton_l30_pre1", kaptonMat, 3867.885, 3867.985, 169, -1},
    	//{"glue_l30_pre2", glueMat, 3867.985, 3868.06, 169, -1},
    	{"PCB_l30_pre3", pcbMat, 3868.06, 3869.06, 169, -1},
    	{"CoolingplateCu_l30_pre4", cuMat, 3869.06, 3875.41, 169, -1},
    	{"Absorber_l30_pre5", stainlessMat, 3875.41, 3916.91, 169, -1},
    	//{"Air_l30_pre6", airMat, 3916.91, 3920.91, 169, -1},
    	{"StainlessSteel_l30_pre7", stainlessMat, 3920.91, 3923.41, 169, -1},
    	//{"Air_l30_pre8", airMat, 3923.41, 3923.81, 169, -1},
    	{"PCB_l30_pre9", pcbMat, 3923.81, 3925.41, 169, -1},
    	//{"Airelectronics_l30_pre10", airMat, 3925.41, 3928.885, 169, -1},
    	{"PCBHexaboard_l30_pre11", pcbMat, 3928.885, 3930.485, 169, -1},
    	//{"glue_l30_pre12", glueMat, 3930.485, 3930.56, 169, -1},

    	//{"glue_l31_post1", glueMat, 3930.86, 3930.935, 167, -1},
    	{"kapton_l31_pre1", kaptonMat, 3930.935, 3931.035, 167, -1},
    	//{"glue_l31_pre2", glueMat, 3931.035, 3931.11, 167, -1},
    	{"PCB_l31_pre3", pcbMat, 3931.11, 3932.11, 167, -1},
    	{"CoolingplateCu_l31_pre4", cuMat, 3932.11, 3938.46, 167, -1},
    	{"Absorberplate_l31_pre5", stainlessMat, 3938.46, 3979.96, 167, -1},
    	//{"Air_l31_pre6", airMat, 3979.96, 3983.96, 167, -1},
    	{"StainlessSteel_l31_pre7", stainlessMat, 3983.96, 3986.46, 167, -1},
    	//{"Air_l31_pre8", airMat, 3986.46, 3986.86, 167, -1},
    	{"PCB_l31_pre9", pcbMat, 3986.86, 3988.46, 167, -1},
    	//{"Airelectronics_l31_pre10", airMat, 3988.46, 3991.935, 167, -1},
    	{"PCBHexaboard_l31_pre11", pcbMat, 3991.935, 3993.535, 167, -1},
    	//{"glue_l31_pre12", glueMat, 3993.535, 3993.61, 167, -1},

    	//{"glue_l32_post1", glueMat, 3993.91, 3993.985, 178, -1},
    	{"kapton_l32_pre1", kaptonMat, 3993.985, 3994.085, 178, -1},
    	//{"glue_l32_pre2", glueMat, 3994.085, 3994.16, 178, -1},
    	{"PCB_l32_pre3", pcbMat, 3994.16, 3995.16, 178, -1},
    	{"CoolingplateCu_l32_pre4", cuMat, 3995.16, 4001.51, 178, -1},
    	{"Absorberplate_l32_pre5", stainlessMat, 4001.51, 4043.01, 178, -1},
    	//{"Air_l32_pre6", airMat, 4043.01, 4047.01, 178, -1},
    	{"StainlessSteel_l32_pre7", stainlessMat, 4047.01, 4049.51, 178, -1},
    	//{"Air_l32_pre8", airMat, 4049.51, 4049.91, 178, -1},
    	{"PCB_l32_pre9", pcbMat, 4049.91, 4051.51, 178, -1},
    	//{"Airelectronics_l32_pre10", airMat, 4051.51, 4054.985, 178, -1},
    	{"PCBHexaboard_l32_pre111", pcbMat, 4054.985, 4056.585, 178, -1},
    	//{"glue_l32_pre12", glueMat, 4056.585, 4056.66, 178, -1},

    	//{"glue_l33_post1", glueMat, 4056.96, 4057.035, 183, -1},
    	{"kapton_l33_pre1", kaptonMat, 4057.035, 4057.135, 183, -1},
    	//{"glue_l33_pre2", glueMat, 4057.135, 4057.21, 183, -1},
    	{"PCB_l33_pre3", pcbMat, 4057.21, 4058.21, 183, -1},
    	{"CoolingplateCu_l33_pre4", cuMat, 4058.21, 4064.56, 183, -1},
    	{"Absorberplate_l33_pre5", stainlessMat, 4064.56, 4106.06, 183, -1},
    	//{"Air_l33_pre6", airMat, 4106.06, 4110.06, 183, -1},
    	{"StainlessSteel_l33_pre7", stainlessMat, 4110.06, 4112.56, 183, -1},
    	//{"Air_l33_pre8", airMat, 4112.56, 4112.96, 183, -1},
    	{"PCB_l33_pre9", pcbMat, 4112.96, 4114.56, 183, -1},
    	//{"Airelectronics_l33_pre10", airMat, 4114.56, 4118.035, 183, -1},
    	{"PCBHexaboard_l33_pre11", pcbMat, 4118.035, 4119.635, 183, -1},
    	//{"glue_l33_pre12", glueMat, 4119.635, 4119.71, 183, -1},

    	//{"glue_l34_post1", glueMat, 4120.01, 4120.085, 197, -1},
    	{"kapton_l34_pre1", kaptonMat, 4120.085, 4120.185, 197, -1},
    	//{"glue_l34_pre2", glueMat, 4120.185, 4120.26, 197, -1},
    	{"PCB_l34_pre3", pcbMat, 4120.26, 4121.26, 197, -1},
    	{"CoolingplateCu_l34_pre4", cuMat, 4121.26, 4127.61, 197, -1},
    	{"Absorberplate_l34_pre5", stainlessMat, 4127.61, 4169.11, 197, -1},
    	//{"Air_l34_pre6", airMat, 4169.11, 4173.11, 197, -1},
    	{"StainlessSteel_l34_pre7", stainlessMat, 4173.11, 4175.61, 197, -1},
    	//{"Air_l34_pre8", airMat, 4175.61, 4176.01, 197, -1},
    	{"PCB_l34_pre9", pcbMat, 4176.01, 4177.61, 197, -1},
    	//{"Airelectronics_l34_pre10", airMat, 4177.61, 4181.085, 197, -1},
    	{"PCBHexaboard_l34_pre11", pcbMat, 4181.085, 4182.685, 197, -1},
    	//{"glue_l34_pre12", glueMat, 4182.685, 4182.76, 197, -1},

    	//{"glue_l35_post1", glueMat, 4183.06, 4183.135, 206, -1},
    	{"kapton_l35_pre1", kaptonMat, 4183.135, 4183.235, 206, -1},
    	//{"glue_l35_pre2", glueMat, 4183.235, 4183.31, 206, -1},
    	{"PCB_l35_pre3", pcbMat, 4183.31, 4184.31, 206, -1},
    	{"CoolingplateCu_l35_pre4", cuMat, 4184.31, 4190.66, 206, -1},
    	{"Absorberplate_l35_pre5", stainlessMat, 4190.66, 4232.16, 206, -1},
    	//{"Air_l35_pre6", airMat, 4232.16, 4236.16, 206, -1},
    	{"StainlessSteel_l35_pre7", stainlessMat, 4236.16, 4238.66, 206, -1},
    	//{"Air_l35_pre8", airMat, 4238.66, 4239.06, 206, -1},
    	{"PCB_l35_pre9", pcbMat, 4239.06, 4240.66, 206, -1},
    	//{"Airelectronics_l35_pre10", airMat, 4240.66, 4244.135, 206, -1},
    	{"PCBHexaboard_l35_pre11", pcbMat, 4244.135, 4245.735, 206, -1},
    	//{"glue_l35_pre12", glueMat, 4245.735, 4245.81, 206, -1},

    	//{"glue_l36_post1", glueMat, 4246.11, 4246.185, 215, -1},
    	{"kapton_l36_pre1", kaptonMat, 4246.185, 4246.285, 215, -1},
    	//{"glue_l36_pre2", glueMat, 4246.285, 4246.36, 215, -1},
    	{"PCB_l36_pre3", pcbMat, 4246.36, 4247.36, 215, -1},
    	{"CoolingplateCu_l36_pre4", cuMat, 4247.36, 4253.71, 215, -1},
    	{"Absorberplate_l36_pre5", stainlessMat, 4253.71, 4295.21, 215, -1},
    	//{"Air_l36_pre6", airMat, 4295.21, 4299.21, 215, -1},
    	{"StainlessSteel_l36_pre7", stainlessMat, 4299.21, 4301.71, 215, -1},
    	//{"Air_l36_pre8", airMat, 4301.71, 4302.11, 215, -1},
    	{"PCB_l36_pre9", pcbMat, 4302.11, 4303.71, 215, -1},
    	//{"Airelectronics_l36_pre10", airMat, 4303.71, 4307.185, 215, -1},
    	{"PCBHexaboard_l36_pre11", pcbMat, 4307.185, 4308.785, 215, -1},
    	//{"glue_l36_pre12", glueMat, 4308.785, 4308.86, 215, -1},

    	//{"glue_l37_post1", glueMat, 4309.16, 4309.235, 215, -1},
    	{"kapton_l37_pre1", kaptonMat, 4309.235, 4309.335, 215, -1},
    	//{"glue_l37_pre2", glueMat, 4309.335, 4309.41, 215, -1},
    	{"PCB_l37_pre3", pcbMat, 4309.41, 4310.41, 215, -1},
    	{"CoolingplateCu_l37_pre4", cuMat, 4310.41, 4316.76, 215, -1},
    	{"Absorberplate_l37_pre5", stainlessMat, 4316.76, 4377.46, 215, -1},
    	//{"Air_l37_pre6", airMat, 4377.46, 4381.46, 215, -1},
    	{"StainlessSteel_l37_pre7", stainlessMat, 4381.46, 4383.96, 215, -1},
    	//{"Air_l37_pre8", airMat, 4383.96, 4384.36, 215, -1},
    	{"PCB_l37_pre9", pcbMat, 4384.36, 4385.96, 215, -1},
    	//{"Airelectronics_l37_pre10", airMat, 4385.96, 4389.435, 215, -1},
    	{"PCBHexaboard_l37_pre11", pcbMat, 4389.435, 4391.035, 215, -1},
    	//{"glue_l37_pre12", glueMat, 4391.035, 4391.11, 215, -1},

    	//{"glue_l38_post1", glueMat, 4391.41, 4391.485, 234, -1},
    	{"kapton_l38_pre1", kaptonMat, 4391.485, 4391.585, 234, -1},
    	//{"glue_l38_pre2", glueMat, 4391.585, 4391.66, 234, -1},
    	{"PCB_l38_pre3", pcbMat, 4391.66, 4392.66, 234, -1},
    	{"CoolingplateCu_l38_pre4", cuMat, 4392.66, 4399.01, 234, -1},
    	{"Absorberplate_l38_pre5", stainlessMat, 4399.01, 4459.71, 234, -1},
    	//{"Air_l38_pre6", airMat, 4459.71, 4463.71, 234, -1},
    	{"StainlessSteel_l38_pre7", stainlessMat, 4463.71, 4466.21, 234, -1},
    	//{"Air_l38_pre8", airMat, 4466.21, 4466.61, 234, -1},
    	{"PCB_l38_pre9", pcbMat, 4466.61, 4468.21, 234, -1},
    	//{"Airelectronics_l38_pre10", airMat, 4468.21, 4471.685, 234, -1},
    	{"PCBHexaboard_l38_pre11", pcbMat, 4471.685, 4473.285, 234, -1},
    	//{"glue_l38_pre12", glueMat, 4473.285, 4473.36, 234, -1},

    	//{"glue_l39_post1", glueMat, 4473.66, 4473.735, 245, -1},
    	{"kapton_l39_pre1", kaptonMat, 4473.735, 4473.835, 245, -1},
    	//{"glue_l39_pre2", glueMat, 4473.835, 4473.91, 245, -1},
    	{"PCB_l39_pre3", pcbMat, 4473.835, 4474.91, 245, -1},
    	{"CoolingplateCu_l39_pre4", cuMat, 4474.91, 4481.26, 245, -1},
    	{"Absorberplate_l39_pre5", stainlessMat, 4481.26, 4541.96, 245, -1},
    	//{"Air_l39_pre6", airMat, 4541.96, 4545.96, 245, -1},
    	{"StainlessSteel_l39_pre7", stainlessMat, 4545.96, 4548.46, 245, -1},
    	//{"Air_l39_pre8", airMat, 4548.46, 4548.86, 245, -1},
    	{"PCB_l39_pre9", pcbMat, 4548.86, 4550.46, 245, -1},
    	//{"Airelectronics_l39_pre10", airMat, 4550.46, 4553.935, 245, -1},
    	{"PCBHexaboard_l39_pre11", pcbMat, 4553.935, 4555.535, 245, -1},
    	//{"glue_l39_pre12", glueMat, 4555.535, 4555.61, 245, -1},

    	//{"glue_l40_post1", glueMat, 4555.91, 4555.985, 256, -1},
    	{"kapton_l40_pre1", kaptonMat, 4555.985, 4556.085, 256, -1},
    	//{"glue_l40_pre2", glueMat, 4556.085, 4556.16, 256, -1},
    	{"PCBHexaboard_l40_pre3", pcbMat, 4556.16, 4557.16, 256, -1},
    	{"CoolingplateCu_l40_pre4", cuMat, 4557.16, 4563.51, 256, -1},
    	{"Absorberplate_l40_pre5", stainlessMat, 4563.51, 4624.21, 256, -1},
    	//{"Air_l40_pre6", airMat, 4624.21, 4628.21, 256, -1},
    	{"StainlessSteel_l40_pre7", stainlessMat, 4628.21, 4630.71, 256, -1},
    	//{"Air_l40_pre8", airMat, 4630.71, 4631.11, 256, -1},
    	{"PCB_l40_pre9", pcbMat, 4631.11, 4632.71, 256, -1},
    	//{"Airelectronics_l40_pre10", airMat, 4632.71, 4636.185, 256, -1},
    	{"PCBHexaboard_l40_pre11", pcbMat, 4636.185, 4637.785, 256, -1},
    	//{"glue_l40_pre12", glueMat, 4637.785, 4637.86, 256, -1},

    	//{"glue_l41_post1", glueMat, 4638.16, 4638.235, 256, -1},
    	{"kapton_l41_pre1", kaptonMat, 4638.235, 4638.335, 256, -1},
    	//{"glue_l41_pre2", glueMat, 4638.335, 4638.41, 256, -1},
    	{"PCB_l41_pre3", pcbMat, 4638.41, 4639.61, 256, -1},
    	{"CoolingplateCu_l41_pre4", cuMat, 4639.61, 4645.76, 256, -1},
    	{"ABsorberplate_l41_pre5", stainlessMat, 4645.76, 4706.46, 256, -1},
    	//{"Air_l41_pre6", airMat, 4706.46, 4710.46, 256, -1},
    	{"StainlessSteel_l41_pre7", stainlessMat, 4710.46, 4712.96, 256, -1},
    	//{"Air_l41_pre8", airMat, 4712.96, 4713.36, 256, -1},
    	{"PCB_l41_pre9", pcbMat, 4713.36, 4714.96, 256, -1},
    	//{"Airelectronics_l41_pre10", airMat, 4714.96, 4718.435, 256, -1},
    	{"PCBHexaboard_l41_pre11", pcbMat, 4718.435, 4720.035, 256, -1},
    	//{"glue_l41_pre12", glueMat, 4720.035, 4720.11, 256, -1},

    	//{"glue_l42_post1", glueMat, 4720.41, 4720.485, 256, -1},
    	{"kapton_l42_pre1", kaptonMat, 4720.485, 4720.585, 256, -1},
    	//{"glue_l42_pre2", glueMat, 4720.585, 4720.66, 256, -1},
    	{"PCBHexaboard_l42_pre3", pcbMat, 4720.66, 4721.66, 256, -1},
    	{"CoolingplateCu_l42_pre4", cuMat, 4721.66, 4728.01, 256, -1},
    	{"Absorberplate_l42_pre5", stainlessMat, 4728.01, 4788.71, 256, -1},
    	//{"Air_l42_pre6", airMat, 4788.71, 4792.71, 256, -1},
    	{"StainlessSteel_l42_pre7", stainlessMat, 4792.71, 4795.21, 256, -1},
    	//{"Air_l42_pre8", airMat, 4795.21, 4795.61, 256, -1},
    	{"PCB_l42_pre9", pcbMat, 4795.61, 4797.21, 256, -1},
    	//{"Airelectronics_l42_pre10", airMat, 4797.21, 4800.685, 256, -1},
    	{"PCBHexaboard_l42_pre11", pcbMat, 4800.685, 4802.285, 256, -1},
    	//{"glue_l42_pre12", glueMat, 4802.285, 4802.36, 256, -1},

    	//{"glue_l43_post1", glueMat, 4802.66, 4802.735, 256, -1},
    	{"kapton_l43_pre1", kaptonMat, 4802.735, 4802.835, 256, -1},
    	//{"glue_l43_pre2", glueMat, 4802.835, 4802.91, 256, -1},
    	{"PCB_l43_pre3", pcbMat, 4802.91, 4803.91, 256, -1},
    	{"CoolingplateCu_l43_pre4", cuMat, 4803.91, 4810.26, 256, -1},
    	{"Absorberplate_l43_pre5", stainlessMat, 4810.26, 4870.96, 256, -1},
    	//{"Air_l43_pre6", airMat, 4870.96, 4874.96, 256, -1},
    	{"StainlessSteel_l43_pre7", stainlessMat, 4874.96, 4877.46, 256, -1},
    	//{"Air_l43_pre8", airMat, 4877.46, 4877.86, 256, -1},
    	{"PCB_l43_pre9", pcbMat, 4877.86, 4879.46, 256, -1},
    	//{"Airelectronics_l43_pre10", airMat, 4879.46, 4882.935, 256, -1},
    	{"PCBHexaboard_l43_pre11", pcbMat, 4882.935, 4884.535, 256, -1},
    	//{"glue_l43_pre12", glueMat, 4884.535, 4884.61, 256, -1},

    	//{"glue_l44_post1", glueMat, 4884.91, 4884.985, 256, -1},
    	{"kapton_l44_pre1", kaptonMat, 4884.985, 4885.085, 256, -1},
    	//{"glue_l44_pre2", glueMat, 4885.085, 4885.16, 256, -1},
    	{"PCB_l44_pre3", pcbMat, 4885.16, 4886.16, 256, -1},
    	{"CoolingplateCu_l44_pre4", cuMat, 4886.16, 4892.51, 256, -1},
    	{"Absorberplate_l44_pre5", stainlessMat, 4892.51, 4953.21, 256, -1},
    	//{"Air_l44_pre6", airMat, 4953.21, 4957.21, 256, -1},
    	{"StainlessSteel_l44_pre7", stainlessMat, 4957.21, 4959.71, 256, -1},
    	//{"Air_l44_pre8", airMat, 4959.71, 4960.11, 256, -1},
    	{"PCB_l44_pre9", pcbMat, 4960.11, 4961.71, 256, -1},
    	//{"Airelectronics_l44_pre10", airMat, 4961.71, 4965.185, 256, -1},
    	{"PCBHexaboard_l44_pre11", pcbMat, 4965.185, 4966.785, 256, -1},
    	//{"glue_l44_pre12", glueMat, 4966.785, 4966.86, 256, -1},

    	//{"glue_l45_post1", glueMat, 4967.16, 4967.235, 256, -1},
    	{"kapton_l45_pre1", kaptonMat, 4967.235, 4967.335, 256, -1},
    	//{"glue_l45_pre2", glueMat, 4967.335, 4967.41, 256, -1},
    	{"PCB_l45_pre3", pcbMat, 4967.41, 4968.41, 256, -1},
    	{"CoolingplateCu_l45_pre4", cuMat, 4968.41, 4974.76, 256, -1},
    	{"Absorberplate_l45_pre5", stainlessMat, 4974.76, 5035.46, 256, -1},
    	//{"Air_l45_pre6", airMat, 5035.46, 5039.46, 256, -1},
    	{"StainlessSteel_l45_pre7", stainlessMat, 5039.46, 5041.96, 256, -1},
    	//{"Air_l45_pre8", airMat, 5041.96, 5042.36, 256, -1},
    	{"PCB_l45_pre9", pcbMat, 5042.36, 5043.96, 256, -1},
    	//{"Airelectronics_l45_pre10", airMat, 5043.96, 5047.435, 256, -1},
    	{"PCBHexaboard_l45_pre11", pcbMat, 5047.435, 5049.035, 256, -1},
    	//{"glue_l45_pre12", glueMat, 5049.035, 5049.11, 256, -1},

    	//{"glue_l46_post1", glueMat, 5049.41, 5049.485, 256, -1},
    	{"kapton_l46_pre1", kaptonMat, 5049.485, 5049.585, 256, -1},
    	//{"glue_l46_pre2", glueMat, 5049.585, 5049.66, 256, -1},
    	{"PCB_l46_pre3", pcbMat, 5049.66, 5050.66, 256, -1},
    	{"CoolingplateCu_l46_pre4", cuMat, 5050.66, 5057.01, 256, -1},
    	{"Absorberplate_l46_pre5", stainlessMat, 5057.01, 5117.71, 256, -1},
    	//{"Air_l46_pre6", airMat, 5117.71, 5121.71, 256, -1},
    	{"StainlessSteel_l46_pre7", stainlessMat, 5121.71, 5124.21, 256, -1},
    	//{"Air_l46_pre8", airMat, 5124.21, 5124.61, 256, -1},
    	{"PCB_l46_pre9", pcbMat, 5124.61, 5126.21, 256, -1},
    	//{"Airelectronics_l46_pre10", airMat, 5126.21, 5129.685, 256, -1},
    	{"PCBHexaboard_l46_pre11", pcbMat, 5129.685, 5131.285, 256, -1},
    	//{"glue_l46_pre12", glueMat, 5131.285, 5131.36, 256, -1},

    	//{"glue_l47_post1", glueMat, 5131.66, 5131.735, 245, -1},
    	{"kapton_l47_pre1", kaptonMat, 5131.735, 5131.835, 245, -1},
    	//{"glue_l47_pre2", glueMat, 5131.835, 5131.91, 245, -1},
    	{"PCB_l47_pre3", pcbMat, 5131.91, 5132.91, 256, -1},
    	{"CoolingplateCu_l47_pre4", cuMat, 5132.91, 5139.26, 245, -1},
    	
    };

    // ---------------------
    // Create silicon layers
    // ---------------------
    int siLayerCounter = 0;
    for (const auto& layer : siliconLayers) {
        G4double thickness = (layer.zMax - layer.zMin) * mm;
        G4double zPos = (layer.zMin + layer.zMax) / 2.0 * mm;
        G4double outerR = layer.outerRadius * cm;

        G4String solidName = "solid_" + layer.name;
        G4Tubs* solidLayer = new G4Tubs(solidName, innerRadius, outerR, thickness / 2.0, 0.0, 2.0 * CLHEP::pi);

        G4String logicName = "logic_" + layer.name;
        G4LogicalVolume* logicLayer = new G4LogicalVolume(solidLayer, layer.material, logicName);

        G4VisAttributes* siVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.5));
        siVis->SetForceSolid(true);
        logicLayer->SetVisAttributes(siVis);

        G4String physName = "phys_" + layer.name;
        new G4PVPlacement(
            0,
            G4ThreeVector(0, 0, zPos),
            logicLayer,
            physName,
            logicWorld,
            false,
            layer.layerNumber,
            true
        );

        siLayerCounter++;
    }

    // ---------------------
    // Create non-silicon layers
    // ---------------------
    int nonSiLayerCounter = 0;
    for (const auto& layer : nonSiliconLayers) {
        G4double thickness = (layer.zMax - layer.zMin) * mm;
        G4double zPos = (layer.zMin + layer.zMax) / 2.0 * mm;
        G4double outerR = layer.outerRadius * cm;

        G4String solidName = "solid_" + layer.name;
        G4Tubs* solidLayer = new G4Tubs(solidName, innerRadius, outerR, thickness / 2.0, 0.0, 2.0 * CLHEP::pi);

        G4String logicName = "logic_" + layer.name;
        G4LogicalVolume* logicLayer = new G4LogicalVolume(solidLayer, layer.material, logicName);

        G4VisAttributes* vis = nullptr;
        if (layer.material == pbMat)
            vis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.8));
        else if (layer.material == cuMat)
            vis = new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.5));
        else if (layer.material == stainlessMat)
            vis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7, 0.5));
        else if (layer.material == kaptonMat)
            vis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.3));
        else if (layer.material == pcbMat)
            vis = new G4VisAttributes(G4Colour(0.0, 0.5, 0.0, 0.3));
        else if (layer.material == airMat)
            vis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.1));
        else if (layer.material == glueMat)
            vis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.1));
        else
            vis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.5));

        vis->SetForceSolid(false);
        logicLayer->SetVisAttributes(vis);
        

        G4String physName = "phys_" + layer.name;
        new G4PVPlacement(
            0,
            G4ThreeVector(0, 0, zPos),
            logicLayer,
            physName,
            logicWorld,
            false,
            2000 + nonSiLayerCounter,
            true
        );

        nonSiLayerCounter++;
    }

    // ---------------------
    // Register sensitive detector
    // ---------------------
    MySensitiveDetector* sensDet = new MySensitiveDetector("SensitiveDetector");
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);

    // Attach sensitive detector to silicon layers
    for (const auto& layer : siliconLayers) {
        G4String logicName = "logic_" + layer.name;
        G4LogicalVolume* layerLogic = G4LogicalVolumeStore::GetInstance()->GetVolume(logicName);
        if (layerLogic)
            layerLogic->SetSensitiveDetector(sensDet);
    }

    return physWorld;
}
