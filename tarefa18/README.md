# Tarefa 18 — OpenMP CPU e GPU

Os scripts fazem uma execução e o próprio programa imprime `Init time`,
`Compute time`, `Test time` e `Total time` no arquivo do job em `saidasNpad/`.

```bash
sbatch executar_cpu.slurm
sbatch executar_gpu.slurm
```

O `executar_cpu.slurm` solicita 32 threads na partição `intel-512`. O script
GPU solicita uma GPU V100 na partição `gpu-8-v100` e compila para a arquitetura
`sm_70`, correspondente a essa GPU.

O script GPU carrega os módulos `llvm` e `cuda`, que precisam disponibilizar
um LLVM/Clang com `libomptarget` e suporte CUDA. Ele usa
`OMP_TARGET_OFFLOAD=MANDATORY`: uma configuração incompleta gera erro, em vez
de medir uma execução acidental na CPU. Se o NPAD usar nomes/versionamentos
diferentes, descubra-os com `module avail` e ajuste somente as duas linhas
`module load` do script.
