#!/usr/bin/env bash
# Argumentos: repeticoes celulas passos alpha
# Os valores abaixo formam um job mínimo (2 nós, 1 processo por nó).
# submeterBateriaNpad.sh os sobrescreve para a matriz de experimentos.
#SBATCH --job-name=tarefa15-mpi
#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:10:00
#SBATCH --output=tarefa15-%j.out
#SBATCH --error=tarefa15-%j.err
#SBATCH --partition=intel-512

set -euo pipefail

repeticoes="${1:-3}"
celulas_lista="${2:-100000 1000000 10000000}"
passos="${3:-500}"
alpha="${4:-0.24}"

cd "${SLURM_SUBMIT_DIR:-.}"
origem="$PWD"
nos="${SLURM_JOB_NUM_NODES}"
processos="${SLURM_NTASKS}"
diretorio_trabalho="calor-mpi-${SLURM_JOB_ID}"
trabalho="${SLURM_TMPDIR:-/tmp}/${diretorio_trabalho}"

# /tmp é local a cada nó. Prepara e compila uma cópia em todos os nós
# alocados; assim cada rank encontra seu executável localmente.
srun --nodes="${nos}" --ntasks="${nos}" --ntasks-per-node=1 bash -c '
    set -euo pipefail
    destino="${SLURM_TMPDIR:-/tmp}/calor-mpi-${SLURM_JOB_ID}"
    mkdir -p "${destino}"
    cp "${SLURM_SUBMIT_DIR}"/Makefile "${SLURM_SUBMIT_DIR}"/calorComum.h "${SLURM_SUBMIT_DIR}"/calor*.c "${destino}/"
    make -C "${destino}"
'
trap 'srun --nodes="${nos}" --ntasks="${nos}" --ntasks-per-node=1 rm -rf "${SLURM_TMPDIR:-/tmp}/${diretorio_trabalho}" || true' EXIT

arquivo="resultados-n${nos}-p${processos}-job${SLURM_JOB_ID}.csv"
caminho_resultado="${trabalho}/${arquivo}"

echo 'nos,processos,celulas,passos,alpha,programa,repeticao,tempo_maximo_s,tempo_medio_s' > "${caminho_resultado}"
echo "Nos: ${nos}; processos totais: ${processos}; passos: ${passos}; celulas: ${celulas_lista}"

for celulas in ${celulas_lista}; do
    for programa in calorBloqueante calorNaoBloqueante calorComSobreposicao; do
        for repeticao in $(seq 1 "${repeticoes}"); do
            saida="$(srun --ntasks="${processos}" "${trabalho}/${programa}" "${celulas}" "${passos}" "${alpha}")"
            printf '%s\n' "${saida}"
            tempo_maximo="$(sed -nE 's/.*tempo_maximo = ([0-9.]+) s.*/\1/p' <<< "${saida}")"
            tempo_medio="$(sed -nE 's/.*tempo_medio = ([0-9.]+) s.*/\1/p' <<< "${saida}")"
            test -n "${tempo_maximo}" && test -n "${tempo_medio}"
            echo "${nos},${processos},${celulas},${passos},${alpha},${programa},${repeticao},${tempo_maximo},${tempo_medio}" >> "${caminho_resultado}"
        done
    done
done

cp "${caminho_resultado}" "${origem}/"
echo "Resultados gravados em ${origem}/${arquivo}"
