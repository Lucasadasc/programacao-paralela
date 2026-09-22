#!/usr/bin/env bash
#SBATCH --job-name=tarefa14-mpi
#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:10:00
#SBATCH --output=tarefa14-%j.out

set -euo pipefail
cd "${SLURM_SUBMIT_DIR:-.}"

make
srun --ntasks=2 ./ping_pong > resultados.csv
