#!/bin/bash

JOBDIR="/persistent/data1/ms21080/Regression/Data/electron"
INDIR="/persistent/data1/ms21080/Data/Signal_PileUp_Data/Electron_Var_Pileup"
OUTBASE="/persistent/data1/ms21080/Regression/Data/Electron/100"

cd "$JOBDIR"

for SET in 01 02 03 04 05; do

cat > ele_var_set${SET}.sh << ENDSCRIPT
#!/bin/bash
#PBS -N lowpt_s${SET}
#PBS -q default
#PBS -l nodes=gpc10:ppn=1
#PBS -j oe

cd ${JOBDIR}

LOGFILE="pos_s${SET}.log"
exec > >(tee -a "\$LOGFILE") 2>&1

echo "Processing Set${SET} with FLOAT32 LINEAR scaling and VARIABLE 98% box..."
echo "Running on \$(hostname)"
echo "Start time: \$(date)"

source ~/.bashrc
conda activate /apps/tools/root

export PYTHONUNBUFFERED=1

python -u << 'PYEOF'
from process_root import process_root_file

IN_FILE = "${INDIR}/Electron_nPU_100_Pt_015_to_250_Eta_16_29_Events_2K_Set${SET}_Step2.root"
OUT_DIR = "${OUTBASE}"

print(f"Input  : {IN_FILE}")
print(f"Output : {OUT_DIR}")

process_root_file(
    in_file=IN_FILE,
    prefix="ele_var_set${SET}",
    start_index=0,
    n_events=-1,
    val_fraction=0.1,
    test_fraction=0.0,
    box_size='l',         # 'l' -> f98 (98% energy containment)
    base_dir=OUT_DIR,
    fixed_box=True,
    seed=41
)
PYEOF

ENDSCRIPT

    qsub ele_var_set${SET}.sh

done
