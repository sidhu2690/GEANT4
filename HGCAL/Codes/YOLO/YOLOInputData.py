import ROOT as rt
import numpy as np
import os


def create_directories(base_dir='data'):
    for split in ['train', 'val', 'test']:
        os.makedirs(f"{base_dir}/images/{split}", exist_ok=True)
        os.makedirs(f"{base_dir}/labels/{split}", exist_ok=True)
        os.makedirs(f"{base_dir}/energy/{split}", exist_ok=True)


def log_scale_adc(img, adc_max=1_000_000):
    scaled = 255.0 * np.log1p(np.maximum(img, 0)) / np.log1p(adc_max)
    return np.clip(scaled, 0, 255).astype(np.uint8)


def collapse_layers(img):
    h, w = img.shape[:2]
    out = np.zeros((h, w, 16), dtype=np.float32)
    out[:, :, :15] = img[:, :, :45].reshape(h, w, 15, 3).sum(axis=3)
    out[:, :, 15]  = img[:, :, 45:47].sum(axis=2)
    return out


def process_root_file(in_file, prefix='evt', start_index=0, n_events=-1,
                      val_fraction=0.1, test_fraction=0.1, box_size='n',
                      base_dir='data', adc_max=1_000_000, seed=42):

    dim1, dim2 = 736, 736
    create_directories(base_dir)

    print(f"Opening {in_file} …")
    tmp = rt.TFile.Open(in_file, "READ")
    t_events = tmp.Get('YOLOLabels').GetEntriesFast()
    tmp.Close()

    if n_events <= 0 or n_events > t_events:
        n_events = t_events

    n_train = int(n_events * (1.0 - val_fraction - test_fraction))
    n_val   = int(n_events * val_fraction)
    n_test  = n_events - n_train - n_val
    print(f"Events: {n_events}  ({n_train} train / {n_val} val / {n_test} test)")

    # Shuffled train/val/test split map
    shuffled = np.random.RandomState(seed).permutation(n_events)
    split_map = {}
    for i, idx in enumerate(shuffled):
        if i < n_train:
            split_map[idx] = "train"
        elif i < n_train + n_val:
            split_map[idx] = "val"
        else:
            split_map[idx] = "test"

    # Vectorised load of YOLOLabels
    print("Loading YOLOLabels …")
    lbl_rdf = rt.RDataFrame("YOLOLabels", in_file)
    lbl = lbl_rdf.AsNumpy(["event_id", "class_label",
                            "ieta_seed", "iphi_seed",
                            "f90_eta", "f95_eta", "f98_eta",
                            "f90_phi", "f95_phi", "f98_phi"])

    eta_key = {'s': 'f90_eta', 'n': 'f95_eta', 'l': 'f98_eta'}[box_size]
    phi_key = {'s': 'f90_phi', 'n': 'f95_phi', 'l': 'f98_phi'}[box_size]

    # Vectorised load of Signal_GeneratorInfo (energy regression)
    print("Loading Signal_GeneratorInfo …")
    gen_rdf = rt.RDataFrame("Signal_GeneratorInfo", in_file)
    gen = gen_rdf.AsNumpy(["event_id", "energy_MeV"])

    gen_lookup = {}
    for i in range(len(gen["event_id"])):
        eid = int(gen["event_id"][i])
        if eid < n_events:
            gen_lookup[eid] = i

    # Vectorised load of hits
    print("Loading hits via RDataFrame …")
    hit_rdf = rt.RDataFrame("Eta_Phi_CellWiseSegmentation", in_file)
    if n_events < t_events:
        hit_rdf = hit_rdf.Filter(f"event_id < {n_events}")

    cols = hit_rdf.AsNumpy(["event_id", "ieta", "iphi", "layer", "ADC"])
    h_evt   = cols["event_id"].astype(np.int32)
    h_ieta  = cols["ieta"].astype(np.int32)
    h_iphi  = cols["iphi"].astype(np.int32)
    h_layer = cols["layer"].astype(np.int32)
    h_adc   = cols["ADC"].astype(np.float32)
    del cols

    valid = ((h_ieta >= 0) & (h_ieta < dim1) &
             (h_iphi >= 0) & (h_iphi < dim2) &
             (h_layer >= 1) & (h_layer <= 47))
    h_evt   = h_evt[valid]
    h_ieta  = h_ieta[valid]
    h_iphi  = h_iphi[valid]
    h_layer = h_layer[valid] - 1
    h_adc   = h_adc[valid]
    del valid

    order = np.argsort(h_evt, kind='mergesort')
    h_evt   = h_evt[order]
    h_ieta  = h_ieta[order]
    h_iphi  = h_iphi[order]
    h_layer = h_layer[order]
    h_adc   = h_adc[order]
    del order

    bounds = np.searchsorted(h_evt, np.arange(n_events + 1))
    print(f"  {len(h_evt):,} valid hits loaded.")

    # Event loop
    print("Saving events …")
    for evt in range(n_events):

        split = split_map[evt]
        fname = f"{prefix}_{start_index + evt:05d}"

        # ── YOLO label (with clipping) ───────────────────────────
        cls      = int(lbl["class_label"][evt])
        x_center = float(lbl["iphi_seed"][evt]) / dim2
        y_center = float(lbl["ieta_seed"][evt]) / dim1
        f_eta    = float(lbl[eta_key][evt])
        f_phi    = float(lbl[phi_key][evt])
        width    = (2.0 * f_phi + 1.0) / dim2
        height   = (2.0 * f_eta + 1.0) / dim1

        # Clip box to stay within [0, 1]
        x1 = max(0.0, x_center - width / 2)
        y1 = max(0.0, y_center - height / 2)
        x2 = min(1.0, x_center + width / 2)
        y2 = min(1.0, y_center + height / 2)

        # Recompute clipped centre and size
        width    = x2 - x1
        height   = y2 - y1
        x_center = x1 + width / 2
        y_center = y1 + height / 2

        with open(f"{base_dir}/labels/{split}/{fname}.txt", 'w') as f:
            f.write(f"{cls} {x_center:.6f} {y_center:.6f} "
                    f"{width:.6f} {height:.6f}\n")

        # ── Energy regression target ─────────────────────────────
        if evt in gen_lookup:
            e_mev = float(gen["energy_MeV"][gen_lookup[evt]])
        else:
            print(f"  ⚠️  Event {evt} missing from Signal_GeneratorInfo — writing zero")
            e_mev = 0.0

        with open(f"{base_dir}/energy/{split}/{fname}.txt", 'w') as f:
            f.write(f"{e_mev:.6f}\n")

        # ── Build 736×736×47 image (vectorised) ──────────────────
        img = np.zeros((dim1, dim2, 47), dtype=np.float32)
        s, e = int(bounds[evt]), int(bounds[evt + 1])
        if s < e:
            np.add.at(img,
                      (h_ieta[s:e], h_iphi[s:e], h_layer[s:e]),
                      h_adc[s:e])

        img = collapse_layers(img)
        img = log_scale_adc(img, adc_max)
        np.save(f"{base_dir}/images/{split}/{fname}.npy", img)

        if (evt + 1) % 100 == 0:
            print(f"  {evt + 1} / {n_events}")

    # Cleanup
    del h_evt, h_ieta, h_iphi, h_layer, h_adc, bounds
    del lbl, gen, gen_lookup

    next_idx = start_index + n_events
    print(f"✅ Done!  Skipped {n_skipped} pathological events.")
    print(f"   Next start_index = {next_idx}")
    return next_idx


# ========================= USAGE =========================
#
# idx = 0
# idx = process_root_file(
#     "MuonM_nPU_035_Pt_015_to_250_Eta_16_29_Events_2K_Set01_Step2.root",
#     prefix="muon", start_index=idx, n_events=-1
# )
# idx = process_root_file("Electron_Set01.root",  prefix="ele",    start_index=idx)
# idx = process_root_file("Photon_Set01.root",    prefix="photon", start_index=idx)
# idx = process_root_file("Positron_Set01.root",  prefix="pos",    start_index=idx)
