#!/usr/bin/env bash
# Submete 2, 4 e 8 nós x 2, 4 e 8 processos por nó.
# Uso: ./submeterBateriaNpad.sh [repeticoes]
# Exemplo: ./submeterBateriaNpad.sh 6

set -euo pipefail

repeticoes="${1:-5}"
alpha="0.24"
passos="500"
celulas="100000 1000000 10000000"
nos_lista="2 4 8"
processos_por_no_lista="2 4 8"

test "${repeticoes}" -ge 1

for nos in ${nos_lista}; do
    for processos_por_no in ${processos_por_no_lista}; do
        echo "Submetendo: ${nos} nós x ${processos_por_no} processos/nó"
        sbatch --partition=intel-128 --nodes="${nos}" --ntasks-per-node="${processos_por_no}" \
            executarBateriaNpad.slurm "${repeticoes}" "${celulas}" "${passos}" "${alpha}"
    done
done
