#!/usr/bin/env bash
#SBATCH --job-name=vadd-gpu
#SBATCH --partition=gpu-8-v100  
#SBATCH --ntasks=1
#SBATCH --gres=gpu:1
#SBATCH --time=00:05:00
#SBATCH --output=saidasNpad/vadd-gpu-%j.out

set -euo pipefail

cd "${SLURM_SUBMIT_DIR:-.}"

# O NVHPC fornece o compilador nvc e suporte a OpenMP offload para CUDA.
# --ignore_cache contorna o cache defeituoso do Lmod atualmente no NPAD.
module --ignore_cache load compilers/nvidia/nvhpc/24.11

command -v nvc >/dev/null || {
    echo "Erro: nvc não foi encontrado após carregar o módulo NVHPC." >&2
    exit 1
}

nvc -O3 -mp=gpu -gpu=cc70 vadd_target.c -o vadd_target

export OMP_TARGET_OFFLOAD=MANDATORY
srun ./vadd_target
