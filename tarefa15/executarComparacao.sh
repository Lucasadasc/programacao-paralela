#!/usr/bin/env bash
set -euo pipefail
quantidadeProcessos=${1:-4}
quantidadeCelulas=${2:-1000000}
quantidadePassos=${3:-1000}
fatorDifusao=${4:-0.24}
make
for programa in calorBloqueante calorNaoBloqueante calorComSobreposicao; do
  echo "--- $programa ---"
  mpirun -np "$quantidadeProcessos" "./$programa" "$quantidadeCelulas" "$quantidadePassos" "$fatorDifusao"
done
