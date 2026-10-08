#!/usr/bin/env bash
# Uso: ./executarExperimentoLocal.sh ["2 4 8"] ["2000 4000 8000"] [repeticoes]

set -euo pipefail

processosLista="${1:-1 2 4 8}"
tamanhos="${2:-512 1000 4000 8000 12000 16000 20000 24000}"
repeticoes="${3:-10}"

command -v mpicc >/dev/null || { echo "mpicc nao esta instalado" >&2; exit 1; }
command -v mpirun >/dev/null || { echo "mpirun nao esta instalado" >&2; exit 1; }

make
mkdir -p saidasLocais
arquivo="saidasLocais/resultados.csv"
echo "linhas,colunas,processos,repeticoes,mediana_tempo_s" > "${arquivo}"

for processos in ${processosLista}; do
    for tamanho in ${tamanhos}; do
        if (( tamanho % processos != 0 )); then
            echo "Ignorando M=${tamanho}: nao e divisivel por ${processos} processos."
            continue
        fi

        tempos=()
        for repeticao in $(seq 1 "${repeticoes}"); do
            saida=$(mpirun -np "${processos}" ./matrizVetor "${tamanho}" "${tamanho}")
            echo "${saida}"
            tempo=$(awk -F "tempo_s=" "{print \$2}" <<< "${saida}" | awk "{print \$1}")
            tempos+=("${tempo}")
        done

        mediana=$(printf "%s\n" "${tempos[@]}" | LC_ALL=C sort -n | awk -v n="${repeticoes}" \
            "NR == (n + 1) / 2 {esquerda = \$1} NR == n / 2 + 1 {print (esquerda + \$1) / 2}")
        echo "${tamanho},${tamanho},${processos},${repeticoes},${mediana}" >> "${arquivo}"
    done
done

echo "Medianas gravadas em ${arquivo}"
