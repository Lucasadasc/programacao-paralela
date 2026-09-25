# Tarefa 16 — produto matriz-vetor com MPI

O processo 0 cria a matriz `A` de tamanho `M x N` e o vetor `x` de tamanho
`N`. As linhas de `A` são divididas igualmente com `MPI_Scatter`; o vetor
inteiro é enviado a todos os processos com `MPI_Bcast`. Cada processo calcula
sua parte de `y = A*x`, e `MPI_Gather` reúne o resultado no processo 0.

## Compilar e executar

```bash
make
mpirun -np 4 ./matrizVetor 2000 2000
```

Os argumentos são `M N` e têm valor padrão `1000 1000`. `M` deve ser divisível
pela quantidade de processos, pois a divisão usa `MPI_Scatter` com o mesmo
número de linhas para todos. A saída informa o tempo medido pelo processo 0.

## Experimento

Execute o script com diferentes números de processos, por exemplo 2, 4 e 8. O script fixa um processo MPI por nó, portanto cada configuração usa a mesma quantidade de processos por nó:

```bash
mkdir -p saidasNpad
sbatch --ntasks=2 executarExperimento.slurm
sbatch --ntasks=4 executarExperimento.slurm
sbatch --ntasks=8 executarExperimento.slurm
```

Cada execução testa matrizes quadradas de 1000, 2000 e 4000, três vezes, e
salva os tempos em `saidasNpad/resultados-np<processos>.csv`. Para cada tamanho e número
de processos, compare a média (ou mediana) de `tempo_s`.
