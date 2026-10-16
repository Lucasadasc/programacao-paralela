#!/usr/bin/env bash
#SBATCH --job-name=tarefa12-pascal
#SBATCH --partition=amd-512
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --hint=nomultithread
#SBATCH --exclusive
#SBATCH --mem=8G
#SBATCH --time=02:00:00
#SBATCH --output=tarefa12-pascal-%j.out
#SBATCH --error=tarefa12-pascal-%j.err
set -euo pipefail
cd "${SLURM_SUBMIT_DIR:?Submeta na pasta tarefa12}"
if [[ -n ${GCC_MODULE:-} ]]; then module load "$GCC_MODULE"; fi
# Ex.: sbatch --export=ALL,PASCAL_ENV=/caminho/pascal-releases-master/env.sh executarPascal.slurm
if [[ -n ${PASCAL_ENV:-} ]]; then
    if [[ ! -r "$PASCAL_ENV" ]]; then
        printf 'PASCAL_ENV nao aponta para um arquivo legivel: %s\n' "$PASCAL_ENV" >&2
        exit 1
    fi
    # Alguns env.sh de ferramentas referenciam variaveis ainda nao definidas.
    set +u
    source "$PASCAL_ENV"
    set -u
fi
command -v pascalanalyzer >/dev/null || {
    cat >&2 <<'EOF'
PaScal Analyzer indisponivel: pascalanalyzer nao foi encontrado no PATH do job.
No terminal do NPAD, localize o env.sh da instalacao do PaScal e submeta com:
  sbatch --export=ALL,PASCAL_ENV=/caminho/real/do/PaScal/env.sh executarPascal.slurm
Se o PaScal for disponibilizado por modulo, carregue-o antes de submeter.
Definir PASCAL_ENV nao instala a ferramenta; o arquivo deve existir no no de execucao.
EOF
    exit 1
}
export OMP_PLACES=cores OMP_PROC_BIND=spread OMP_DYNAMIC=FALSE
# Nao exportar OMP_NUM_THREADS fixo: o Analyzer controla cada configuracao.
make validar
make pascal
resultado="resultadosNpad/pascal-${SLURM_JOB_ID}"
mkdir -p "$resultado"
cp navierStokes.c navierStokes.h navierStokesV{1,2,3}.c Makefile executarPascal.slurm "$resultado/"
pascalanalyzer -h > "$resultado/pascal-ajuda.txt"
gcc --version > "$resultado/compilador.txt"
lscpu > "$resultado/cpu.txt"
env | sort | sed -n '/^OMP_/p; /^SLURM_/p' > "$resultado/ambiente.txt"
cores=${CORES_PASCAL:-1,4,16}
passos=${PASSOS:-300}
repeticoes=${REPETICOES:-5}
# Pontos internos: 256^2, 512^2, 1024^2, 2048^2. Razao de trabalho: 4.
# Cada item de -i e a cadeia completa de argumentos para uma execucao.
entradas="65536 $passos,262144 $passos,1048576 $passos,4194304 $passos"
for versao in 1 2 3; do
    srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --cpu-bind=cores \
        pascalanalyzer "./navierPascal${versao}" -t man \
        -c "$cores" -i "$entradas" -r "$repeticoes" \
        -o "$resultado/versao${versao}.json"
done
