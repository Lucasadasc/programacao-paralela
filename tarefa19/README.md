# Tarefa 19 — OpenMP target e Nsight Systems

`heat.c` é a referência serial. `heatTarget.c` usa `target teams loop
collapse(2)` com `map` e `device`: as atualizações independentes do stencil
são distribuídas entre equipes e threads da GPU. Os mapeamentos permanecem
dentro de `solve` intencionalmente, portanto cada passo transfere `u` para a
GPU e devolve `u_tmp` ao host. Este é o caso-base da Tarefa 19, que permite
visualizar no Nsight Systems o custo e a frequência das cópias; a permanência
dos dados no dispositivo fica para a Tarefa 20.

No NPAD, execute na pasta da tarefa:

```bash
sbatch executarExperimento.slurm
```

O script usa uma V100 na partição `gpu-8-v100`, NVHPC 24.11 e limite de
uma hora. Executa malhas 256², 1024² e 4096², com 100 passos.
Para outros tamanhos (até 4096) ou número de passos:

```bash
TAMANHOS="256 512 1024" PASSOS=100 sbatch executarExperimento.slurm
```

Cada job salva **apenas um arquivo de resultados**, `resumo.csv`, em uma
pasta exclusiva de `resultadosNpad/`. Ele contém:

- Tamanho da malha e número de passos.
- Mediana do tempo de solve de três execuções sem profiler, em segundos.
- Erro L2 da última dessas execuções.
- Tempo acumulado dos kernels e das cópias, em milissegundos, em uma
  execução separada com `nsys`.

O Slurm também mantém os logs `heat-nsys-<job>.out` e `.err`.
Executável, perfis e SQLite são intermediários em `/tmp` e são removidos
ao final; resultados antigos permanecem intactos.

O tempo de solve inclui as transferências de cada passo. Os tempos dos
kernels e das cópias vêm de outra execução e não devem ser interpretados
como uma decomposição exata da mediana.

Se necessário, informe `NSYS_MODULE` para carregar um módulo próprio de
Nsight Systems ou `NVHPC_MODULE` para substituir o módulo do compilador.
