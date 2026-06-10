#!/bin/bash

JOBDIR="/persistent/data1/ms21080/Regression/Data/Fix/any"
INDIR="/persistent/data1/ms21080/Data/Signal_PileUp_Data/Electron_Fix_Pileup"

cd "$JOBDIR"

PU=200
ETA=220

for PT in 025 050 100 150 200; do
    OUTBASE="/persistent/data1/ms21080/Regression/Data/Electron/Testing/PT${PT}"
    INFILE="${INDIR}/Electron_nPU_${PU}_Pt_${PT}_Eta_${ETA}_Events_2K_Set01_Step2.root"
    JOBNAME="ele_PU${PU}_PT${PT}_ETA${ETA}"

    # Check if input file exists
    if [ ! -f "$INFILE" ]; then
        echo "SKIPPING (file not found): $INFILE"
        continue
    fi

    cat > ${JOBNAME}.sh << ENDSCRIPT
#!/bin/bash
#PBS -N ${JOBNAME}
#PBS -q default
#PBS -l nodes=gpc10:ppn=1
#PBS -j oe

cd ${JOBDIR}

LOGFILE="${JOBNAME}.log"
exec > >(tee -a "\$LOGFILE") 2>&1

echo "Processing PU=${PU} PT=${PT} ETA=${ETA}"
echo "Running on \$(hostname)"
echo "Start time: \$(date)"

source ~/.bashrc
conda activate /apps/tools/root

export PYTHONUNBUFFERED=1

python -u << 'PYEOF'
from process_root import process_root_file

IN_FILE = "${INFILE}"
OUT_DIR = "${OUTBASE}"

print(f"Input  : {IN_FILE}")
print(f"Output : {OUT_DIR}")

process_root_file(
    in_file=IN_FILE,
    prefix="ele_PU${PU}_PT${PT}_ETA${ETA}",
    start_index=0,
    n_events=-1,
    val_fraction=0.0,
    test_fraction=0.0,
    box_size='l',
    base_dir=OUT_DIR,
    fixed_box=True,
    seed=42
)
PYEOF

echo "End time: \$(date)"
ENDSCRIPT

    qsub ${JOBNAME}.sh
    echo "Submitted: ${JOBNAME}"
    sleep 0.5

done

echo "All jobs submitted."
