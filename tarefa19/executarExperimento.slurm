#!/usr/bin/env bash
#SBATCH --job-name=heat-nsys
#SBATCH --partition=gpu-8-v100
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --gres=gpu:1
#SBATCH --mem=4G
#SBATCH --time=01:00:00
#SBATCH --output=heat-nsys-%j.out
#SBATCH --error=heat-nsys-%j.err

set -euo pipefail
cd "${SLURM_SUBMIT_DIR:?Submeta com sbatch na pasta tarefa19.}"
module --ignore_cache load "${NVHPC_MODULE:-compilers/nvidia/nvhpc/24.11}"
if [[ -n ${NSYS_MODULE:-} ]]; then
    module --ignore_cache load "$NSYS_MODULE"
fi
for ferramenta in nvc nsys srun; do
    command -v "$ferramenta" >/dev/null || {
        printf 'Erro: %s não encontrado. Confira os módulos.\n' "$ferramenta" >&2
        exit 1
    }
done

read -r -a tamanhos <<< "${TAMANHOS:-256 512 1024 2048 4096}"
passos=${PASSOS:-100}
if (( ${#tamanhos[@]} == 0 )); then
    printf 'Erro: informe pelo menos um tamanho.\n' >&2
    exit 1
fi
for valor in "$passos" "${tamanhos[@]}"; do
    [[ $valor =~ ^[1-9][0-9]*$ ]] || {
        printf 'Erro: tamanhos e passos devem ser inteiros positivos.\n' >&2
        exit 1
    }
done
for n in "${tamanhos[@]}"; do
    (( n <= 4096 )) || {
        printf 'Erro: tamanho máximo desta configuração: 4096.\n' >&2
        exit 1
    }
done

mkdir -p resultadosNpad
resultado=$(mktemp -d "resultadosNpad/heat-${SLURM_JOB_ID}-XXXXXX")
temporarios=$(mktemp -d /tmp/tarefa19-nsys-XXXXXX)
# Remove apenas os arquivos intermediários criados por este job.
trap 'rm -r -- "$temporarios"' EXIT
nvc -O3 -mp=gpu -gpu=cc70 -Minfo=mp heatTarget.c -lm -o "$temporarios/heatTarget"

export OMP_TARGET_OFFLOAD=MANDATORY
export OMP_NUM_THREADS=1
export LC_ALL=C
printf 'n,passos,mediana_solve_s,erro_l2,kernels_ms,copias_ms\n' > "$resultado/resumo.csv"

for n in "${tamanhos[@]}"; do
    : > "$temporarios/tempos.txt"
    for repeticao in 1 2 3; do
        srun "$temporarios/heatTarget" "$n" "$passos" > "$temporarios/execucao.txt"
        awk '/Solve time \(s\):/ { print $4; encontrado=1 }
             END { if (!encontrado) exit 1 }' "$temporarios/execucao.txt" >> "$temporarios/tempos.txt"
    done
    mediana=$(sort -n "$temporarios/tempos.txt" | sed -n '2p')
    erro=$(awk '/Error \(L2norm\):/ { print $3; encontrado=1 }
                END { if (!encontrado) exit 1 }' "$temporarios/execucao.txt")

    perfil="$temporarios/n${n}"
    srun nsys profile --trace=cuda --sample=none --cpuctxsw=none \
        --output="$perfil" "$temporarios/heatTarget" "$n" "$passos"
    for tipo in kern mem_time; do
        nsys stats --force-export=true --report="cuda_gpu_${tipo}_sum" --format=csv \
            "${perfil}.nsys-rep" > "$temporarios/${tipo}.csv"
    done
    # Coluna 2 dos resumos do nsys: tempo total em nanossegundos.
    kernels=$(awk -F, '$1 ~ /^[0-9]+([.][0-9]+)?$/ { soma += $2; encontrou=1 }
        END { if (!encontrou) exit 1; printf "%.3f", soma/1000000 }' "$temporarios/kern.csv")
    copias=$(awk -F, '$1 ~ /^[0-9]+([.][0-9]+)?$/ { soma += $2; encontrou=1 }
        END { if (!encontrou) exit 1; printf "%.3f", soma/1000000 }' "$temporarios/mem_time.csv")

    printf '%s,%s,%s,%s,%s,%s\n' "$n" "$passos" "$mediana" "$erro" "$kernels" "$copias" \
        >> "$resultado/resumo.csv"
    printf 'n=%s | solve=%s s | erro L2=%s | kernels=%s ms | cópias=%s ms\n' \
        "$n" "$mediana" "$erro" "$kernels" "$copias"
done
printf 'Resumo salvo em %s/resumo.csv\n' "$resultado"
