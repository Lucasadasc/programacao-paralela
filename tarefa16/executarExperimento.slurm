#!/usr/bin/env bash
# Exemplo: sbatch --ntasks=4 executarExperimento.slurm
#SBATCH --job-name=matriz-vetor-mpi
#SBATCH --partition=intel-128
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:15:00
#SBATCH --output=matriz-vetor-%j.out
#SBATCH --error=matriz-vetor-%j.err

set -euo pipefail

test -n "${SLURM_NTASKS:-}"
command -v mpicc >/dev/null || { echo "mpicc nao esta disponivel no ambiente do NPAD" >&2; exit 1; }
command -v srun >/dev/null || { echo "srun nao esta disponivel no ambiente Slurm" >&2; exit 1; }
make

processos="${SLURM_NTASKS}"
arquivo="resultados-np${processos}.csv"

echo 'linhas,colunas,processos,repeticao,tempo_s' > "${arquivo}"

for tamanho in 1000 2000 4000; do
    for repeticao in 1 2 3 4 5 6; do
        saida=$(srun --ntasks="${processos}" ./matrizVetor "${tamanho}" "${tamanho}")
        echo "${saida}"
        tempo=$(awk -F 'tempo_s=' '{print $2}' <<< "${saida}" | awk '{print $1}')
        echo "${tamanho},${tamanho},${processos},${repeticao},${tempo}" >> "${arquivo}"
    done
done

echo "Resultados gravados em ${arquivo}"
