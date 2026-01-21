#!/bin/bash
#PBS -N pileup_addition
#PBS -l nodes=2:ppn=2
#PBS -q long
#PBS -o segmentation_out.log
#PBS -e segmentation_err.log

START_TIME=$(date +%s)
source ~/.bashrc

cd "$PBS_O_WORKDIR" || {
  echo "ERROR: PBS_O_WORKDIR not found"
  exit 1
}

conda activate /apps/tools/root

LOGFILE="elec_pileup_addition.log"
exec > >(tee -a "$LOGFILE") 2>&1

echo "========================================"
echo "Job started at: $(date)"
echo "Working directory: $(pwd)"
echo "========================================"

root -l -b -q "electron_var_pileup_automation.C"

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
HOURS=$((DURATION / 3600))
MINUTES=$(((DURATION % 3600) / 60))
SECONDS=$((DURATION % 60))

echo "========================================"
echo "Job ended at: $(date)"
echo "Total execution time: ${HOURS}h ${MINUTES}m ${SECONDS}s"
echo "========================================"
