# HGCAL Simulation Workflow

Complete end-to-end workflow for HGCAL simulation, from signal generation through ML-ready data preparation.

## Overview

This pipeline processes both signal events and pileup events through simulation, segmentation, and combination stages to produce ML-training-ready datasets for YOLO-based particle detection and energy regression.

---

## Complete Workflow Diagram

```
SIGNAL PATH                                    PILEUP PATH
═══════════════════════════════════════════   ═══════════════════════════════════════════

[1] Geant4 Signal Simulation                  [A] Pythia8 Generation
    ↓                                              ↓
Signal_nPU_000_Pt_025_Eta_170_Events          Pythia8_PU_Events_20K.root
_2K_Set01_Step1.root                               ↓
    ↓                                         [B] ROOT to Text Conversion
[2] Segmentation                                   ↓
    ↓                                          PileUp.txt
Signal_nPU_000_Pt_025_Eta_170_Events_2K_           ↓
Set01_Step2.root                              [C] Geant4 Pileup Simulation
                                                   ↓
                                              PileUp_Pt_GT_pt3_Eta_15_31_
                                              Events_20K_Step1.root
                                                   ↓
                                              [D] Segmentation
                                                   ↓
                                              PileUp_Pt_GT_pt3_Eta_15_31_
                                              Events_20K_Step2.root

                    ╔═══════════════════════════════════════╗
                    ║  [3] Signal + Pileup Addition         ║
                    ║  Combines both paths with random      ║
                    ║  unique pileup for each signal event  ║
                    ╚═══════════════════════════════════════╝
                                    ↓
                    Combined ROOT file with η-φ trees,
                    YOLO labels, and validation data
                                    ↓
                    ╔═══════════════════════════════════════╗
                    ║  [4] YOLO Training Data Preparation   ║
                    ║  Converts ROOT output to numpy        ║
                    ║  images, labels, and energy targets   ║
                    ╚═══════════════════════════════════════╝
                                    ↓
                    ML-Ready Dataset (images / labels / energy)
```

---

## Detailed Step-by-Step Process

### SIGNAL PATH

#### Step 1: Geant4 Signal Simulation
**Purpose:** Simulate physics signal events (electron/positron/photon/muon) through HGCAL

**Input:**
- Particle configuration (type, energy, eta range, etc.)

**Output:**
```
Electron_Step1/
└── Electron_nPU_000_Pt_025_Eta_170_Events_2K_Set01_Step1.root
```

**Description:** Raw Geant4 simulation output containing all hits and energy deposits for signal particles.

---

#### Step 2: Signal Segmentation (Digitisation)
**Purpose:** Convert continuous energy deposits to discrete detector cells

**Code:** `Segmentation/cellwise_segmentation.C`

**Input:**
```
Electron_nPU_000_Pt_025_Eta_170_Events_2K_Set01_Step1.root
```

**Output:**
```
Electron_Step2/
└── Electron_nPU_000_Pt_025_Eta_170_Events_2K_Set01_Step2.root
```

**Description:** Digitised signal with both pixel-wise and η–φ cell-wise segmentation trees. Energy deposits are aggregated per detector cell, with ADC values computed for each cell. The original `GeneratorInfo` tree is preserved.

---

### PILEUP PATH

#### Step A: Pythia8 Pileup Generation

**Output:**
```
PileUp_Pythia_Data/
└── Pythia8_PU_Events_20K.root
```

**Description:** Raw Pythia8 output containing 20K minimum-bias pileup events.

---

#### Step B: ROOT to Text Conversion
**Purpose:** Convert Pythia8 ROOT format to text format for Geant4 input

**Code:** `PileupUtils/rootToText.C`

**Input:**
```
Pythia8_PU_Events_20K.root
```

**Output:**
```
PileUp.txt
```

**Description:** Reads the `gen` tree, applies kinematic cuts (`pT > 0.3`, `1.5 ≤ η ≤ 3.1`), removes non-detectable or short-lived particles, and writes a structured text file that Geant4 can read.

---

#### Step C: Geant4 Pileup Simulation
**Purpose:** Simulate pileup events through HGCAL detector

**Input:**
```
PileUp.txt
```

**Output:**
```
Pileup_Step1_Data/
└── PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step1.root
```

**Description:** Raw Geant4 simulation of pileup events with detector response.

---

