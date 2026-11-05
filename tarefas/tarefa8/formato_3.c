/**
 * @file formato_3.c
 * @brief Estimativa estocástica de Pi - Redução manual com 'critical' e 'rand_r'.
 * 
 * OBJETIVO DIDÁTICO (Formato 3):
 * 1. Baseado no Formato 1 (redução manual via '#pragma omp critical').
 * 
 * 2. Substituição de rand() por rand_r():
 *    - 'rand()' não é reentrante e usa travas/estado global interno, o que causa
 *      grande contenção e perda de desempenho entre as threads.
 *    - 'rand_r()' é thread-safe e recebe uma semente ('seed') privada por thread,
 *      eliminando a contenção na geração de números pseudoaleatórios.
 * 
 * Compilação:
 *   gcc -fopenmp -O3 formato_3.c -o formato_3 -lm
 * Execução:
 *   ./formato_3 [numero_de_pontos]
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

    printf("Threads ativas : %d\n", omp_get_max_threads());
    printf("Total de pontos: %lld\n", total_pontos);

    long long pontos_no_circulo_total = 0; // Compartilhada entre todas as threads
    long long contador_local;              // Será tornada 'private' para cada thread

    double t_inicio = omp_get_wtime();

    // 1. Região paralela: define o que é compartilhado e o que é privado
    #pragma omp parallel \
        shared(total_pontos, pontos_no_circulo_total) \
        private(contador_local)
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)time(NULL) + tid;

        // 'contador_local' é private, portanto inicia com LIXO DE MEMÓRIA.
        // inicializando manualmente aqui dentro:
        contador_local = 0;

        double x, y, dist_sq; // Variáveis locais à thread (privadas)

        // Distribuindo as iterações do loop entre as threads
        #pragma omp for private(x, y, dist_sq)
        for (long long i = 0; i < total_pontos; i++) {
            x = (double)rand_r(&seed) / (double)RAND_MAX;
            y = (double)rand_r(&seed) / (double)RAND_MAX;
            dist_sq = x * x + y * y;

            if (dist_sq <= 1.0) {
                contador_local++; // Incremento local sem condição de corrida
            }
        }

        // Atualizando o total global apenas uma única vez por thread
        #pragma omp critical
        {
            pontos_no_circulo_total += contador_local;
        }
    }

    double t_fim = omp_get_wtime();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo_total / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - PI);

    printf("Pontos no circulo : %lld / %lld (Correto!)\n", pontos_no_circulo_total, total_pontos);
    printf("Pi Estimado       : %.10f (Correto!)\n", pi_estimado);
    printf("Pi Real (PI)      : %.10f\n", PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    return 0;
}
