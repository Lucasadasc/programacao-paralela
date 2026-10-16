# Tarefa 12

Cada implementacao possui seu proprio fonte e executavel:

- `navierStokesV1.c`: `collapse(2)` e copia dos contornos a cada passo.
- `navierStokesV2.c`: distribuicao por linhas, SIMD e contornos preservados.
- `navierStokesV3.c`: regiao paralela persistente, SIMD e ponteiros privados.

`navierStokes.c` compartilha inicializacao, argumentos, medicao e validacao.
`navierStokes.h` define a grade, a interface e a formula de atualizacao.
Cada executavel e ligado somente com a sua implementacao.

Compile e valide as tres versoes contra a referencia serial:

```bash
make
make validar
./navierStokesV1 256 300 4
./navierStokesV2 256 300 4
./navierStokesV3 256 300 4
```

Os argumentos sao `N PASSOS THREADS`. A saida CSV mantem a versao na primeira
coluna. `executarExperimentos.py` seleciona automaticamente cada executavel.

No NPAD, a partir desta pasta:

```bash
module load softwares/pascalsuite/2025-07-08
sbatch executarPascal.slurm
```

`make pascal` gera `navierPascal1`, `navierPascal2` e `navierPascal3`, usando
os mesmos fontes separados. Esses executaveis recebem `PONTOS_INTERNOS PASSOS`
e usam o numero de threads definido pelo Analyzer.
