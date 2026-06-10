import ROOT as rt
import numpy as np
import os


GLOBAL_ADC_MAX = 1_000_000.0   
NOISE_FLOOR    = 0.0


def create_directories(base_dir='data'):
    for split in ['train', 'val', 'test']:
        os.makedirs(f"{base_dir}/images/{split}", exist_ok=True)
        os.makedirs(f"{base_dir}/labels/{split}", exist_ok=True)
        os.makedirs(f"{base_dir}/energy/{split}", exist_ok=True)


def process_root_file(in_file, prefix='evt', start_index=0, n_events=-1,
                      val_fraction=0.1, test_fraction=0.1, box_size='l',
                      base_dir='data', fixed_box=False,
                      fixed_w=0.034249, fixed_h=0.107281, seed=42):

    dim1, dim2 = 736, 736

    eta_0 = 1.4
    eta_n = 3.1
    phi_0 = -3.1416
    phi_n =  3.1416

    create_directories(base_dir)

    print(f"Opening {in_file} ...")
    tmp = rt.TFile.Open(in_file, "READ")
    t_events = tmp.Get('YOLOLabels').GetEntriesFast()
    tmp.Close()

    if n_events <= 0 or n_events > t_events:
        n_events = t_events

    n_train = int(n_events * (1.0 - val_fraction - test_fraction))
    n_val   = int(n_events * val_fraction)
    n_test  = n_events - n_train - n_val
    print(f"Events: {n_events}  ({n_train} train / {n_val} val / {n_test} test)")
    print(f"Mode: FLOAT32 LINEAR  |  GLOBAL_ADC_MAX={GLOBAL_ADC_MAX}  |  NOISE_FLOOR={NOISE_FLOOR}")
    print(f"Box: {'FIXED (w={}, h={})'.format(fixed_w, fixed_h) if fixed_box else 'VARIABLE 98% (box_size={})'.format(box_size)}")

    shuffled = np.random.RandomState(seed).permutation(n_events)
    split_map = {}
    for i, idx in enumerate(shuffled):
        if i < n_train:
            split_map[idx] = "train"
        elif i < n_train + n_val:
            split_map[idx] = "val"
        else:
            split_map[idx] = "test"

    print("Loading YOLOLabels ...")
    lbl_rdf = rt.RDataFrame("YOLOLabels", in_file)
    lbl = lbl_rdf.AsNumpy(["event_id", "class_label",
                            "ieta_seed", "iphi_seed",
                            "f90_eta", "f95_eta", "f98_eta",
                            "f90_phi", "f95_phi", "f98_phi"])

    eta_key = {'s': 'f90_eta', 'n': 'f95_eta', 'l': 'f98_eta'}[box_size]
    phi_key = {'s': 'f90_phi', 'n': 'f95_phi', 'l': 'f98_phi'}[box_size]

    print("Loading Signal_GeneratorInfo ...")
    gen_rdf = rt.RDataFrame("Signal_GeneratorInfo", in_file)
    gen = gen_rdf.AsNumpy(["event_id", "energy_MeV", "eta", "phi"])

    gen_lookup = {}
    for i in range(len(gen["event_id"])):
        eid = int(gen["event_id"][i])
        if eid < n_events:
            gen_lookup[eid] = i

    print("Loading hits via RDataFrame ...")
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

    obs_max_raw = 0.0
    obs_max_after_collapse = 0.0

    print("Saving events ...")
    skipped = 0
    for evt in range(n_events):

        split = split_map[evt]
        fname = f"{prefix}_{start_index + evt:05d}"

        if evt not in gen_lookup:
            print(f"  Warning: Event {evt} missing from Signal_GeneratorInfo, skipping")
            skipped += 1
            continue

        eid = gen_lookup[evt]
        gen_eta_val = float(gen["eta"][eid])
        gen_phi_val = float(gen["phi"][eid])

        if gen_phi_val > np.pi:
            gen_phi_val = gen_phi_val - 2.0 * np.pi

        cls = int(lbl["class_label"][evt])

        x_center = (gen_phi_val - phi_0) / (phi_n - phi_0)
        y_center = (gen_eta_val - eta_0) / (eta_n - eta_0)

        if fixed_box:
            width  = fixed_w
            height = fixed_h
        else:
            f_eta  = float(lbl[eta_key][evt])
            f_phi  = float(lbl[phi_key][evt])
            width  = (2.0 * f_phi + 1.0) / dim2
            height = (2.0 * f_eta + 1.0) / dim1

        x1 = max(0.0, x_center - width / 2)
        y1 = max(0.0, y_center - height / 2)
        x2 = min(1.0, x_center + width / 2)
        y2 = min(1.0, y_center + height / 2)

        width    = x2 - x1
        height   = y2 - y1
        x_center = x1 + width / 2
        y_center = y1 + height / 2

        with open(f"{base_dir}/labels/{split}/{fname}.txt", 'w') as f:
            f.write(f"{cls} {x_center:.6f} {y_center:.6f} "
                    f"{width:.6f} {height:.6f}\n")

        e_mev = float(gen["energy_MeV"][eid])

        with open(f"{base_dir}/energy/{split}/{fname}.txt", 'w') as f:
            f.write(f"{e_mev:.6f}\n")

        img = np.zeros((dim1, dim2, 48), dtype=np.float32)
        s, e = int(bounds[evt]), int(bounds[evt + 1])
        if s < e:
            np.add.at(img,
                      (h_ieta[s:e], h_iphi[s:e], h_layer[s:e]),
                      h_adc[s:e])


        img = np.maximum(img - NOISE_FLOOR, 0.0)

        if img.size:
            obs_max_raw = max(obs_max_raw, float(img.max()))

        img = (img[:, :, ::3] +
               img[:, :, 1::3] +
               img[:, :, 2::3]).astype(np.float32)

        if img.size:
            obs_max_after_collapse = max(obs_max_after_collapse, float(img.max()))

        img = img / GLOBAL_ADC_MAX               
        img = np.clip(img, 0.0, 1.0).astype(np.float32)  

        np.save(f"{base_dir}/images/{split}/{fname}.npy", img)

        if (evt + 1) % 100 == 0:
            print(f"  {evt + 1} / {n_events}  "
                  f"max_raw_after_noise={obs_max_raw:.1f}  "
                  f"max_after_collapse={obs_max_after_collapse:.1f}")

    del h_evt, h_ieta, h_iphi, h_layer, h_adc, bounds
    del lbl, gen, gen_lookup

    print(f"\n  Skipped {skipped} events (missing gen info)")
    print(f"  Observed max (after noise sub, per-layer):  {obs_max_raw:.1f}")
    print(f"  Observed max (after 3-layer collapse):      {obs_max_after_collapse:.1f}")
    print(f"  GLOBAL_ADC_MAX used:                        {GLOBAL_ADC_MAX:.1f}")
    if obs_max_after_collapse > GLOBAL_ADC_MAX:
        print(f"  ⚠  WARNING: observed max > GLOBAL_ADC_MAX, values were clipped!")

    next_idx = start_index + n_events
    print(f"  Next start_index = {next_idx}")
    return next_idx
