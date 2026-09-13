/**
 * @file atomic.c
 * @brief Estimativa de Pi com contador compartilhado e operacao atomica.
 *
 * Compilacao:
 *   gcc -std=c17 -fopenmp -O3 atomic.c -o atomic -lm
 * Execucao:
 *   ./atomic [numero_de_pontos]
 */

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846

int main(int argc, char **argv) {
    long long total_pontos = 10000000LL;

    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: o numero de pontos deve ser positivo.\n");
            return EXIT_FAILURE;
        }
    }

    long long pontos_no_circulo = 0;
    double t_inicio = omp_get_wtime();

    #pragma omp parallel shared(total_pontos, pontos_no_circulo)
    {
        unsigned int seed = (unsigned int)time(NULL) ^
            (unsigned int)(omp_get_thread_num() + 1);

        #pragma omp for
        for (long long i = 0; i < total_pontos; i++) {
            double x = (double)rand_r(&seed) / (double)RAND_MAX;
            double y = (double)rand_r(&seed) / (double)RAND_MAX;

            if (x * x + y * y <= 1.0) {
                #pragma omp atomic
                pontos_no_circulo++;
            }
        }
    }

    double tempo_total = omp_get_wtime() - t_inicio;
    double pi_estimado = 4.0 * (double)pontos_no_circulo / (double)total_pontos;

    printf("Threads ativas : %d\n", omp_get_max_threads());
    printf("Total de pontos: %lld\n", total_pontos);
    printf("Pontos no circulo : %lld / %lld\n", pontos_no_circulo, total_pontos);
    printf("Pi Estimado       : %.10f\n", pi_estimado);
    printf("Pi Real (PI)      : %.10f\n", PI);
    printf("Erro Absoluto     : %.10e\n", fabs(pi_estimado - PI));
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    return 0;
}
