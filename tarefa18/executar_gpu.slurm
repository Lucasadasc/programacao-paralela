#!/usr/bin/env bash
#SBATCH --job-name=vadd-gpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --gres=gpu:1
#SBATCH --time=00:05:00
#SBATCH --output=saidasNpad/vadd-gpu-%j.out

cd "${SLURM_SUBMIT_DIR:-.}"

# Carregue antes o modulo LLVM/Clang com suporte a OpenMP target/CUDA do NPAD.
clang -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda vadd_target.c -o vadd_target
export OMP_TARGET_OFFLOAD=MANDATORY
srun ./vadd_target
