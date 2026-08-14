#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Recuperar tempo de relógio transcorrido (em segundos) conforme orientado na sala de aula
double recuperar_tempo_instantaneo() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

// Versão A: Acesso por linha (loop interno varia colunas) - Padrão do C
void multiplicacao_matriz_vetor_linha(const double *restrict A, const double *restrict x, double *restrict y, int N) {
    for (int i = 0; i < N; i++) {
        double soma = 0.0;
        for (int j = 0; j < N; j++) {
            soma += A[i * N + j] * x[j];
        }
        y[i] = soma;
    }
}

// Versão B: Acesso por coluna (loop interno varia linhas) - Estrutura de coluna
void multiplicacao_matriz_vetor_coluna(const double *restrict A, const double *restrict x, double *restrict y, int N) {
    for (int i = 0; i < N; i++) {
        y[i] = 0.0;
    }
    for (int j = 0; j < N; j++) {
        double xj = x[j];
        for (int i = 0; i < N; i++) {
            y[i] += A[i * N + j] * xj;
        }
    }
}

// Inicializa matriz e vetor com valores dummy
void inicializar_matrizes(double *matriz_A, double *vetor_x, int N) {
    for (int i = 0; i < N; i++) {
        vetor_x[i] = (double)i / N;
        for (int j = 0; j < N; j++) {
            matriz_A[i * N + j] = (double)(i + j) / N;
        }
    }
}


void realizar_benchmark(int N) {
    printf("Evaluating N = %d...\n", N);
    
    // Alocando memória
    double *matriz_A = (double *)malloc((size_t)N * N * sizeof(double));
    double *vetor_x = (double *)malloc((size_t)N * sizeof(double));
    double *resultado_em_linha = (double *)malloc((size_t)N * sizeof(double));
    double *resultado_em_coluna = (double *)malloc((size_t)N * sizeof(double));
    
    inicializar_matrizes(matriz_A, vetor_x, N);

    double inicio_calculo_vetor_linha = recuperar_tempo_instantaneo();
    multiplicacao_matriz_vetor_linha(matriz_A, vetor_x, resultado_em_linha, N);
    double fim_calculo_vetor_linha = recuperar_tempo_instantaneo();
    double tempo_total_calculo_vetor_linha = fim_calculo_vetor_linha - inicio_calculo_vetor_linha;
    
    double inicio_calculo_vetor_coluna = recuperar_tempo_instantaneo();
    multiplicacao_matriz_vetor_coluna(matriz_A, vetor_x, resultado_em_coluna, N);
    double fim_calculo_vetor_coluna = recuperar_tempo_instantaneo();
    double tempo_total_calculo_vetor_coluna = fim_calculo_vetor_coluna - inicio_calculo_vetor_coluna;
    
    // Otimizações

    double ratio = tempo_total_calculo_vetor_coluna / tempo_total_calculo_vetor_linha;
    
    printf("  Linha:        %10.6f s\n", tempo_total_calculo_vetor_linha);
    printf("  Coluna:       %10.6f s\n", tempo_total_calculo_vetor_coluna);
    printf("  Razão (Col/Linha): %.2fx\n", ratio);

    free(matriz_A);
    free(vetor_x);
    free(resultado_em_linha);
    free(resultado_em_coluna);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Rodando com um tamanho de matriz específico fornecido como argumento
        int N = atoi(argv[1]);
        if (N <= 0) {
            fprintf(stderr, "Invalid matrix size: %s\n", argv[1]);
            return 1;
        }
        realizar_benchmark(N);

        return 0;
    } 
    
    //Testando diferentes tamanhos de matriz
    int sizes[] = {10, 100, 1000, 2000, 5000, 10000, 15000, 20000, 25000}; // Com 30000, o wsl foi 'derrubado' por falta de memória
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    printf("=====================================================\n");
    printf("  Matrix-Vector Multiplication (MxV) Benchmark in C  \n");
    printf("=====================================================\n\n");
    
    for (int i = 0; i < num_sizes; i++) {
        realizar_benchmark(sizes[i]);
    }
        
    return 0;
}

// Por que com matriz pequena o tempo é o mesmo? Por que o tempo não muda: Tamanho da cache
