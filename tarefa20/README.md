# Tarefa 20 — Dados persistentes e acesso à memória

Esta implementação mantém o modelo físico e o stencil da tarefa 19 e avalia
duas mudanças: manter os campos na GPU durante todos os passos e inverter
a ordem dos laços para percorrer posições consecutivas do array.
A tarefa 19 permanece como referência histórica; este experimento compila
o mesmo arquivo `heatTarget.c` três vezes, com os mesmos parâmetros.

| Versão | Opções de compilação | Movimentação dos campos | Laço interno |
|---|---|---|---|
| `copias` | `-DCOPY_PER_STEP -DORIGINAL_LOOP_ORDER` | Envio e retorno em cada passo | `j`, ordem original |
| `persistente` | `-DORIGINAL_LOOP_ORDER` | Envio inicial e retorno final | `j`, ordem original |
| `otimizado` | Nenhuma macro | Envio inicial e retorno final | `i`, acesso contíguo |

`COPY_PER_STEP` desativa a persistência dos dados.
`ORIGINAL_LOOP_ORDER` mantém `j` no laço interno.
Essas opções são escolhidas pelo compilador: o script cria três executáveis,
mas o código-fonte é único. Não são parâmetros passados ao programa.

## Como as mudanças funcionam

- `target data map(to: u[0:cells]) map(alloc: u_tmp[0:cells])` mantém as
  duas alocações mapeadas durante o laço temporal. Só o campo inicial é enviado.
- Os `map` do kernel reutilizam esses mapeamentos. Cada passo atualiza a
  saída na GPU, sem devolver os elementos à CPU entre passos.
- A CPU continua trocando os ponteiros. As duas alocações permanecem
  mapeadas, e o kernel seguinte acessa o campo produzido pelo anterior.
- `target update from(u[0:cells])`, dentro da região de dados, devolve
  o resultado final apontado por `u`, para passos pares ou ímpares.
- Com `i` no laço interno, o índice `i+j*n` avança de um em um. Essa ordem
  favorece acessos a endereços próximos pelas threads; o ganho depende da
  compilação e do dispositivo e deve ser confirmado pelas medições.

Para P passos, a referência movimenta `2*P*8*n²` bytes dos campos, enquanto
as versões persistentes movimentam `2*8*n²`. Com 100 passos e n=4096, isso
corresponde a 25 GiB e 256 MiB, respectivamente. Os 100 kernels continuam
necessários porque os passos dependem uns dos outros.

## Executar no NPAD

Na pasta `tarefa20`:

```bash
sbatch executarExperimento.slurm
```

O job usa uma V100, NVHPC 24.11, uma CPU e as malhas 256², 512², 1024²,
2048² e 4096², com 100 passos. Para um teste menor:

```bash
TAMANHOS="256 1024" PASSOS=100 sbatch executarExperimento.slurm
```

`NVHPC_MODULE` e `NSYS_MODULE` permitem substituir os módulos carregados.
Os resultados ficam em uma pasta exclusiva de `resultadosNpad/`:

- `resumo.csv`: mediana de três execuções, erro L2 da última execução e
  tempos acumulados dos kernels e das cópias em uma execução separada.
- `repeticoes.csv`: tempos e erros de cada repetição, para avaliar variações.
- Arquivos `*-kern.csv` e `*-mem_time.csv`: relatórios do Nsight Systems,
  para conferir durações, instâncias e volumes registrados.
- `heatTarget.c` e `ambiente.txt`: código executado e versões das ferramentas.

Compare `copias` com `persistente` para avaliar a persistência e
`persistente` com `otimizado` para avaliar a ordem dos acessos.
O ganho é a mediana da versão de referência dividida pela mediana da
versão modificada. Confira também se os erros L2 permanecem equivalentes.
Para os campos, espera-se um envio e um retorno por passo em `copias` e
somente um envio inicial e um retorno final nas versões persistentes.
O runtime pode registrar outras pequenas operações de memória.

O tempo de solve inclui a entrada e a saída da região de dados, os kernels,
as transferências, a sincronização e a primeira chamada ao dispositivo.
Ele exclui a inicialização dos campos e a norma L2. Os tempos do profiler
vêm de outra execução e não são uma decomposição exata da mediana.

## Verificação local

As três versões foram compiladas com GCC/OpenMP e verificadas com execução
no host (`OMP_TARGET_OFFLOAD=DISABLED`), nas malhas 1², 8² e 32², com 1, 2,
5 e 100 passos. Os erros L2 coincidiram na precisão impressa. A sintaxe
Bash do script também foi verificada. Esses testes verificam o cálculo e
a leitura dos argumentos; a persistência e o desempenho na GPU precisam
ser confirmados com NVHPC e Nsight Systems no NPAD.

O programa aceita `./heatTarget n passos`, valida os parâmetros e verifica
a alocação. O aviso de estabilidade usa o limite bidimensional `r > 0,25`.
Ainda não há resultados de desempenho da tarefa 20: serão produzidos pelo job.
