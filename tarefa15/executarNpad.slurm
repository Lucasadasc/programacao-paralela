#!/usr/bin/env bash
#SBATCH --job-name=calor-mpi
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --time=00:10:00
#SBATCH --output=calor-mpi-%j.out
#SBATCH --error=calor-mpi-%j.err

set -euo pipefail

# Ajuste o módulo caso o NPAD use outro nome/versão de MPI.
module load openmpi

processos="${1:-${SLURM_NTASKS}}"
celulas="${2:-1000000}"
passos="${3:-1000}"
alpha="${4:-0.24}"

make clean
make

echo "Processos: ${processos}; células: ${celulas}; passos: ${passos}; alpha: ${alpha}"

srun --ntasks="${processos}" ./calorBloqueante "${celulas}" "${passos}" "${alpha}"
srun --ntasks="${processos}" ./calorNaoBloqueante "${celulas}" "${passos}" "${alpha}"
srun --ntasks="${processos}" ./calorComSobreposicao "${celulas}" "${passos}" "${alpha}"
