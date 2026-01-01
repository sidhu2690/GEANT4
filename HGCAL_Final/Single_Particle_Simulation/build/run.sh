#! /bin/bash
#PBS -N g
#PBS -l nodes=2:ppn=2
#PBS -q long
#PBS -o outphyjobMu.log
#PBS -e errphyjobMu.log

# Record start time
START_TIME=$(date +%s)
echo "Script started at: $(date)"

cd $PBS_O_WORKDIR

# Change to the directory where you want to store the copied file
#cd /path/to/destination/directory

# Use sshpass with scp to copy the file from the remote location
module load codes/geant4/11.1

export G4ENSDFSTATEDATA=/gscratch/apps/root/geant4/install/share/Geant4/data/G4ENSDFSTATE2.3
export G4LEVELGAMMADATA=/gscratch/apps/root/geant4/install/share/Geant4/data/PhotonEvaporation5.7
export G4LEDATA=/gscratch/apps/root/geant4/install/share/Geant4/data/G4EMLOW8.2
export G4PARTICLEXSDATA=/gscratch/apps/root/geant4/install/share/Geant4/data/G4PARTICLEXS4.0

cmake -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) ../
make
./sim run.mac &> "$PBS_O_WORKDIR/job_electron_run0.log"

# Record end time and calculate duration
END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

# Convert to hours, minutes, seconds
HOURS=$((DURATION / 3600))
MINUTES=$(((DURATION % 3600) / 60))
SECONDS=$((DURATION % 60))

echo "Script ended at: $(date)"
echo "Total execution time: ${HOURS}h ${MINUTES}m ${SECONDS}s (${DURATION} seconds)"

# Print a message indicating that the file has been copied
#echo "File copied successfully."
#whoami
