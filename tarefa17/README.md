# Tarefa 17 — produto matriz-vetor com colunas distribuídas

O processo 0 cria a matriz `A` de tamanho `M x N` e o vetor `x` de tamanho
`N`. A matriz, armazenada por linhas, é distribuída por blocos de colunas.
Para isso, `MPI_Type_vector` descreve as colunas não contíguas de cada bloco e
`MPI_Type_create_resized` ajusta a extensão do tipo para que `MPI_Scatter`
avance de um bloco de colunas para o próximo.

Cada processo recebe também o segmento correspondente de `x`, calcula a sua
contribuição para todas as `M` posições de `y = A*x` e `MPI_Reduce` com
`MPI_SUM` soma as contribuições no processo 0.

## Compilar e executar

```bash
make
mpirun -np 4 ./matrizVetor 2000 2000
```

Os argumentos são `M N`; os valores padrão são `1000 1000`. `N` deve ser
divisível pela quantidade de processos, pois todos recebem a mesma quantidade
de colunas. A saída inclui o tempo total, medido após uma barreira, e `y0` para
uma verificação simples do resultado.
