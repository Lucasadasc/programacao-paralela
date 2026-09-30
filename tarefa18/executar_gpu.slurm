#!/usr/bin/env bash
#SBATCH --job-name=vadd-gpu
#SBATCH --partition=gpu-8-v100  
#SBATCH --ntasks=1
#SBATCH --gres=gpu:1
#SBATCH --time=00:05:00
#SBATCH --output=saidasNpad/vadd-gpu-%j.out

set -euo pipefail

cd "${SLURM_SUBMIT_DIR:-.}"

# O compilador padrão do nó não necessariamente tem suporte a OpenMP offload.
# Estes módulos devem disponibilizar clang, libomptarget e o toolkit CUDA.
module load llvm
module load cuda

command -v clang >/dev/null || {
    echo "Erro: clang não foi encontrado após carregar os módulos llvm e cuda." >&2
    exit 1
}

clang -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda \
    -Xopenmp-target=nvptx64-nvidia-cuda --offload-arch=sm_70 \
    vadd_target.c -o vadd_target

export OMP_TARGET_OFFLOAD=MANDATORY
srun ./vadd_target
