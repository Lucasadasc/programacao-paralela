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

## Experimento local

Execute no seu computador:

```bash
chmod +x executarExperimentoLocal.sh
./executarExperimentoLocal.sh
```

O script compila o programa e testa matrizes quadradas de 2000, 4000 e 8000 com 2, 4 e 8 processos, seis vezes cada. Os resultados ficam em `saidasLocais/resultados.csv`, com uma linha por configuração e a mediana das seis repetições.

Os parâmetros são opcionais:

```bash
./executarExperimentoLocal.sh "2 4" "2000 4000" 3
```

O exemplo executa 2 e 4 processos, matrizes de 2000 e 4000, com três repetições. `M` precisa ser divisível pela quantidade de processos. Compare `mediana_tempo_s`.
