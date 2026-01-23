# HGCAL Simulation Workflow

Complete end-to-end workflow for HGCAL simulation, from signal generation through ML-ready data preparation.

## Overview

This pipeline processes both signal events and pileup events through simulation, filtering, segmentation, and combination stages to produce ML-training-ready datasets.

---

## Complete Workflow Diagram

```
SIGNAL PATH                                    PILEUP PATH
═══════════════════════════════════════════   ═══════════════════════════════════════════

[1] Geant4 Signal Simulation                  [A] Pythia8 Generation
    ↓                                              ↓
Signal_Pt_025_Eta_170_Events_2K_              Pythia8_PU_Events_20K.root
PU_000_Set01_Step1.root                           ↓
    ↓                                          [B] ROOT to Text Conversion
[2] SimhitFilter                                   ↓
    ↓                                          PileUp.txt
Signal_Pt_025_Eta_170_Events_2K_                  ↓
PU_000_Set01_Step1_trimmed.root              [C] Geant4 Pileup Simulation
    ↓                                              ↓
[3] Segmentation                               PileUp_Pt_GT_pt3_Eta_15_31_
    ↓                                          Events_20K_Step1.root
Signal_Pt_025_Eta_170_Events_2K_                  ↓
PU_000_Set01_Step2.root                      [D] SimhitFilter
                                                   ↓
                                               PileUp_Pt_GT_pt3_Eta_15_31_
                                               Events_20K_Step1_trimmed.root
                                                   ↓
                                               [E] Segmentation
                                                   ↓
                                               PileUp_Pt_GT_pt3_Eta_15_31_
                                               Events_20K_Step2.root
                                                   ↓
                                               [F] Create Pileup File (with nPU)
                                                   ↓
                                               PileUp_nPU_070_Pt_GT_pt3_Eta_15_31_
                                               Events_20K_Step2.root

                    ╔═══════════════════════════════════════╗
                    ║  [4] Signal + Pileup Addition         ║
                    ║  Combines both paths with random      ║
                    ║  unique pileup for each signal event  ║
                    ╚═══════════════════════════════════════╝
                                    ↓
                    ML-Ready Dataset with η-φ trees
```

---

## Detailed Step-by-Step Process

### SIGNAL PATH

#### Step 1: Geant4 Signal Simulation
**Purpose:** Simulate physics signal events (electron/positron/photon/muon) through HGCAL

**Input:**
- Particle configuration (type, energy, eta range, etc)

**Output:**
```
Electron_Step1/
└── Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step1.root
```

**Description:** Raw Geant4 simulation output containing all hits and energy deposits for signal particles.

---

#### Step 2: Signal Filtering
**Purpose:** Reduce file size by applying an energy threshold

**Code:** `SimhitFilter/filter.C`

**Input:**
```
Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step1.root
```

**Output:**
```
Electron_Step1_Filtered/
└── Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step1_trimmed.root
```

**Description:** Filtered output retaining only hits with energy deposits > 10 eV.

---

#### Step 3: Signal Segmentation (Digitisation)
**Purpose:** Convert continuous energy deposits to discrete detector cells

**Code:** `Segmentation/cellwise_segmentation.C`

**Input:**
```
Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step1_trimmed.root
```

**Output:**
```
Electron_Step2/
└── Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root
```

**Description:** Digitised signal with both pixel-wise and η-φ cell-wise segmentation trees.

---

### PILEUP PATH

#### Step A: Pythia8 Pileup Generation

**Output:**
```
PileUp_Pythia_Data/
├── Pythia8_PU_Events_20K.root
└── PileUp.txt (to be generated)
```

**Description:** Raw Pythia8 output containing 20K pileup events.

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

**Description:** Text file containing particle information that Geant4 can read.

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

#### Step D: Pileup Filtering
**Purpose:** Apply energy threshold to reduce file size

**Code:** `SimhitFilter/filter.C`

**Input:**
```
PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step1.root
```

**Output:**
```
Pileup_Step1_Data/
└── PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step1_trimmed.root
```

**Description:** Filtered pileup data with energy threshold applied.

---

#### Step E: Pileup Segmentation
**Purpose:** Digitise pileup events to detector cells

**Code:** `Segmentation/cellwise_segmentation.C`

**Input:**
```
PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step1_trimmed.root
```

**Output:**
```
PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
```

**Description:** Digitised pileup with cellwise segmentation.

---

#### Step F: Create Pileup File with nPU
**Purpose:** Generate files with a specific number of pileup events per event.

**Code:** `PileupUtils/createPileupFile.C`

**Input:**
```
PileUp_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
```

**Parameters:**
- nOutputEvents: Number of signal events to prepare
- nPU: Number of pileup events per signal (e.g., 70)

**Output:**
```
PileUp_nPU_070_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
PileUp_nPU_150_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
...
```

**Description:** Each file contains random combinations of nPU pileup events, ready to be added to signal events.

---

### COMBINATION STAGE

#### Step 4: Signal + Pileup Addition
**Purpose:** Combine signal with pileup to create realistic detector conditions

**Code:** `SignalToPileupAddition/addPileupToSignal.C`

**Inputs:**
```
Signal: Electron_Pt_025_Eta_170_Events_2K_PU_000_Set01_Step2.root
Pileup: PileUp_nPU_070_Pt_GT_pt3_Eta_15_31_Events_20K_Step2.root
```

**Output:**
```
Electron_Pt_025_Eta_170_Events_2K_PU_070_Set01_Combined.root
```

**Description:** 
- Each signal event is combined with a **random unique pileup** event from the nPU file
- Output contains η-φ tree with combined energy deposits
- **Ready for ML training:** Each event represents realistic detector conditions with signal + pileup

---

## File Naming Convention

### Signal Files
```
{Particle}_Pt_{Energy}_Eta_{EtaRange}_Events_{N}_PU_{nPU}_Set{XX}_Step{N}.root
```
- **Particle:** Electron, Positron, Photon, Muon
- **Energy:** Transverse momentum (e.g., 025 = 25 GeV)
- **EtaRange:** Pseudorapidity range (e.g., 170 = 1.70)
- **N:** Number of events
- **nPU:** Number of pileup events (000 for pure signal)
- **Step:** Processing stage (1=raw, 2=digitized)

### Pileup Files
```
PileUp_nPU_{XXX}_Pt_GT_pt{Y}_Eta_{A}_{B}_Events_{N}_Step{S}.root
```
- **nPU:** Number of pileup events per signal
- **Pt_GT_pt{Y}:** Minimum pT threshold (e.g., pt3 = 0.3 GeV)
- **Eta:** Pseudorapidity range (e.g., 15_31 = 1.5 to 3.1)
- **Step:** Processing stage

### Trimmed Files
- Add `_trimmed` suffix after Step1 to indicate filtered files
- Example: `..._Step1_trimmed.root`

---
