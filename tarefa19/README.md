# Tarefa 19 — OpenMP target e Nsight Systems

`heat.c` é a referência serial. `heatTarget.c` reutiliza seu cálculo e usa
somente `target` (`map`, `device`) e `loop` (`collapse`). A redução de uma
soma descartada foi removida após o NVHPC 24.11 abortar com
`Internal compiler error. unexpected ILM number operands for reduction pragma`.
Essa remoção não altera o cálculo da solução nem do erro L2.

## Execução no NPAD

Na pasta `tarefa19` do cluster:

```bash
sbatch executarExperimento.slurm
```

O script solicita uma GPU na partição `gpu-8-v100` e carrega
`compilers/nvidia/nvhpc/24.11`, seguindo a configuração da tarefa 18 deste
projeto. Compila com `nvc -O3 -mp=gpu -gpu=cc70,lineinfo` para a V100 e
exige execução no dispositivo com `OMP_TARGET_OFFLOAD=MANDATORY`.
Confirme a disponibilidade da partição e dos módulos no cluster.

A bateria padrão usa malhas de **256, 512, 1024, 2048 e 4096** por dimensão,
**100 passos** e **três repetições sem profiler** por tamanho. Também faz
uma execução de aquecimento e uma execução perfilada por tamanho.
O limite de 4096 corresponde à configuração de memória deste script.

Para personalizar:

```bash
TAMANHOS="512 1024 2048" PASSOS=200 REPETICOES=5 sbatch executarExperimento.slurm
```

Se `nsys` não estiver no PATH após carregar o NVHPC, localize seu módulo
com `module spider nsight` ou `module spider nsys` e submeta informando
`NSYS_MODULE=nome_do_modulo`. O módulo do compilador pode ser substituído
com `NVHPC_MODULE=nome_do_modulo`.

## Resultados e análise

Cada job cria uma pasta exclusiva em `resultadosNpad/`, com:

- `tempos.csv`: tamanho, células, passos, repetição, tempo do solve,
  tempo total e erro L2 das execuções sem profiler.
- `n*_perfil.nsys-rep`: linha do tempo para abrir no Nsight Systems.
- `n*_perfil_stats.txt` e CSVs: resumos das APIs CUDA, kernels e
  transferências, incluindo tempo e volume de memória.
- Logs de execução, compilação, ambiente e cópia do código utilizado.

Compare a média do tempo de `solve` entre repetições para cada tamanho e
calcule células atualizadas por segundo como `n*n*passos/tempo_solve`.
Use os perfis para identificar quanto tempo está nos kernels e nas cópias
host–GPU. Os percentuais do `nsys stats` são relativos às operações do
respectivo relatório, não ao tempo total do programa.

Nesta implementação, `map(to: u)` e `map(from: u_tmp)` transferem a malha
em cada passo; cada direção movimenta aproximadamente `8*n*n*passos`
bytes. O tempo total inclui inicialização e cálculo
serial do erro L2. As execuções são processos separados: o aquecimento
não elimina o custo de criar o contexto CUDA nas execuções seguintes.

Referências: [guia do Nsight Systems](https://docs.nvidia.com/nsight-systems/UserGuide/index.html)
e [relatórios de análise](https://docs.nvidia.com/nsight-systems/AnalysisGuide/index.html).

Os resultados medidos serão gerados ao executar o job no NPAD; não foram
coletados neste ambiente, que não possui NVHPC, `nsys` ou Slurm.
