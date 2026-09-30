# Tarefa 18 — OpenMP CPU e GPU

Os scripts fazem uma execução e o próprio programa imprime `Init time`,
`Compute time`, `Test time` e `Total time` no arquivo do job em `saidasNpad/`.

```bash
sbatch executar_cpu.slurm
sbatch executar_gpu.slurm
```

O `executar_cpu.slurm` usa 32 threads da partição `intel-512`, conforme os
scripts das tarefas anteriores. O script GPU solicita uma GPU com
`--gres=gpu:1` e não fixa uma partição, pois o nome da partição GPU deve ser o
configurado no NPAD. Caso `sinfo` indique uma partição específica, acrescente,
por exemplo, `--partition=NOME_DA_PARTICAO` ao comando `sbatch`.

Para o alvo OpenMP na GPU, carregue no NPAD um módulo LLVM/Clang que tenha
`libomptarget` e suporte CUDA antes de submeter, por exemplo `module load ...`.
O script usa `OMP_TARGET_OFFLOAD=MANDATORY`: uma configuração incompleta gera
erro, em vez de medir uma execução acidental na CPU.
