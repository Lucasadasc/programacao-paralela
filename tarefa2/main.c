#define _POSIX_C_SOURCE 199309L // Necessário para clock_gettime() no Linux
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Recuperar tempo de relógio transcorrido (em segundos) conforme orientado na sala de aula
double recuperar_tempo_instantaneo() {
    struct timespec ts; // Estrutura para armazenar o tempo
    clock_gettime(CLOCK_MONOTONIC, &ts); // Obtém o tempo de relógio monotônico (não retrocede)
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Converte para segundos
}

// Versão A: Acesso por linha (loop interno varia colunas) - Padrão do C
// O * antes do parametro indica que é um ponteiro para o tipo double, ou seja, é um array de doubles
void multiplicacao_matriz_vetor_linha(const double *A, const double  *x, double  *y, int N) {
    for (int i = 0; i < N; i++) {
        double soma = 0.0;
        for (int j = 0; j < N; j++) {
            soma += A[i * N + j] * x[j];
        }
        y[i] = soma;
    }
}

// Versão B: Acesso por coluna (loop interno varia linhas) - Estrutura de coluna
void multiplicacao_matriz_vetor_coluna(const double *A, const double *x, double *y, int N) {
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

// Inicializa matriz e vetor com valores determinísticos para facilitar a verificação de resultados
void inicializar_matrizes(double *matriz_A, double *vetor_x, int N) {
    for (int i = 0; i < N; i++) {
        vetor_x[i] = (double)i / N;
        for (int j = 0; j < N; j++) {
            matriz_A[i * N + j] = (double)(i + j) / N;
        }
    }
}


void realizar_teste(int N) {
    printf("Calculando com N = %d...\n", N);
    
    // Alocando memória
    double *matriz_A = (double *)malloc((size_t)N * N * sizeof(double));
    double *vetor_x = (double *)malloc((size_t)N * sizeof(double)); // o vetor_x seria 
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
    printf("  Linha:        %10.6f s\n", tempo_total_calculo_vetor_linha);
    printf("  Coluna:       %10.6f s\n", tempo_total_calculo_vetor_coluna);

    free(matriz_A);
    free(vetor_x);
    free(resultado_em_linha);
    free(resultado_em_coluna);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Rodando com um tamanho de matriz específico fornecido como argumento
        int N = atoi(argv[1]); // o atoi converte uma string para um inteiro
        if (N <= 0) {
            fprintf(stderr, "Tamanho inválido: %s\n", argv[1]);
            return 1;
        }
        realizar_teste(N);

        return 0;
    } 
    
    //Testando diferentes tamanhos de matriz
    int tamanhos[] = {10, 50, 100, 250, 500, 1000, 2000, 5000, 10000, 15000, 20000, 25000};
    int quantidade_tamanhos = 12;
    
    for (int i = 0; i < quantidade_tamanhos; i++) {
        realizar_teste(tamanhos[i]);
    }
        
    return 0;
}
