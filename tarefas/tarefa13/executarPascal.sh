#!/usr/bin/env bash
# Executar dentro do step Slurm, com o mesmo cpuset para todas as politicas.
set -euo pipefail
cd "$(dirname "$0")"
resultado=${1:?Informe o diretorio de saida dos JSONs}
mkdir -p "$resultado"
command -v pascalanalyzer >/dev/null || {
    printf 'PaScal Analyzer indisponivel. Carregue o modulo ou PASCAL_ENV.\n' >&2
    exit 1
}
export OMP_PLACES=cores OMP_DYNAMIC=FALSE
unset OMP_NUM_THREADS OMP_THREAD_LIMIT GOMP_CPU_AFFINITY KMP_AFFINITY
unset OMP_DISPLAY_ENV OMP_DISPLAY_AFFINITY
cores=${CORES_PASCAL:-1,4,8,16,32}
passos=${PASSOS_PASCAL:-300}
repeticoes=${REPETICOES_PASCAL:-5}
# Mesmas quatro cargas da tarefa 12, expressas em pontos internos.
entradas="65536 $passos,262144 $passos,1048576 $passos,4194304 $passos"
pascalanalyzer -h > "$resultado/pascal-ajuda.txt"
for politica in false true close spread master; do
    export OMP_PROC_BIND="$politica"
    # O Analyzer configura OMP_NUM_THREADS em cada execucao.
    pascalanalyzer ./navierPascal -t man -c "$cores" -i "$entradas" \
        -r "$repeticoes" -o "$resultado/afinidade-${politica}.json"
    # Preservar as medidas sem publicar o hostname interno.
    sed -i -E 's/("hostname"[[:space:]]*:[[:space:]]*)"[^"]*"/\1"host-anonimizado"/g' "$resultado/afinidade-${politica}.json"
done
