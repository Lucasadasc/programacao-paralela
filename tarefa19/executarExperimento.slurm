#!/usr/bin/env bash
#SBATCH --job-name=heat-nsys
#SBATCH --partition=gpu-8-v100
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --gres=gpu:1
#SBATCH --mem=4G
#SBATCH --time=00:30:00
#SBATCH --output=heat-nsys-%j.out
#SBATCH --error=heat-nsys-%j.err

set -euo pipefail
cd "${SLURM_SUBMIT_DIR:?Submeta este script com sbatch na pasta tarefa19.}"

module --ignore_cache load "${NVHPC_MODULE:-compilers/nvidia/nvhpc/24.11}"
# Se o nsys tiver um módulo próprio no cluster, informe seu nome na submissão.
if [[ -n ${NSYS_MODULE:-} ]]; then
    module --ignore_cache load "$NSYS_MODULE"
fi
for ferramenta in nvc nsys srun; do
    if ! command -v "$ferramenta" >/dev/null; then
        printf 'Erro: %s não está no PATH. Confira os módulos do NPAD.\n' "$ferramenta" >&2
        exit 1
    fi
done

read -r -a tamanhos <<< "${TAMANHOS:-256 512 1024 2048 4096}"
passos=${PASSOS:-100}
repeticoes=${REPETICOES:-3}
for valor in "$passos" "$repeticoes" "${tamanhos[@]}"; do
    if [[ ! $valor =~ ^[1-9][0-9]*$ ]]; then
        printf 'Erro: tamanhos, passos e repetições devem ser inteiros positivos.\n' >&2
        exit 1
    fi
done
if (( ${#tamanhos[@]} == 0 )); then
    printf 'Erro: informe pelo menos um tamanho de malha.\n' >&2
    exit 1
fi
# O código-base usa índices int. Esta bateria limita tamanho e memória.
for n in "${tamanhos[@]}"; do
    if (( n > 4096 )); then
        printf 'Erro: esta configuração de recursos aceita malhas até 4096.\n' >&2
        exit 1
    fi
done

mkdir -p resultadosNpad
resultado=$(mktemp -d "resultadosNpad/heat-${SLURM_JOB_ID}-XXXXXX")
printf 'Resultados: %s\n' "$resultado"
{
    date --iso-8601=seconds
    hostname
    module list
    nvc --version
    nsys --version
    if command -v nvidia-smi >/dev/null; then
        nvidia-smi
    fi
    printf 'Tamanhos: %s\nPassos: %s\nRepetições: %s\n' "${tamanhos[*]}" "$passos" "$repeticoes"
} > "$resultado/ambiente.txt" 2>&1

cp heatTarget.c "$resultado/heatTarget.c"
nvc -O3 -mp=gpu -gpu=cc70,lineinfo -Minfo=mp \
    "$resultado/heatTarget.c" -lm -o "$resultado/heatTarget" \
    > "$resultado/compilacao.txt" 2>&1

export OMP_TARGET_OFFLOAD=MANDATORY
export OMP_NUM_THREADS=1
printf 'n,celulas,passos,repeticao,solve_s,total_s,erro_l2\n' > "$resultado/tempos.csv"
relatorios=cuda_api_sum,cuda_gpu_kern_sum,cuda_gpu_mem_time_sum,cuda_gpu_mem_size_sum

for n in "${tamanhos[@]}"; do
    printf 'Executando malha %s x %s, %s passos\n' "$n" "$n" "$passos"
    # Aquecimento separado; cada execução ainda inicializa seu próprio contexto CUDA.
    srun "$resultado/heatTarget" "$n" "$passos" > "$resultado/n${n}_aquecimento.txt"
    for (( repeticao=1; repeticao<=repeticoes; repeticao++ )); do
        saida="$resultado/n${n}_r${repeticao}.txt"
        # Tempos para comparação são coletados sem o overhead do profiler.
        srun "$resultado/heatTarget" "$n" "$passos" > "$saida"
        awk -v n="$n" -v passos="$passos" -v repeticao="$repeticao" '
            /Error \(L2norm\):/ { erro=$3 }
            /Solve time \(s\):/ { solve=$4 }
            /Total time \(s\):/ { total=$4 }
            END {
                if (erro == "" || solve == "" || total == "") exit 1
                printf "%d,%d,%d,%d,%s,%s,%s\n", n,n*n,passos,repeticao,solve,total,erro
            }
        ' "$saida" >> "$resultado/tempos.csv"
    done

    perfil="$resultado/n${n}_perfil"
    srun nsys profile --trace=cuda --sample=none --cpuctxsw=none \
        --output="$perfil" "$resultado/heatTarget" "$n" "$passos" \
        > "${perfil}_execucao.txt" 2>&1
    nsys stats --report="$relatorios" --format=column \
        "${perfil}.nsys-rep" > "${perfil}_stats.txt" 2>&1
    nsys stats --report="$relatorios" --format=csv --output="${perfil}" \
        "${perfil}.nsys-rep" > "${perfil}_exportacao.txt" 2>&1
done

printf 'Experimento concluído. Tempos: %s/tempos.csv\n' "$resultado"
