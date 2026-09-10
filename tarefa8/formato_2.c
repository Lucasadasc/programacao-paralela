/**
 * @file formato_2.c
 * @brief Estimativa estocástica de Pi - Vetor compartilhado e acumulação serial.
 * 
 * OBJETIVO DIDÁTICO (Formato 2):
 * 1. Cada thread escreve seus acertos em uma posição distinta de um vetor compartilhado:
 *    - Um vetor de tamanho suficiente para as threads ativas é alocado e compartilhado (`shared(acertos)`).
 *    - Cada thread identifica seu índice através de `omp_get_thread_num()` e escreve seus acertos
 *      em `acertos[tid]`, eliminando a necessidade de diretivas de sincronização (como `#pragma omp critical`).
 * 
 * 2. Acumulação em laço serial após a região paralela:
 *    - Após a finalização da região paralela (barreira implícita), a thread principal
 *      executa um laço serial somando as posições do vetor para obter o total de pontos
 *      dentro do círculo.
 * 
 * 3. Observação do Falso Compartilhamento (False Sharing):
 *    - Os acertos são incrementados diretamente em `acertos[tid]++` dentro do laço paralelo.
 *    - Como posições adjacentes residem na mesma linha de cache (64 bytes),
 *      ocorrem frequentes invalidações de cache entre os núcleos da CPU.
 * 
 * Compilação:
 *   gcc -fopenmp -O3 formato_2.c -o formato_2 -lm
 * Execução:
 *   ./formato_2 [numero_de_pontos]
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define PI 3.14159265358979323846

int main(int argc, char *argv[]) {
    long long total_pontos = 10000000LL; // 10 milhões de pontos
    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: O numero de pontos deve ser positivo.\n");
            return 1;
        }
    }

    int max_threads = omp_get_max_threads();
    printf("Threads ativas : %d\n", max_threads);
    printf("Total de pontos: %lld\n", total_pontos);

    // Vetor compartilhado onde cada thread armazenará seus acertos
    long long *acertos = (long long *)calloc(max_threads, sizeof(long long));
    if (acertos == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para o vetor de acertos.\n");
        return 1;
    }

    long long pontos_no_circulo_total = 0;

    srand((unsigned int)time(NULL));

    double t_inicio = omp_get_wtime();

    // Cada thread escreve diretamente na sua posição do vetor compartilhado dentro do loop
    #pragma omp parallel shared(total_pontos, acertos)
    {
        int tid = omp_get_thread_num();

        double x, y, dist_sq; // Variáveis locais à thread (privadas)

        // Distribuindo as iterações do loop entre as threads
        #pragma omp for private(x, y, dist_sq)
        for (long long i = 0; i < total_pontos; i++) {
            x = (double)rand() / (double)RAND_MAX;
            y = (double)rand() / (double)RAND_MAX;
            dist_sq = x * x + y * y;

            if (dist_sq <= 1.0) {
                acertos[tid]++; // Incremento direto no vetor compartilhado (provoca falso compartilhamento)
            }
        }
    }

    // Acumulação em um laço serial após a região paralela
    for (int t = 0; t < max_threads; t++) {
        pontos_no_circulo_total += acertos[t];
    }

    double t_fim = omp_get_wtime();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo_total / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - PI);

    printf("Pontos no circulo : %lld / %lld\n", pontos_no_circulo_total, total_pontos);
    printf("Pi Estimado       : %.10f\n", pi_estimado);
    printf("Pi Real (PI)      : %.10f\n", PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    free(acertos);

    return 0;
}
