#!/usr/bin/env bash
# Envie um experimento por vez. Exemplo:
# sbatch --ntasks=4 executarExperimento.slurm escalabilidade 1000000 1000 0.24 6
# O primeiro resultado de cada programa deve ser descartado (aquecimento).
#SBATCH --job-name=calor-mpi
#SBATCH --time=00:30:00
#SBATCH --output=calor-mpi-%j.out
#SBATCH --error=calor-mpi-%j.err

set -euo pipefail

module load openmpi

experimento="${1:-carga-grande}"
celulas="${2:-1000000}"
passos="${3:-1000}"
alpha="${4:-0.24}"
repeticoes="${5:-6}"
processos="${SLURM_NTASKS}"
arquivo="resultados-${experimento}-np${processos}.csv"

make clean
make

echo 'experimento,processos,celulas,passos,alpha,programa,repeticao,tempo_maximo,tempo_medio' > "${arquivo}"

for programa in calorBloqueante calorNaoBloqueante calorComSobreposicao; do
    for repeticao in $(seq 1 "${repeticoes}"); do
        saida=$(srun --ntasks="${processos}" "./${programa}" "${celulas}" "${passos}" "${alpha}")
        echo "${saida}"
        tempo_maximo=$(awk -F 'tempo_maximo = | s \\| tempo_medio = ' '/tempo_maximo/ {print $2}' <<< "${saida}")
        tempo_medio=$(awk -F 'tempo_medio = | s \\| T' '/tempo_medio/ {print $2}' <<< "${saida}")
        echo "${experimento},${processos},${celulas},${passos},${alpha},${programa},${repeticao},${tempo_maximo},${tempo_medio}" >> "${arquivo}"
    done
done

echo "Resultados gravados em ${arquivo}"
