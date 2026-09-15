/**
 * @file 5_collapse.c
 * @brief Estudo comparativo da cláusula 'collapse(2)'.
 *
 * Disciplina: Programação Paralela - UFRN
 * Tarefa 11: Navier-Stokes (Apenas Viscosidade)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>

#define IDX(i, j, ny) ((i) * (ny) + (j))

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

double rodar_simulacao(int N, int passos, double coef_x, double coef_y, bool usar_collapse) {
    size_t total = (size_t)N * N;
    double * restrict u = (double *)malloc(total * sizeof(double));
    double * restrict u_new = (double *)malloc(total * sizeof(double));
    inicializar(N, N, 1.0/(N-1), 1.0/(N-1), u);

    double t0 = omp_get_wtime();

    for (int p = 0; p < passos; p++) {
        if (usar_collapse) {
            #pragma omp parallel for collapse(2) schedule(static)
            for (int i = 1; i < N - 1; i++) {
                for (int j = 1; j < N - 1; j++) {
                    int c = IDX(i, j, N);
                    u_new[c] = u[c] + (u[IDX(i+1, j, N)] - 2.0 * u[c] + u[IDX(i-1, j, N)]) * coef_x
                                    + (u[IDX(i, j+1, N)] - 2.0 * u[c] + u[IDX(i, j-1, N)]) * coef_y;
                }
            }
        } else {
            #pragma omp parallel for schedule(static)
            for (int i = 1; i < N - 1; i++) {
                for (int j = 1; j < N - 1; j++) {
                    int c = IDX(i, j, N);
                    u_new[c] = u[c] + (u[IDX(i+1, j, N)] - 2.0 * u[c] + u[IDX(i-1, j, N)]) * coef_x
                                    + (u[IDX(i, j+1, N)] - 2.0 * u[c] + u[IDX(i, j-1, N)]) * coef_y;
                }
            }
        }

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            u_new[IDX(i, 0, N)] = 0.0; u_new[IDX(i, N - 1, N)] = 0.0;
            u_new[IDX(0, i, N)] = 0.0; u_new[IDX(N - 1, i, N)] = 0.0;
        }

        double *tmp = u; u = u_new; u_new = tmp;
    }

    double tempo = omp_get_wtime() - t0;
    free(u); free(u_new);
    return tempo;
}

int main(int argc, char *argv[]) {
    int N       = (argc > 1) ? atoi(argv[1]) : 256;
    int passos  = (argc > 2) ? atoi(argv[2]) : 200;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();

    omp_set_num_threads(threads);

    double dx = 1.0 / (N - 1), dy = 1.0 / (N - 1);
    double nu = 0.05;
    double dt = 0.8 * (dx * dx) / (4.0 * nu);
    double coef_x = nu * dt / (dx * dx), coef_y = nu * dt / (dy * dy);

    printf("====================================================================\n");
    printf(" TAREFA 11: Impacto de collapse(2)\n");
    printf(" Malha: %dx%d | Passos: %d | Threads: %d\n", N, N, passos, threads);
    printf("====================================================================\n");

    double t_sem = rodar_simulacao(N, passos, coef_x, coef_y, false);
    double t_com = rodar_simulacao(N, passos, coef_x, coef_y, true);

    printf("Tempo SEM collapse : %.6f s\n", t_sem);
    printf("Tempo COM collapse : %.6f s (Ganho: %.2f%%)\n", t_com, ((t_sem - t_com) / t_sem) * 100.0);
    printf("====================================================================\n");

    return 0;
}
