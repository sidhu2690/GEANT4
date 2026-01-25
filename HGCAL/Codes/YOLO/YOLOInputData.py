import ROOT as rt
import numpy as np
import os
import argparse as arg

parser = arg.ArgumentParser(description='Data Pipeline')
parser.add_argument('--PU', dest='PU', type=str, default='00')
parser.add_argument('-i', '--particle', dest='particle', type=str, default='ele')
parser.add_argument('-n', '--number', dest='n_events', type=str, default='-1')
parser.add_argument('--in_dir', dest='in_dir', type=str, default='a')
parser.add_argument('--box', dest='box', type=str, default='n')
args = parser.parse_args()


PU = f'{int(args.PU):03d}'   

if args.particle == 'ele':
    particle = 'Electron'
    cls = '0'
elif args.particle == 'pos':
    particle = 'Positron'
    cls = '1'
elif args.particle == 'muon':
    particle = 'Mu_Minus'
    cls = '0'
elif args.particle == 'muplus':
    particle = 'Mu_Plus'
    cls = '1'
else:
    print('Wrong particle')
    exit()

if args.box not in ['s','n','l']:
    print('Wrong box size')
    exit()

box_names = {'s':'Small','n':'Nominal','l':'Large'}
box_name = box_names[args.box]

in_dir = args.in_dir.rstrip('/')

root_file = None
for f in os.listdir(in_dir):
    if f.endswith('.root'):
        root_file = f
        break

if root_file is None:
    print("No ROOT file found")
    exit()

out_dir = f'Output/Image/Regular/{particle}/PU_{PU}'
label_out_dir = f'Output/Label/Regular/{particle}/PU_{PU}/{box_name}/labels'

for s in ['Training','Validation','Testing']:
    os.makedirs(f"{out_dir}/{s}", exist_ok=True)
    os.makedirs(f"{label_out_dir}/{s}", exist_ok=True)

dim1, dim2 = 736, 736

infile = rt.TFile(f"{in_dir}/{root_file}")

hitTree = infile.Get('Eta_Phi_CellWiseSegmentation')
labelTree = infile.Get('YOLOLabels')

t_events = labelTree.GetEntriesFast()

if int(args.n_events) <= 0:
    n_events = t_events
elif int(args.n_events) > t_events:
    print("Too many events selected")
    exit()
else:
    n_events = int(args.n_events)

print("Total events:", t_events)
print("Selected events:", n_events)


# Preload labels 

labels = {}

for i in range(n_events):
    labelTree.GetEntry(i)

    labels[i] = {
        "cls": str(labelTree.class_label),
        "ieta_seed": labelTree.ieta_seed,
        "iphi_seed": labelTree.iphi_seed,
        "f90_eta": labelTree.f90_eta,
        "f95_eta": labelTree.f95_eta,
        "f98_eta": labelTree.f98_eta,
        "f90_phi": labelTree.f90_phi,
        "f95_phi": labelTree.f95_phi,
        "f98_phi": labelTree.f98_phi
    }

# Loop hits ONCE

n_hits = hitTree.GetEntries()

current_event = -1
img = None

def save_event(i, img):

    img = np.clip(img, 0, 255)

    img2 = np.clip(
        img[:, :, ::3] + img[:, :, 1::3] + img[:, :, 2::3],
        0, 255
    )

    img4 = img2.astype(np.uint8)

    if i < 6000:
        split = "Training"
    elif i < 8000:
        split = "Validation"
    else:
        split = "Testing"

    np.save(
        f"{out_dir}/{split}/YOLO_Images_Regular_{particle}_PU_{PU}_Event_{i:05d}.npy",
        img4
    )

    lab = labels[i]

    center_phi = lab["iphi_seed"] / dim2
    center_eta = lab["ieta_seed"] / dim1

    if args.box == 's':
        width  = (2*lab["f90_eta"] + 1) / dim1
        height = (2*lab["f90_phi"] + 1) / dim2
    elif args.box == 'n':
        width  = (2*lab["f95_eta"] + 1) / dim1
        height = (2*lab["f95_phi"] + 1) / dim2
    else:
        width  = (2*lab["f98_eta"] + 1) / dim1
        height = (2*lab["f98_phi"] + 1) / dim2

    label_path = f"{label_out_dir}/{split}/YOLO_Images_Regular_{particle}_PU_{PU}_Event_{i:05d}.txt"

    with open(label_path,'w') as f:
        f.write(
            f'{cls} {center_phi} {center_eta} {width} {height}\n'
        )

# Main loop

for j in range(n_hits):

    hitTree.GetEntry(j)

    evt = hitTree.event_id

    if evt >= n_events:
        break

    if evt != current_event:

        if current_event >= 0:
            save_event(current_event, img)

            if current_event % 1 == 0:
                print("Processed:", current_event)

        img = np.zeros((dim1, dim2, 48), dtype=np.float32)
        current_event = evt

    ieta  = hitTree.ieta
    iphi  = hitTree.iphi
    layer = hitTree.layer
    ADC   = hitTree.ADC

    if 0 <= ieta < dim1 and 0 <= iphi < dim2 and 1 <= layer <= 47:
        img[ieta, iphi, layer-1] += ADC

# Save last event
if current_event >= 0 and current_event < n_events:
    save_event(current_event, img)

print("Done ✅")
