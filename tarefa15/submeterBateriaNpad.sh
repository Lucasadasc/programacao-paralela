#!/usr/bin/env bash
# Submete 2, 4 e 8 nós, com um processo MPI por nó.
# Uso: ./submeterBateriaNpad.sh [repeticoes]
# Exemplo: ./submeterBateriaNpad.sh 6

set -euo pipefail

repeticoes="${1:-5}"
alpha="0.24"
passos="500"
celulas="100000 1000000 2000000 5000000 7000000 10000000"
nos_lista="2 4 8"

test "${repeticoes}" -ge 1
mkdir -p saidaNpad

for nos in ${nos_lista}; do
    echo "Submetendo: ${nos} nós e ${nos} processos MPI"
    sbatch --partition=intel-128 --nodes="${nos}" --ntasks="${nos}" --ntasks-per-node=1 \
        executarBateriaNpad.slurm "${repeticoes}" "${celulas}" "${passos}" "${alpha}"
done
