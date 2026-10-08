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
    } else if (argc != 1) {
        if (processo == 0) {
            fprintf(stderr, "Uso: %s [linhas colunas]\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (linhas <= 0 || colunas <= 0 || colunas % quantidadeProcessos != 0) {
        if (processo == 0) {
            fprintf(stderr,
                    "As dimensoes devem ser positivas e o numero de colunas "
                    "deve ser divisivel pelo numero de processos.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Cada processo recebera a mesma quantidade de colunas da matriz A.
    const int colunasLocais = colunas / quantidadeProcessos;
    double *matriz = NULL;
    double *vetor = NULL;
    double *resultado = NULL;
    double *matrizLocal = malloc((size_t)linhas * colunasLocais * sizeof(*matrizLocal));
    double *vetorLocal = malloc((size_t)colunasLocais * sizeof(*vetorLocal));
    double *resultadoParcial = calloc((size_t)linhas, sizeof(*resultadoParcial));

    if (matrizLocal == NULL || vetorLocal == NULL || resultadoParcial == NULL) {
        fprintf(stderr, "Processo %d: falha ao alocar memoria.\n", processo);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Somente o processo 0 cria os dados completos de A, x e y.
    if (processo == 0) {
        matriz = malloc((size_t)linhas * colunas * sizeof(*matriz));
        vetor = malloc((size_t)colunas * sizeof(*vetor));
        resultado = malloc((size_t)linhas * sizeof(*resultado));
        if (matriz == NULL || vetor == NULL || resultado == NULL) {
            fprintf(stderr, "Falha ao alocar memoria no processo 0.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        preencherDados(matriz, vetor, linhas, colunas);
    }

    /*
     * A matriz e armazenada por linhas em C. Para acessar o mesmo bloco de
     * colunas na proxima linha, e necessario saltar "colunas" elementos.
     * MPI_Type_vector descreve esses blocos nao contiguos na matriz original.
     */
    MPI_Datatype blocoColunas, blocoColunasRedimensionado;
    MPI_Type_vector(
        linhas, // numero de blocos
        colunasLocais, // numero de elementos em cada bloco
        colunas, // numero de elementos entre o inicio de cada bloco
        MPI_DOUBLE, // tipo de dado
        &blocoColunas // tipo de dado resultante
    );
    // cria um tipo redimensionado para que o deslocamento entre blocos seja correto
    MPI_Type_create_resized( 
        blocoColunas, // tipo de dado original
        0, // deslocamento inicial
        (MPI_Aint)colunasLocais * sizeof(double), // tamanho do bloco
        &blocoColunasRedimensionado // tipo de dado resultante
    );
    // comita o tipo de dado para que possa ser usado em comunicacoes
    MPI_Type_commit(&blocoColunasRedimensionado); 
    MPI_Type_free(&blocoColunas);

    MPI_Barrier(MPI_COMM_WORLD);
    const double inicio = MPI_Wtime();

    /*
     * O tipo redimensionado faz MPI_Scatter avancar um bloco de colunas por
     * vez. No receptor, o bloco chega de forma contigua em matrizLocal.
     */
    MPI_Scatter(matriz, 1, blocoColunasRedimensionado,
                matrizLocal, linhas * colunasLocais, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Cada processo recebe os elementos de x associados as suas colunas.
    MPI_Scatter(vetor, colunasLocais, MPI_DOUBLE,
                vetorLocal, colunasLocais, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Cada processo calcula uma contribuicao parcial para todas as linhas de y.
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunasLocais; j++) {
            resultadoParcial[i] += matrizLocal[i * colunasLocais + j] * vetorLocal[j];
        }
    }

    // MPI_SUM soma as contribuicoes parciais e deixa o vetor y completo no processo 0.
    MPI_Reduce(
        resultadoParcial, 
        resultado, 
        linhas,
        MPI_DOUBLE, 
        MPI_SUM, 
        0,
        MPI_COMM_WORLD
    );

    if (processo == 0) {
        const double tempo = MPI_Wtime() - inicio;
        printf("M=%d N=%d processos=%d tempo_s=%.6f y0=%.1f\n",
               linhas, colunas, quantidadeProcessos, tempo, resultado[0]);
    }

    MPI_Type_free(&blocoColunasRedimensionado);
    free(matriz);
    free(vetor);
    free(resultado);
    free(matrizLocal);
    free(vetorLocal);
    free(resultadoParcial);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
