#!/usr/bin/env bash
# Uso: ./executarExperimentoLocal.sh ["2 4 8"] ["2000 4000 8000"] [repeticoes]

set -euo pipefail

processosLista="${1:-1 2 4 8}"
tamanhos="${2:-512 1000 4000 8000 12000 16000 20000 24000}"
repeticoes="${3:-10}"

command -v mpicc >/dev/null || { echo "mpicc nao esta instalado" >&2; exit 1; }
command -v mpirun >/dev/null || { echo "mpirun nao esta instalado" >&2; exit 1; }
[[ "${repeticoes}" =~ ^[1-9][0-9]*$ ]] || {
    echo "O numero de repeticoes deve ser um inteiro positivo." >&2
    exit 1
}

make
mkdir -p saidasLocais
arquivo="saidasLocais/resultados.csv"
echo "linhas,colunas,processos,repeticoes,mediana_tempo_s" > "${arquivo}"

for processos in ${processosLista}; do
    [[ "${processos}" =~ ^[1-9][0-9]*$ ]] || {
        echo "Quantidade de processos invalida: ${processos}" >&2
        exit 1
    }

    for tamanho in ${tamanhos}; do
        [[ "${tamanho}" =~ ^[1-9][0-9]*$ ]] || {
            echo "Tamanho invalido: ${tamanho}" >&2
            exit 1
        }

        # Nesta tarefa, a matriz e dividida por colunas: N deve ser divisivel por P.
        if (( tamanho % processos != 0 )); then
            echo "Ignorando N=${tamanho}: nao e divisivel por ${processos} processos."
            continue
        fi

        tempos=()
        for repeticao in $(seq 1 "${repeticoes}"); do
            saida=$(mpirun -np "${processos}" ./matrizVetor "${tamanho}" "${tamanho}")
            echo "${saida}"
            tempo=$(awk -F 'tempo_s=' '{print $2}' <<< "${saida}" | awk '{print $1}')
            [[ -n "${tempo}" ]] || {
                echo "Nao foi possivel obter tempo_s da execucao." >&2
                exit 1
            }
            tempos+=("${tempo}")
        done

        mediana=$(printf '%s\n' "${tempos[@]}" | LC_ALL=C sort -n | awk -v n="${repeticoes}" \
            '{ valores[NR] = $1 } END {
                if (n % 2 == 1) print valores[(n + 1) / 2]
                else print (valores[n / 2] + valores[n / 2 + 1]) / 2
            }')
        echo "${tamanho},${tamanho},${processos},${repeticoes},${mediana}" >> "${arquivo}"
    done
done

echo "Medianas gravadas em ${arquivo}"
