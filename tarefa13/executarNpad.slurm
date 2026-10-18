#!/usr/bin/env bash
#SBATCH --job-name=tarefa13-afinidade
#SBATCH --partition=amd-512
#SBATCH --nodelist=r2n50
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --hint=nomultithread
#SBATCH --exclusive
#SBATCH --mem=8G
#SBATCH --time=04:00:00
#SBATCH --output=tarefa13-%j.out
#SBATCH --error=tarefa13-%j.err
set -euo pipefail
cd "${SLURM_SUBMIT_DIR:?Submeta de dentro de tarefa13}"
if [[ -n ${GCC_MODULE:-} ]]; then module load "$GCC_MODULE"; fi
if [[ -n ${PASCAL_ENV:-} ]]; then
    set +u
    source "$PASCAL_ENV"
    set -u
else
    module load softwares/pascalsuite/2025-07-08
fi
command -v pascalanalyzer >/dev/null
make all pascal
resultado="resultadosNpad/job-${SLURM_JOB_ID}"
# Um unico step preserva o conjunto de CPUs disponiveis para todas as politicas.
# Binding do Slurm desabilitado: o OpenMP controla a afinidade dentro do cpuset.
srun --ntasks=1 --cpus-per-task=32 --cpu-bind=none \
    python3 executarExperimentos.py --saida "$resultado"
srun --ntasks=1 --cpus-per-task=32 --cpu-bind=none \
    bash executarPascal.sh "$resultado/pascal"
lscpu > "$resultado/cpu.txt"
gcc --version > "$resultado/compilador.txt"
cp navierStokes.c navierStokes.h navierStokesV3.c afinidade.c Makefile \
    executarExperimentos.py executarNpad.slurm executarPascal.sh README.md "$resultado/"
sha256sum navierStokes afinidade navierPascal > "$resultado/binarios.sha256"
