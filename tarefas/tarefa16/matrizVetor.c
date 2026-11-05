#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

static void preencherDados(double *matriz, double *vetor, int linhas, int colunas) {
    for (int i = 0; i < linhas * colunas; i++) {
        matriz[i] = (double)((i % 10) + 1);
    }

    for (int i = 0; i < colunas; i++) {
        vetor[i] = 1.0;
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int processo, quantidadeProcessos;
    MPI_Comm_rank(MPI_COMM_WORLD, &processo);
    MPI_Comm_size(MPI_COMM_WORLD, &quantidadeProcessos);

    int linhas = 1000;
    int colunas = 1000;

    if (argc == 3) {
        linhas = atoi(argv[1]);
        colunas = atoi(argv[2]);
    }

    // Verifica se o número de linhas é divisível pelo número de processos
    int linhasLocais = linhas / quantidadeProcessos;
    // cria matriz e vetor local para cada processo
    double *matrizM = NULL;
    double *vetorN = malloc((size_t)colunas * sizeof(double));
    // cria matriz local e vetor local para cada processo
    double *matrizLocal = malloc((size_t)linhasLocais * colunas * sizeof(double));
    double *resultadoLocal = malloc((size_t)linhasLocais * sizeof(double));
    double *resultado = NULL;

    if (processo == 0) {
        matrizM = malloc((size_t)linhas * colunas * sizeof(double));
        resultado = malloc((size_t)linhas * sizeof(double));
        preencherDados(matrizM, vetorN, linhas, colunas);
    }

    MPI_Barrier(MPI_COMM_WORLD);
   
    double inicio = MPI_Wtime();

    MPI_Scatter(
        matrizM, // linhas * colunas elementos da matriz
        linhasLocais * colunas, // cada processo recebe linhasLocais * colunas elementos
        MPI_DOUBLE, // tipo de dado
        matrizLocal, // buffer de recepção
        linhasLocais * colunas, // quantidade de elementos recebidos
        MPI_DOUBLE, // tipo de dado
        0, // processo raiz
        MPI_COMM_WORLD // comunicador
    );
    MPI_Bcast(
        vetorN, // vetor de tamanho 
        colunas, // quantidade de elementos
        MPI_DOUBLE, // tipo de dado
        0, // processo raiz
        MPI_COMM_WORLD // comunicador
    );

    // Cada processo realiza a multiplicação da sua parte da matriz pelo vetor
    for (int i = 0; i < linhasLocais; i++) {
        // Inicializa o resultado local para a linha i
        resultadoLocal[i] = 0.0;
        // Multiplica a linha i da matriz local pelo vetor
        for (int j = 0; j < colunas; j++) {
            // resultadoLocal[i] += matrizLocal[i][j] * vetorN[j];
            resultadoLocal[i] += matrizLocal[i * colunas + j] * vetorN[j];
        }
    }

    MPI_Gather(
        resultadoLocal, 
        linhasLocais, 
        MPI_DOUBLE,
        resultado, 
        linhasLocais, 
        MPI_DOUBLE,
        0, 
        MPI_COMM_WORLD
    );

    if (processo == 0) {
        double tempo = MPI_Wtime() - inicio;
        printf("M=%d N=%d processos=%d tempo_s=%.6f y0=%.1f\n",
               linhas, colunas, quantidadeProcessos, tempo, resultado[0]);
    }

    free(matrizM);
    free(vetorN);
    free(matrizLocal);
    free(resultadoLocal);
    free(resultado);

    MPI_Finalize();
    
    return 0;
}
