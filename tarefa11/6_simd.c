/**
 * @file 6_simd.c
 * @brief Vetorização Explícita em Nível de Instrução com '#pragma omp simd'.
 *
 * Disciplina: Programação Paralela - UFRN
 * Tarefa 11: Navier-Stokes (Apenas Viscosidade)
 *
 * CONCEITO EXPLORADO:
 * Enquanto '#pragma omp parallel for' explora o paralelismo de múltiplos núcleos (multi-core),
 * a diretiva '#pragma omp simd' instrui o compilador a explorar o paralelismo de dados
 * no nível de instrução do processador (Single Instruction, Multiple Data - SIMD),
 * como as instruções AVX2 (256 bits, 4 doubles por ciclo) e AVX-512 (512 bits, 8 doubles por ciclo).
 *
 * REQUISITOS PARA VETORIZAÇÃO EFICIENTE DO STENCIL:
 * 1. Acesso Contíguo em Memória (Stride-1):
 *    - Na disposição Row-Major: IDX(i, j, ny) = i * ny + j.
 *    - Conforme 'j' varia no laço interno, os elementos 'u[IDX(i, j, ny)]' estão
 *      armazenados em posições consecutivas de memória física.
 *    - Isso permite carregar 4 doubles simultaneamente em registradores YMM (AVX2).
 *
 * 2. Ausência de Aliasing (Ponteiros 'restrict'):
 *    - O compilador precisa ter certeza de que 'u' (leitura) e 'u_new' (escrita)
 *      não apontam para a mesma região de memória.
 *
 * 3. Diretiva '#pragma omp for simd':
 *    - O OpenMP divide as linhas 'i' entre os núcleos da CPU e vetoriza o avanço
 *      ao longo das colunas 'j' dentro de cada núcleo.
 *
 * Compilação:
 *   gcc -std=c17 -O3 -fopenmp -mavx2 -mfma 6_simd.c -o 6_simd -lm
 * Execução:
 *   ./6_simd [tamanho_malha N] [passos_tempo] [num_threads]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define IDX(i, j, ny) ((i) * (ny) + (j))

void inicializar(int nx, int ny, double dx, double dy, double lx, double ly, double *u) {
    double cx = lx / 2.0, cy = ly / 2.0;
    double sigma2 = 2.0 * (lx * 0.08) * (lx * 0.08);
    for (int i = 0; i < nx; i++) {
        double x = i * dx;
        for (int j = 0; j < ny; j++) {
            double y = j * dy;
            double dist2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            u[IDX(i, j, ny)] = 2.0 * exp(-dist2 / sigma2);
        }
    }
}

double executar_sem_simd(int nx, int ny, int passos, double coef_x, double coef_y) {
    size_t total = (size_t)nx * ny;
    double * restrict u = (double *)malloc(total * sizeof(double));
    double * restrict u_new = (double *)malloc(total * sizeof(double));
    inicializar(nx, ny, 1.0/(nx-1), 1.0/(ny-1), 1.0, 1.0, u);

    double t0 = omp_get_wtime();

    for (int p = 0; p < passos; p++) {
        #pragma omp parallel for schedule(static)
        for (int i = 1; i < nx - 1; i++) {
            for (int j = 1; j < ny - 1; j++) {
                int c = IDX(i, j, ny);
                double lap = (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * coef_x +
                             (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * coef_y;
                u_new[c] = u[c] + lap;
            }
        }

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < nx; i++) {
            u_new[IDX(i, 0, ny)] = 0.0;
            u_new[IDX(i, ny - 1, ny)] = 0.0;
        }
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < ny; j++) {
            u_new[IDX(0, j, ny)] = 0.0;
            u_new[IDX(nx - 1, j, ny)] = 0.0;
        }

        double *tmp = u; u = u_new; u_new = tmp;
    }

    double tempo = omp_get_wtime() - t0;
    free(u); free(u_new);
    return tempo;
}

double executar_com_simd(int nx, int ny, int passos, double coef_x, double coef_y) {
    size_t total = (size_t)nx * ny;
    double * restrict u = (double *)malloc(total * sizeof(double));
    double * restrict u_new = (double *)malloc(total * sizeof(double));
    inicializar(nx, ny, 1.0/(nx-1), 1.0/(ny-1), 1.0, 1.0, u);

    double t0 = omp_get_wtime();

    for (int p = 0; p < passos; p++) {
        /* Paralelismo de threads no laço externo + vetorização SIMD no laço interno contíguo */
        #pragma omp parallel for schedule(static)
        for (int i = 1; i < nx - 1; i++) {
            #pragma omp simd
            for (int j = 1; j < ny - 1; j++) {
                int c = IDX(i, j, ny);
                double lap = (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * coef_x +
                             (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * coef_y;
                u_new[c] = u[c] + lap;
            }
        }

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < nx; i++) {
            u_new[IDX(i, 0, ny)] = 0.0;
            u_new[IDX(i, ny - 1, ny)] = 0.0;
        }
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < ny; j++) {
            u_new[IDX(0, j, ny)] = 0.0;
            u_new[IDX(nx - 1, j, ny)] = 0.0;
        }

        double *tmp = u; u = u_new; u_new = tmp;
    }

    double tempo = omp_get_wtime() - t0;
    free(u); free(u_new);
    return tempo;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 256;
    int passos = (argc > 2) ? atoi(argv[2]) : 200;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();

    omp_set_num_threads(threads);

    double dx = 1.0 / (N - 1), dy = 1.0 / (N - 1);
    double nu = 0.05;
    double dt = 0.8 * (dx * dx) / (4.0 * nu);
    double coef_x = nu * dt / (dx * dx);
    double coef_y = nu * dt / (dy * dy);

    printf("====================================================================\n");
    printf(" PROGRAMACAO PARALELA - TAREFA 11: '#pragma omp simd'\n");
    printf("====================================================================\n");
    printf("Malha: %dx%d | Passos: %d | Threads: %d\n\n", N, N, passos, threads);

    double t_sem = executar_sem_simd(N, N, passos, coef_x, coef_y);
    double t_simd = executar_com_simd(N, N, passos, coef_x, coef_y);

    printf("Resultados:\n");
    printf("  Tempo SEM simd explícito : %.6f s\n", t_sem);
    printf("  Tempo COM #pragma omp simd : %.6f s\n", t_simd);
    printf("  Ganho Vetorial            : %.2fx (redução de %.2f%% no tempo)\n",
           t_sem / t_simd, ((t_sem - t_simd) / t_sem) * 100.0);
    printf("====================================================================\n");

    return 0;
}
