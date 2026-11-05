# Programação Paralela — UFRN

Atividades da disciplina de **Programação Paralela** da Universidade Federal do Rio Grande do Norte (UFRN). O repositório reúne programas em C, scripts de experimentação e resultados de estudos sobre desempenho sequencial, OpenMP, MPI e processamento em GPU.

Os experimentos exploram localidade de memória, sincronização, distribuição de trabalho, comunicação entre processos e transferência de dados entre CPU e GPU. Há scripts Slurm para execução no NPAD.

## Organização

Cada diretório `tarefaN/` contém os fontes e, conforme a atividade, Makefile, scripts, documentação e resultados.

| Tarefa | Tema |
| --- | --- |
| [1](tarefa1/) | Aproximação de π pela série de Leibniz e erro numérico. |
| [2](tarefa2/) | Acesso a matrizes por linhas e colunas e localidade de memória. |
| [3](tarefa3/) | Medição de tempo e somas com dependências ou acumuladores independentes. |
| [4](tarefa4/) | Programas OpenMP limitados por processamento ou acesso à memória. |
| [5](tarefa5/) | Contagem de primos nas versões sequencial e paralela. |
| [6](tarefa6/) | Monte Carlo para π e cláusulas de compartilhamento de dados do OpenMP. |
| [7](tarefa7/) | Criação de tarefas OpenMP ao percorrer uma lista encadeada. |
| [8](tarefa8/) | Monte Carlo: acumulação, geração aleatória e falso compartilhamento. |
| [9](tarefa9/) | Tarefas OpenMP e sincronização de inserções em listas encadeadas. |
| [10](tarefa10/) | Comparação de `critical`, `atomic`, acumulação local e `reduction`. |
| [11](tarefa11/) | Difusão viscosa em malha 2D; escalonamentos `static`, `dynamic`, `guided` e `collapse`. |
| [12](tarefa12/README.md) | Otimização do solver com distribuição de laços, SIMD e região paralela persistente. |
| [13](tarefa13/README.md) | Afinidade de threads, `OMP_PROC_BIND`, `OMP_PLACES` e escalabilidade. |
| [14](tarefa14/README.md) | Ping-pong MPI entre dois processos com diferentes tamanhos de mensagem. |
| [15](tarefa15/README.md) | Difusão de calor 1D com comunicação bloqueante, não bloqueante e sobreposição com cálculo. |
| [16](tarefa16/README.md) | Produto matriz-vetor MPI com distribuição por linhas. |
| [17](tarefa17/README.md) | Produto matriz-vetor MPI com distribuição por colunas e tipos derivados. |
| [18](tarefa18/README.md) | Soma de vetores em CPU e GPU com OpenMP. |
| [19](tarefa19/README.md) | Difusão de calor com OpenMP target e análise com Nsight Systems. |
| [20](tarefa20/README.md) | Dados persistentes na GPU e acesso contíguo à memória. |

## Requisitos

As dependências variam conforme a atividade:

- **GCC com suporte a OpenMP** para os programas em CPU.
- **GNU Make** nos diretórios que fornecem Makefile.
- **MPI**, como Open MPI ou MPICH, com `mpicc` e `mpirun`, para as tarefas 14 a 17.
- **Python 3** para automação; NumPy e Matplotlib para `tarefa11/navier_stokes/visualizar.py`.
- **GPU NVIDIA e compilador com OpenMP offload** para as versões em GPU. Os scripts do NPAD usam NVHPC 24.11 e V100.
- **Slurm e módulos do NPAD** para os jobs `.slurm`; **PaScal Suite** e **Nsight Systems** nos experimentos que usam essas ferramentas.

Não há compilação única na raiz: compile cada atividade em seu diretório. Recompile os fontes para o seu ambiente, pois os executáveis presentes podem depender da máquina em que foram gerados.

## Compilação e execução local

Cada exemplo abaixo parte da raiz do repositório.

### Programa sequencial

A tarefa 1 recebe a quantidade de termos da aproximação de π:

```bash
gcc -O3 tarefa1/main.c -o /tmp/tarefa1-pi -lm
/tmp/tarefa1-pi 1000000
```

### OpenMP

Compare a contagem de primos usando o mesmo limite:

```bash
cd tarefa5
gcc -O3 sequencial.c -o sequencial -lm
gcc -O3 -fopenmp paralelo.c -o paralelo -lm
./sequencial 100000
OMP_NUM_THREADS=4 ./paralelo 100000
```

`OMP_NUM_THREADS` configura as threads dos programas que usam a variável de ambiente. Algumas atividades recebem essa quantidade como argumento.

Para compilar e validar as três versões da tarefa 12 contra a referência serial:

```bash
cd tarefa12
make
make validar
./navierStokesV1 256 300 4
./navierStokesV2 256 300 4
./navierStokesV3 256 300 4
```

Os argumentos são `N PASSOS THREADS`. O [README da tarefa 12](tarefa12/README.md) detalha os experimentos e a integração com PaScal.

### MPI

O ping-pong da tarefa 14 exige exatamente dois processos:

```bash
cd tarefa14
make
mpirun -np 2 ./ping_pong > resultados.csv
```

Para comparar as estratégias de comunicação na difusão de calor:

```bash
cd tarefa15
make
mpirun -np 4 ./calorBloqueante 1000000 1000 0.24
mpirun -np 4 ./calorNaoBloqueante 1000000 1000 0.24
mpirun -np 4 ./calorComSobreposicao 1000000 1000 0.24
```

Os argumentos são `CELULAS PASSOS ALPHA`; use `0 < ALPHA <= 0.5`. No produto matriz-vetor, a dimensão distribuída deve ser divisível pelo número de processos: linhas (`M`) na tarefa 16 e colunas (`N`) na tarefa 17.

## Execução no NPAD

Submeta os jobs a partir da pasta da atividade. Por exemplo:

```bash
cd tarefa20
sbatch executarExperimento.slurm
```

Para uma configuração menor da tarefa 20:

```bash
TAMANHOS="256 1024" PASSOS=100 sbatch executarExperimento.slurm
```

Os scripts especificam partições, módulos, CPUs, GPUs e limites de tempo do ambiente utilizado. Confira essas configurações ao executar em outro cluster. Consulte os READMEs de cada tarefa para conhecer os parâmetros e os arquivos de saída.

## Resultados e materiais de apoio

Os resultados ficam junto às atividades, em relatórios Markdown, CSVs e pastas como `resultadosNpad/`, `saidasLocais/` e `saidaNpad/`, conforme o script.

- [Resultados dos testes da tarefa 6](tarefa6/resultados_testes.md).
- [Resultados de escalonamento e chunks da tarefa 11](tarefa11/resultados_chunks.md).
- [Descrição e visualização da simulação de difusão viscosa](tarefa11/navier_stokes/README.md).
- [Análise dos resultados da tarefa 12](tarefa12/analise_resultados.md).

Para comparar desempenho, mantenha a mesma carga e registre hardware, compilador, quantidade de threads ou processos e afinidade. Diferencie tempo de cálculo, comunicação, transferências e tempo total conforme a métrica de cada programa. Nas tarefas 19 e 20, os tempos do profiler vêm de uma execução separada e não representam uma decomposição exata da mediana do tempo de solução.
