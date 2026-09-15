/**
 * @file 3_schedule_dynamic.c
 * @brief Estudo da cláusula 'schedule(dynamic, CHUNK)'.
 *
 * Disciplina: Programação Paralela - UFRN
 * Tarefa 11: Navier-Stokes (Apenas Viscosidade)
 *
 * O tamanho do chunk pode ser definido:
 * 1. Na compilação via macro: gcc -DCHUNK=64 ...
 * 2. Na execução via argumento de linha de comando: ./3_schedule_dynamic N passos threads chunk
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define IDX(i, j, ny) ((i) * (ny) + (j))

#ifndef CHUNK
#define CHUNK 32 /* Valor padrão caso não seja passado -DCHUNK na compilação */
#endif

void inicializar(int nx, int ny, double dx, double dy, double *u) {
    double cx = 0.5, cy = 0.5, sigma2 = 2.0 * (0.08 * 0.08);
    for (int i = 0; i < nx; i++) {
        double x = i * dx;
        for (int j = 0; j < ny; j++) {
            double y = j * dy;
            double dist2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            u[IDX(i, j, ny)] = 2.0 * exp(-dist2 / sigma2);
        }
    }
}

int main(int argc, char *argv[]) {
    int N       = (argc > 1) ? atoi(argv[1]) : 256;
    int passos  = (argc > 2) ? atoi(argv[2]) : 200;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();
    int chunk   = (argc > 4) ? atoi(argv[4]) : CHUNK;

    omp_set_num_threads(threads);

    double dx = 1.0 / (N - 1), dy = 1.0 / (N - 1);
    double nu = 0.05;
    double dt = 0.8 * (dx * dx) / (4.0 * nu);
    double coef_x = nu * dt / (dx * dx), coef_y = nu * dt / (dy * dy);

    size_t total = (size_t)N * N;
    double * restrict u = (double *)malloc(total * sizeof(double));
    double * restrict u_new = (double *)malloc(total * sizeof(double));
    inicializar(N, N, dx, dy, u);

    printf("====================================================================\n");
    printf(" TAREFA 11: schedule(dynamic, chunk)\n");
    printf(" Malha: %dx%d | Passos: %d | Threads: %d | Chunk: %d\n", N, N, passos, threads, chunk);
    printf(" (Dica: altere com -DCHUNK=%d na compilacao ou passe como 4º argumento)\n", chunk);
    printf("====================================================================\n");

    double t0 = omp_get_wtime();

    for (int p = 0; p < passos; p++) {
        /* Laço único e limpo com schedule(dynamic, chunk) */
        #pragma omp parallel for schedule(dynamic, chunk)
        for (int i = 1; i < N - 1; i++) {
            for (int j = 1; j < N - 1; j++) {
                int c = IDX(i, j, N);
                u_new[c] = u[c] + (u[IDX(i+1, j, N)] - 2.0 * u[c] + u[IDX(i-1, j, N)]) * coef_x
                                + (u[IDX(i, j+1, N)] - 2.0 * u[c] + u[IDX(i, j-1, N)]) * coef_y;
            }
        }

        /* Condição de contorno Dirichlet nas bordas */
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            u_new[IDX(i, 0, N)] = 0.0;
            u_new[IDX(i, N - 1, N)] = 0.0;
            u_new[IDX(0, i, N)] = 0.0;
            u_new[IDX(N - 1, i, N)] = 0.0;
        }

        double *tmp = u; u = u_new; u_new = tmp;
    }

    double tempo = omp_get_wtime() - t0;
    printf("Tempo de execucao: %.6f segundos (Pico central: %.6f m/s)\n", tempo, u[IDX(N/2, N/2, N)]);
    printf("====================================================================\n");

    free(u); free(u_new);
    return 0;
}
