# Tarefa 13 — afinidade de threads

Avaliar como a afinidade altera a escalabilidade do Navier–Stokes no mesmo
no da tarefa 12 (slide 66, pagina 64 do PDF). Para evitar a espera pelo r2n50,
o job permite qualquer no da particao `amd-512`. Todas as politicas executam
no mesmo no alocado. Confira o hardware em `cpu.txt` e registre essa diferenca
em relacao ao enunciado ao apresentar os resultados.

## Implementacao e comparacao

Os tres fontes `navierStokes.c`, `navierStokes.h` e `navierStokesV3.c` foram
copiados da tarefa 12 para preservar a base do experimento. O solver permanece
na V3: regiao persistente, ponteiros privados, SIMD e inicializacao paralela.
A alteracao no programa comum acrescenta tempos de inicializacao e do intervalo
medido. As politicas comparadas sao `false`, `true`, `close`, `spread`, `master`,
sempre com `OMP_PLACES=cores`. Nao se altera o solver entre politicas.

- `false`: sem fixacao pelo OpenMP, ainda sujeito ao cpuset do sistema/Slurm.
- `true`: habilita fixacao; distribuicao depende da implementacao.
- `close`: aproxima as threads nos lugares disponiveis.
- `spread`: distribui as threads pelos lugares disponiveis.
- `master`: concentra no lugar da thread principal; pode disputar um unico
  nucleo. Nome usado pelo GCC 8.5 e pelos slides (em OpenMP recente: `primary`).

## Executar

Requer Linux, GCC com OpenMP e Python 3.6 ou superior (somente biblioteca padrao).

```bash
make validar
make rapido
```

`make rapido` usa diretorio exclusivo em `/tmp`; se ja existir, indique outro:

```bash
python3 executarExperimentos.py --threads 1 2 --lados 32 64 --base-fraca 1024 --passos 10 --repeticoes 2 --saida /tmp/afinidade-teste2
```

No NPAD, submeta **de dentro da pasta tarefa13**:

```bash
sbatch executarNpad.slurm
```

O job coleta CSVs e tambem executa o PaScal Analyzer. Carrega automaticamente
`softwares/pascalsuite/2025-07-08`, como na tarefa 12. Se a instalacao for
disponibilizada por `env.sh`, use:

```bash
sbatch --export=ALL,PASCAL_ENV=/caminho/real/env.sh executarNpad.slurm
```

Os testes locais com `make rapido` continuam sem depender do PaScal.
O job executa os testes; ainda nao ha resultados do NPAD.
O Slurm pode disponibilizar mais nucleos que os 32 solicitados por causa do
uso exclusivo do no. O conjunto efetivo fica em `configuracao.json`; interprete
`spread` dentro desse conjunto, sem assumir que ocupa dois sockets.

## Cargas e medidas

Por padrao: 1, 4, 8, 16, 32 threads; 300 passos; cinco repeticoes.
As malhas internas fixas 256, 512, 1024, 2048 por lado permitem comparar
escalabilidade forte e recuperar as entradas da tarefa 12. Dobrar o lado
quadruplica o trabalho por passo.

Uma segunda sequencia usa aproximadamente **65.536 pontos por thread**:
lado interno = ceil(sqrt(65536 * threads)). O CSV registra a carga efetiva e a
quantidade de pontos por thread; arredondamentos sao necessarios para formar
malhas quadradas. Todas as entradas tambem executam com uma thread para
calcular speedup com referencia da mesma politica e da mesma malha.

Cada combinacao roda em processo novo. A ordem e embaralhada com semente 13
para reduzir a associacao entre politica e momento de execucao. O programa
verifica os resultados e o runner compara soma/pico entre politicas/threads;
`--validar` compara os campos completos com a referencia serial.

- `medidas.csv`: cada repeticao, com tempos em segundos.
- `resumo.csv`: media, desvio amostral, speedup e eficiencia por regiao.
- `configuracao.json`: parametros, configuracoes de OpenMP, recursos Slurm e CPUs permitidas, sem usuario, conta ou hostname.
- `afinidade-POLITICA-THREADS.txt`: CPU instantanea e mascara permitida de
  cada thread em um programa de diagnostico separado. E uma verificacao da
  configuracao, nao um rastreamento das threads do solver.

Regioes: `inicializacao` mede a reinicializacao apos aquecimento;
`evolucao` mede os passos; `medido` engloba ambas. Nenhuma representa o tempo
do processo inteiro: alocacao, primeira inicializacao, aquecimento, verificacao
e saida ficam fora. Isso difere da regiao 0 do PaScal da tarefa 12; nao compare
diretamente esses tempos como se fossem a mesma medida.

Speedup = media(1 thread, mesma malha e politica)/media(p threads).
Eficiencia = speedup/p. Nas linhas `sequencia_fraca=True`, eficiencia fraca =
tempo da menor malha com 1 thread/tempo atual, observando o arredondamento.

## Interpretar e apresentar

1. Compare tempos e dispersao entre politicas, mantendo entrada e threads.
2. Compare eficiencia ao aumentar threads com malha fixa (forte).
3. Acompanhe `sequencia_fraca=True` para carga aproximadamente constante por
   thread; o ideal e tempo constante. Avalie se malhas maiores recuperam eficiencia.
4. Relacione tendencias com as mascaras observadas e a topologia de `cpu.txt`.
   `true` pode coincidir com outra politica. `master` serve como contraste de
   concentracao; nao representa uso de p nucleos fisicos diferentes.
5. Separe resultado observado de mecanismo possivel. Cache, largura de banda e
   NUMA podem explicar diferencas, mas nao sao medidos aqui. Nao afirmar que
   primeiro acesso garantiu memoria local: inicializacao e evolucao percorrem
   intervalos diferentes e paginas podem conter dados usados por varias threads.

Graficos uteis a partir do resumo: tempo e eficiencia versus threads, uma
curva por politica para cada malha; tempo versus threads na sequencia fraca.
Conclua qual politica foi melhor **nas configuracoes medidas**, considerando
dispersao. A execucao local valida a coleta; a conclusao da tarefa depende do NPAD.

## JSONs para o PaScal Viewer

Dentro de `resultadosNpad/job-ID/pascal`, abra no Viewer:

- `afinidade-false.json`
- `afinidade-true.json`
- `afinidade-close.json`
- `afinidade-spread.json`
- `afinidade-master.json`

Sao arquivos nativos produzidos pelo Analyzer, um por politica. Cada arquivo
cruza as quatro entradas da tarefa 12 com 1, 4, 8, 16 e 32 threads e cinco
repeticoes. As regioes instrumentadas sao 1 (reinicializacao) e 2 (evolucao);
o Viewer tambem oferece a regiao 0 (programa inteiro, incluindo aquecimento).
Compare sempre a mesma regiao. Os CSVs provem de uma bateria separada;
nao representam exatamente as mesmas execucoes dos JSONs.

As entradas quadruplicam os pontos. Para uma verificacao convencional de
escalabilidade fraca nesses JSONs, use (1, i1), (4, i2), (16, i3).
A sequencia fraca ampliada com arredondamento permanece disponivel nos CSVs;
nao se deve assumir que cada celula do mapa fraco mantem carga por thread.

Para executar somente a bateria PaScal, dentro de uma alocacao adequada:

```bash
module load softwares/pascalsuite/2025-07-08
make pascal
bash executarPascal.sh resultadosNpad/pascal-manual
```

`CORES_PASCAL`, `PASSOS_PASCAL` e `REPETICOES_PASCAL` permitem alterar essa
bateria (por padrao `1,4,8,16,32`, `300` e `5`). Isso nao altera a bateria CSV.
