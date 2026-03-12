# HGCAL Simulation Pipeline

This repository contains a complete pipeline for High Granularity Calorimeter (HGCAL) simulation, from pileup generation through digitisation, signal–pileup mixing, and YOLO training data preparation.

## Directory Structure

```
Codes/
├── PileupUtils/
│   └── rootToText.C
│
├── Segmentation/
│   ├── cellwise_segmentation.C
│   ├── automation.C
│   └── job.sh
│
├── SignalToPileupAddition/
│   └── addPileupToSignal.C
│
└── YOLO/
    └── rootToNumpy.py
```

## Module Descriptions

### 1. PileupUtils

Utilities for generating and managing pileup events for simulation.

#### `rootToText.C`

Converts Pythia-generated pileup data stored in a ROOT file to a text format suitable for Geant4 simulations. The script reads particle information (`pdg_id`, `pT`, `eta`, `phi`) from the `gen` tree, filters particles based on kinematic cuts (`pT > 0.3`, `1.5 ≤ η ≤ 3.1`), and removes non-detectable or short-lived particles (such as quarks, gluons, and resonances).

For each event, the remaining stable particles are written to a structured text file containing the event number, track index, PDG ID, transverse momentum, azimuthal angle, polar angle (θ), and pseudorapidity (η). This text file is then used as particle input for Geant4 detector simulations.

---

### 2. Segmentation

Handles the digitisation of Geant4 simulation outputs.

#### `cellwise_segmentation.C`

Processes Geant4 simulation outputs and converts continuous energy deposits into discrete detector cells. The script reads the `ParticleTracking` tree, aggregates energy deposits (`edep`) per detector cell, and produces two segmented representations: **pixel-wise cells** based on spatial coordinates (x–y grid) and **η–φ cells** based on detector geometry.

For each event, hits falling into the same cell are combined by summing their energy deposits and corresponding ADC values. The resulting segmented data are written into two output trees: `Pixel_CellWiseSegmentation` and `Eta_Phi_CellWiseSegmentation`, while the original `GeneratorInfo` tree is preserved. This step effectively digitises the simulated detector hits into cell-level data suitable for further analysis or machine learning workflows.

#### `automation.C`

Automates the segmentation process through HPC.

#### `job.sh`

HPC job submission script for running segmentation tasks on the cluster.

---

### 3. SignalToPileupAddition

Combines signal and pileup events to simulate realistic detector conditions.

#### `addPileupToSignal.C`

Combines signal events with multiple pileup interactions to simulate realistic detector conditions. The script reads segmented η–φ hit data from both the signal and pileup ROOT files, randomly selects `nPU` pileup events for each signal event, and merges their detector hits by summing the energy deposits (`edep`) and ADC values for overlapping cells.

The output file contains several trees:
- **Signal_GeneratorInfo** and **Pileup_GeneratorInfo**: copies of the generator-level information from the input files.
- **Signal_Eta_Phi_CellWiseSegmentation** and **Pileup_Eta_Phi_CellWiseSegmentation**: original segmented hit data from the signal and pileup samples.
- **Eta_Phi_CellWiseSegmentation**: the combined detector hits after merging signal with `nPU` pileup events.
- **EventMapping**: records which pileup events were used for each signal event.
- **YOLOLabels**: stores object detection labels (seed position and energy containment variables such as f90, f95, f98) derived from the signal energy distribution.
- **validation_tree**: per-layer validation quantities such as number of active pixels (`NPix`) and energy statistics (`EMin`, `EMax`, `ETotal`).

This step produces realistic signal+pileup detector data and corresponding labels that can be used for downstream analysis or machine learning workflows.

---

### 4. YOLO Training Data Preparation

Converts combined ROOT simulation output into a machine-learning-ready dataset for training YOLO models.

#### `rootToNumpy.py`

Reads detector hits from the `Eta_Phi_CellWiseSegmentation` tree, YOLO bounding box labels from `YOLOLabels`, and energy targets from `Signal_GeneratorInfo`.

For each event, a **736 × 736 × 47 detector image** is constructed from ADC values stored in the ROOT file. These layers are then **collapsed into 16 channels** to reduce dimensionality while preserving detector depth information. The ADC values are scaled and compressed using a noise floor subtraction and logarithmic transformation to stabilise the dynamic range.

The script performs the following steps:

1. Splits events into **train / validation / test** datasets.
2. Generates **YOLO object detection labels** using the seed position and containment variables (`f90`, `f95`, `f98`) or a fixed bounding box.
3. Extracts **energy regression targets** from generator-level information.
4. Converts detector hits into **multi-channel image tensors** and stores them as `.npy` files.
5. Saves labels and energy targets as `.txt` files for each event.

The output directory structure contains:
- `images/` → detector images (`.npy`)
- `labels/` → YOLO bounding box labels
- `energy/` → regression targets (true particle energy)

---

## Workflow Overview

```
Pythia (pileup generation)
  │
  ▼
rootToText.C ──► Geant4 simulation
                      │
                      ▼
              cellwise_segmentation.C
               ┌──────┴──────┐
               ▼              ▼
         Signal (segmented)  Pileup (segmented)
               │              │
               └──────┬───────┘
                      ▼
            addPileupToSignal.C
                      │
                      ▼
             rootToNumpy.py
                      │
                      ▼
         YOLO-ready dataset (images / labels / energy)
```

## Requirements

- **ROOT** (CERN Data Analysis Framework)
- **Geant4** (detector simulation)
- **Pythia** (pileup generation)
- **Python 3** with `numpy`, `uproot` (or `ROOT` PyROOT bindings)
- HPC cluster access with a job scheduler (e.g. HTCondor, Slurm)
