# Tarefa 14 — Ping-pong MPI

O `ping_pong.c` exige exatamente dois processos MPI. O processo 0 envia um
buffer ao processo 1; assim que o recebe, o processo 1 devolve o mesmo buffer.
O processo 0 mede, com `MPI_Wtime`, o tempo total de várias trocas consecutivas
para cada tamanho entre 8 B e 1 MiB (potências de dois).

## Compilar e executar localmente

```bash
make
mpirun -np 2 ./ping_pong > resultados.csv
```

O programa realiza 10.000 trocas por tamanho e mede mensagens de 8 B até
1 MiB, em potências de dois. Esses parâmetros estão fixos no código.

O CSV inclui o tempo total medido, RTT médio por troca e a taxa efetiva,
considerando os dois percursos da mensagem (ida e volta).

## Slurm

Para medir entre dois nós, submeta o script a partir desta pasta:

```bash
sbatch executar.slurm
```

Ele reserva dois nós com um processo em cada um e cria `resultados.csv`.
