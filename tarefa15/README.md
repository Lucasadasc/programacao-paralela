# Tarefa 15 — difusão de calor 1D com MPI

O domínio contém `N` células e é repartido entre os processos. Cada trecho aloca
duas células *ghost*: índice `0` (vizinho à esquerda) e `quantidadeCelulasLocais + 1` (vizinho à
direita). A atualização explícita é:

`T_novo[i] = T[i] + alpha * (T[i-1] - 2*T[i] + T[i+1])`

As extremidades globais permanecem em 100 e 0 graus. Para estabilidade, use
`0 < alpha <= 0.5`.

## Versões

- `calorBloqueante`: troca de halos com `MPI_Send`/`MPI_Recv`, com ordenação par/ímpar para não criar deadlock.
- `calorNaoBloqueante`: publica `MPI_Irecv` e `MPI_Isend`, depois usa `MPI_Wait` para cada requisição antes de calcular o trecho.
- `calorComSobreposicao`: publica as mesmas operações não bloqueantes; calcula primeiro os pontos internos (que não dependem dos halos) e chama `MPI_Test` nos recebimentos a cada 100 células calculadas. Depois de a troca terminar, calcula apenas os dois pontos de fronteira do trecho.

## Compilar e executar

```bash
make
mpirun -np 4 ./calorBloqueante 1000000 1000 0.24
mpirun -np 4 ./calorNaoBloqueante 1000000 1000 0.24
mpirun -np 4 ./calorComSobreposicao 1000000 1000 0.24
# executa as três sob a mesma carga
./executarComparacao.sh 4 1000000 1000 0.24
```

Os argumentos são `celulas`, `passos` e `alpha`; todos são opcionais. O processo 0 exibe o maior tempo entre processos (a métrica de tempo paralelo) e a média.

## Experimentos no NPAD

### Bateria de nós, processos e células

No NPAD, copie este diretório. A bateria usa 500 passos, `alpha=0.24`, 5
repetições e 100.000, 1.000.000, 2.000.000, 5.000.000, 7.000.000 e
10.000.000 células. Ela submete três jobs: 2, 4 e 8 nós, sempre com um processo
MPI por nó (2, 4 ou 8 processos totais). Cada job grava seus arquivos em
`saidaNpad/`.

```bash
chmod +x submeterBateriaNpad.sh
./submeterBateriaNpad.sh       # 5 repetições
./submeterBateriaNpad.sh 5     # 5 repetições
squeue -u "$USER"
```

`executarBateriaNpad.slurm` compila em diretório temporário por nó, portanto
submissões concorrentes não disputam os binários. Ele produz os resultados brutos
em `saidaNpad/resultados-n<NOS>-p<PROCESSOS>-job<JOB>.csv` e as medianas em
`saidaNpad/medianas-n<NOS>-p<PROCESSOS>-job<JOB>.csv`. Os logs do Slurm também
ficam em `saidaNpad/`. A primeira repetição é descartada
automaticamente como aquecimento; a mediana usa `tempo_maximo_s` das demais.
Para reduzir a matriz ou os tamanhos, altere as listas no início de
`submeterBateriaNpad.sh`.

### Experimentos individuais

O arquivo `executarExperimento.slurm` executa as versões bloqueante, com espera e com `MPI_Test` a cada 100 células, repetindo cada uma seis vezes e salvando as medições em CSV. Submeta uma configuração por vez para que cada quantidade de processos receba recursos equivalentes:

```bash
# Escalabilidade: repita para 2, 4, 8 e 16 processos.
sbatch --ntasks=4 executarExperimento.slurm escalabilidade 1000000 1000 0.24 6

# Trabalho por processo: repita com 100000, 1000000 e 10000000 células.
sbatch --ntasks=4 executarExperimento.slurm trabalho 1000000 1000 0.24 6

# Muitas trocas de halos pequenos.
sbatch --ntasks=4 executarExperimento.slurm latencia 10000 10000 0.24 6

# Carga grande: repita para 4 e 8 processos.
sbatch --ntasks=4 executarExperimento.slurm carga-grande 10000000 1000 0.24 6
```

Descarte a repetição 1 de cada programa e calcule a mediana de `tempo_maximo` das demais.

Para repetir a bateria calibrada, execute:

```bash
chmod +x submeterBateriaNpad.sh
./submeterBateriaNpad.sh 6
```

O argumento opcional é o número de repetições; `10` produz nove amostras úteis após descartar a primeira. A bateria submete sete jobs, de 2 a 8 processos. Ela mantém 1.000.000 de células por processo e 1.000 passos, portanto testa de 2.000.000 a 8.000.000 células. A faixa foi calibrada em 2 processos para ficar próxima de um segundo em cada versão; manter o trabalho por processo evita que os casos com mais processos fiquem curtos demais.

| Cenário | Processos | Células | Passos | Trabalho por processo |
|---|---:|---:|---:|---:|
| `um-segundo` | 2 a 8 | 1.000.000 × processos | 1.000 | 1.000.000 células |

## Resultados locais da bateria

Os tempos abaixo são o `tempo_maximo` de uma execução local para cada configuração,
com `alpha = 0,24`. Eles servem como comparação inicial; repita no NPAD e use a
mediana das repetições após descartar o aquecimento para resultados reportáveis.

| Processos | Células | Passos | Bloqueante (s) | Não bloqueante + Wait (s) | Sobreposição + Test (s) |
|---:|---:|---:|---:|---:|---:|
| 2 | 2.000.000 | 1.000 | 1,391859 | 1,136615 | 1,031798 |
| 3 | 3.000.000 | 1.000 | 1,740029 | 1,867993 | 1,755191 |
| 4 | 4.000.000 | 1.000 | 2,334914 | 2,474300 | 2,393142 |
| 5 | 5.000.000 | 1.000 | 2,958024 | 2,900371 | 2,859952 |
| 6 | 6.000.000 | 1.000 | 3,566159 | 3,811345 | 3,518069 |
| 7 | 7.000.000 | 1.000 | 4,221411 | 4,089120 | 4,175862 |
| 8 | 8.000.000 | 1.000 | 4,614404 | 4,585615 | 4,748191 |

## Comparação esperada

`Send/Recv` força cada processo a aguardar a troca antes de qualquer cálculo.
A versão `Isend/Irecv+Wait` evita possíveis bloqueios de envio, mas ainda espera a
comunicação antes do cálculo; por isso tende a ter desempenho semelhante ao da
versão bloqueante para mensagens pequenas. A versão com `MPI_Test` pode esconder
parte da latência: enquanto os halos chegam, calcula `quantidadeCelulasLocais - 2` pontos
independentes. O ganho cresce quando há trabalho interno suficiente por processo e
a comunicação tem latência relevante. Em redes locais com mensagens de apenas um
`double`, ou com subdomínios muito pequenos, o custo de testar repetidamente pode
anular o ganho. Compare sempre o `tempo_maximo`, usando várias repetições e a mesma
quantidade de processos/células/passos.