#### Step D: Pileup Segmentation (Digitisation)
**Purpose:** Digitise pileup events to detector cells

**Code:** `Segmentation/cellwise_segmentation.C`

**Input:**
```
PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step1.root
```

**Output:**
```
PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
```

**Description:** Digitised pileup with pixel-wise and η–φ cell-wise segmentation, identical in format to the signal segmentation output.

---

### COMBINATION STAGE

#### Step 3: Signal + Pileup Addition
**Purpose:** Combine signal with pileup to create realistic detector conditions

**Code:** `SignalToPileupAddition/addPileupToSignal.C`

**Inputs:**
```
Signal: Electron_nPU_000_Pt_025_Eta_170_Events_2K_Set01_Step2.root
Pileup: PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
```

**Parameters:**
- `nPU`: Number of pileup events to overlay per signal event (e.g., 70)

**Output:**
```
Electron_nPU_070_Pt_025_Eta_170_Events_2K_Set01_Step2.root
```

**Output trees:**
| Tree | Description |
|------|-------------|
| `Signal_GeneratorInfo` | Generator-level info from the signal file |
| `Pileup_GeneratorInfo` | Generator-level info from the pileup file |
| `Signal_Eta_Phi_CellWiseSegmentation` | Original segmented signal hits |
| `Pileup_Eta_Phi_CellWiseSegmentation` | Original segmented pileup hits |
| `Eta_Phi_CellWiseSegmentation` | Combined signal + pileup detector hits |
| `EventMapping` | Which pileup events were used for each signal event |
| `YOLOLabels` | Seed position and containment variables (f90, f95, f98) |
| `validation_tree` | Per-layer NPix, EMin, EMax, ETotal |

**Description:** Each signal event is combined with `nPU` randomly selected unique pileup events. Overlapping cell energies and ADC values are summed. The output is ready for ML data preparation.

---

### ML DATA PREPARATION

#### Step 4: YOLO Training Data Preparation
**Purpose:** Convert combined ROOT output into a machine-learning-ready dataset

**Code:** `YOLO/rootToNumpy.py`

**Input:**
```
Electron_nPU_070_Pt_025_Eta_170_Events_2K_Set01_Step2.root
```

**Output:**
```
dataset/
├── images/   → detector images (.npy)    [736×736 → 16-channel]
├── labels/   → YOLO bounding box labels (.txt)
└── energy/   → regression targets (.txt)
```

**Description:**
1. Constructs a **736 × 736 × 47** detector image from ADC values per event.
2. Collapses the 47 layers into **16 channels** to reduce dimensionality.
3. Applies noise floor subtraction and logarithmic transformation.
4. Splits events into **train / validation / test** sets.
5. Generates YOLO labels from seed position and containment variables.
6. Extracts energy regression targets from generator-level information.

---

## File Naming Convention

### Signal Files
```
{Particle}_nPU_{nPU}_Pt_{Energy}_Eta_{EtaRange}_Events_{N}_Set{XX}_Step{S}.root
```
| Field | Example | Meaning |
|-------|---------|---------|
| `Particle` | `Electron` | Particle type (Electron, Positron, Photon, Muon) |
| `Energy` | `025` | Transverse momentum in GeV (25 GeV) |
| `EtaRange` | `170` | Pseudorapidity (1.70) |
| `N` | `2K` | Number of events |
| `nPU` | `070` | Number of pileup events (000 = pure signal) |
| `Step` | `1` or `2` | Processing stage (1 = raw simulation, 2 = digitised) |

### Pileup File
```
PileUp_Pt_GT_pt{Y}_Eta_{A}_{B}_Events_{N}_Step{S}.root
```
| Field | Example | Meaning |
|-------|---------|---------|
| `Pt_GT_pt{Y}` | `pt3` | Minimum pT threshold (0.3 GeV) |
| `Eta_{A}_{B}` | `15_31` | Pseudorapidity range (1.5 to 3.1) |
| `Step` | `1` or `2` | Processing stage |

### Combined Files
```
{Particle}_nPU_{nPU}_Pt_{Energy}_Eta_{EtaRange}_Events_{N}_Set{XX}_Combined.root
```

---

## Requirements

- **ROOT** (CERN Data Analysis Framework)
- **Geant4** (detector simulation)
- **Pythia8** (pileup generation)
- **Python 3** with `numpy`, `uproot` (or `ROOT` PyROOT bindings)
