#!/usr/bin/env bash
#SBATCH --job-name=vadd-cpu
#SBATCH --partition=gpu-8-v100  
#SBATCH --ntasks=1
#SBATCH --time=00:05:00
#SBATCH --output=saidasNpad/vadd-cpu-%j.out

set -euo pipefail

cd "${SLURM_SUBMIT_DIR:-.}"

gcc -O3 -fopenmp vadd_par.c -o vadd_par
export OMP_NUM_THREADS="$SLURM_CPUS_PER_TASK"
srun ./vadd_par
