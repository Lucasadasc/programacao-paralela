#!/usr/bin/env bash
# Argumentos: repeticoes celulas passos alpha
# Cada nó executa exatamente um processo MPI: nós = processos totais.
# submeterBateriaNpad.sh submete os casos de 2, 4 e 8 nós/processos.
#SBATCH --job-name=tarefa15-mpi
#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:10:00
#SBATCH --output=saidaNpad/tarefa15-%j.out
#SBATCH --error=saidaNpad/tarefa15-%j.err
#SBATCH --partition=intel-512

set -euo pipefail

repeticoes="${1:-5}"
celulas_lista="${2:-100000 1000000 2000000 5000000 7000000 10000000}"
passos="${3:-500}"
alpha="${4:-0.24}"

cd "${SLURM_SUBMIT_DIR:-.}"
origem="$PWD"
saida="${origem}/saidaNpad"
mkdir -p "${saida}"
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
arquivo_resumo="medianas-n${nos}-p${processos}-job${SLURM_JOB_ID}.csv"
caminho_resumo="${trabalho}/${arquivo_resumo}"

echo 'nos,processos,celulas,passos,alpha,programa,repeticao,tempo_maximo_s,tempo_medio_s' > "${caminho_resultado}"
echo 'nos,processos,celulas,passos,alpha,programa,amostras_uteis,mediana_tempo_maximo_s' > "${caminho_resumo}"
echo "Nos: ${nos}; processos totais: ${processos}; passos: ${passos}; celulas: ${celulas_lista}"

for celulas in ${celulas_lista}; do
    for programa in calorBloqueante calorNaoBloqueante calorComSobreposicao; do
        tempos_maximos=()
        for repeticao in $(seq 1 "${repeticoes}"); do
            saida="$(srun --ntasks="${processos}" "${trabalho}/${programa}" "${celulas}" "${passos}" "${alpha}")"
            printf '%s\n' "${saida}"
            tempo_maximo="$(sed -nE 's/.*tempo_maximo = ([0-9.]+) s.*/\1/p' <<< "${saida}")"
            tempo_medio="$(sed -nE 's/.*tempo_medio = ([0-9.]+) s.*/\1/p' <<< "${saida}")"
            test -n "${tempo_maximo}" && test -n "${tempo_medio}"
            echo "${nos},${processos},${celulas},${passos},${alpha},${programa},${repeticao},${tempo_maximo},${tempo_medio}" >> "${caminho_resultado}"
            tempos_maximos+=("${tempo_maximo}")
        done

        # A repetição 1 é aquecimento. A mediana é calculada sobre as demais.
        tempos_uteis=("${tempos_maximos[@]:1}")
        quantidade_uteis="${#tempos_uteis[@]}"
        test "${quantidade_uteis}" -ge 1
        mapfile -t tempos_ordenados < <(printf '%s\n' "${tempos_uteis[@]}" | sort -n)
        if (( quantidade_uteis % 2 == 1 )); then
            mediana="${tempos_ordenados[$((quantidade_uteis / 2))]}"
        else
            esquerda="${tempos_ordenados[$((quantidade_uteis / 2 - 1))]}"
            direita="${tempos_ordenados[$((quantidade_uteis / 2))]}"
            mediana="$(awk -v a="${esquerda}" -v b="${direita}" 'BEGIN { printf "%.6f", (a + b) / 2 }')"
        fi
        echo "${nos},${processos},${celulas},${passos},${alpha},${programa},${quantidade_uteis},${mediana}" >> "${caminho_resumo}"
    done
done

cp "${caminho_resultado}" "${saida}/"
cp "${caminho_resumo}" "${saida}/"
echo "Resultados brutos: ${saida}/${arquivo}"
echo "Medianas: ${saida}/${arquivo_resumo}"
