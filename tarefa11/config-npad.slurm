#!/usr/bin/env bash
#SBATCH --job-name=tarefa12
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --hint=nomultithread
#SBATCH --exclusive
#SBATCH --mem=8G
#SBATCH --time=02:00:00
#SBATCH --output=tarefa12-%j.out

set -euo pipefail

# Entra na pasta de submissão
cd "${SLURM_SUBMIT_DIR:-.}"

# Configuração de runtime OpenMP
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-$SLURM_CPUS_PER_TASK}"
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_DYNAMIC=FALSE

# Arquivo fonte e parâmetros
SRC="${1:?Informe o arquivo .c como primeiro argumento (ex: v4_numa.c)}"
shift  # Remove o nome do arquivo dos argumentos; o restante ($@) é repassado ao executável

# Nome do binário temporário baseado no nome do fonte
BIN="./${SRC%.c}.out"

# Compilação padrão C11 com OpenMP e -O2
gcc -std=c11 -O2 -fopenmp -ffp-contract=off "$SRC" -lm -o "$BIN"

# Execução única via srun com amarração estrita aos núcleos
srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --cpu-bind=cores "$BIN" "$@"
