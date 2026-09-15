/**
 * @file 1_sections.c
 * @brief Paralelização por Decomposição Funcional com '#pragma omp sections'.
 *
 * Disciplina: Programação Paralela - UFRN
 * Tarefa 11: Navier-Stokes (Apenas Viscosidade)
 *
 * CONCEITO EXPLORADO:
 * No problema de Navier-Stokes considerando puramente a difusão viscosa:
 *   ∂u/∂t = ν ∇²u
 *   ∂v/∂t = ν ∇²v
 *
 * No avanço temporal explícito (FTCS), a atualização da componente 'u' no instante
 * (n+1) depende exclusivamente do campo 'u' no instante n. Da mesma forma, a atualização
 * de 'v' depende apenas de 'v' no instante n.
 *
 * Portanto, as duas componentes são MATEMATICAMENTE INDEPENDENTES dentro do mesmo passo de tempo!
 * Podemos atribuir a computação do campo 'u' para uma seção e a computação do campo 'v'
 * para outra seção, executando-as em paralelo através de:
 *   #pragma omp parallel sections
 *   {
 *       #pragma omp section
 *       { atualizar_u(); }
 *
 *       #pragma omp section
 *       { atualizar_v(); }
 *   }
 *
 * LIMITAÇÃO TEÓRICA DE '#pragma omp sections':
 * - Como existem apenas 2 seções de trabalho computacional, o speedup máximo teórico
 *   é limitado a 2x (Lei de Amdahl aplicada à decomposição funcional), independentemente
 *   de termos 4, 8 ou 16 threads disponíveis na máquina!
 *
 * Compilação:
 *   gcc -std=c17 -O3 -fopenmp 1_sections.c -o 1_sections -lm
 * Execução:
 *   ./1_sections [tamanho_malha N] [passos_tempo] [num_threads]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define IDX(i, j, ny) ((i) * (ny) + (j))

void inicializar_campos(int nx, int ny, double dx, double dy, double lx, double ly,
                        double *u, double *v) {
    double cx = lx / 2.0;
    double cy = ly / 2.0;
    double sigma2 = 2.0 * (lx * 0.08) * (lx * 0.08);

    for (int i = 0; i < nx; i++) {
        double x = i * dx;
        for (int j = 0; j < ny; j++) {
            double y = j * dy;
            double dist2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            double pert = 2.0 * exp(-dist2 / sigma2);
            int idx = IDX(i, j, ny);
            u[idx] = pert;
            v[idx] = pert * 0.5;
        }
    }
}

void atualizar_u_stencil(int nx, int ny, const double * restrict u, double * restrict u_new,
                         double coef_x, double coef_y) {
    for (int i = 1; i < nx - 1; i++) {
        for (int j = 1; j < ny - 1; j++) {
            int c = IDX(i, j, ny);
            double lap_x = (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * coef_x;
            double lap_y = (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * coef_y;
            u_new[c] = u[c] + lap_x + lap_y;
        }
    }
    /* Condições de contorno Dirichlet para u */
    for (int i = 0; i < nx; i++) {
        u_new[IDX(i, 0, ny)] = 0.0;
        u_new[IDX(i, ny - 1, ny)] = 0.0;
    }
    for (int j = 0; j < ny; j++) {
        u_new[IDX(0, j, ny)] = 0.0;
        u_new[IDX(nx - 1, j, ny)] = 0.0;
    }
}

void atualizar_v_stencil(int nx, int ny, const double * restrict v, double * restrict v_new,
                         double coef_x, double coef_y) {
    for (int i = 1; i < nx - 1; i++) {
        for (int j = 1; j < ny - 1; j++) {
            int c = IDX(i, j, ny);
            double lap_x = (v[IDX(i+1, j, ny)] - 2.0 * v[c] + v[IDX(i-1, j, ny)]) * coef_x;
            double lap_y = (v[IDX(i, j+1, ny)] - 2.0 * v[c] + v[IDX(i, j-1, ny)]) * coef_y;
            v_new[c] = v[c] + lap_x + lap_y;
        }
    }
    /* Condições de contorno Dirichlet para v */
    for (int i = 0; i < nx; i++) {
        v_new[IDX(i, 0, ny)] = 0.0;
        v_new[IDX(i, ny - 1, ny)] = 0.0;
    }
    for (int j = 0; j < ny; j++) {
        v_new[IDX(0, j, ny)] = 0.0;
        v_new[IDX(nx - 1, j, ny)] = 0.0;
    }
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 256;
    int passos = (argc > 2) ? atoi(argv[2]) : 200;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();

    omp_set_num_threads(threads);

    double lx = 1.0, ly = 1.0;
    double dx = lx / (N - 1);
    double dy = ly / (N - 1);
    double nu = 0.05;
    double dt = 0.8 * ((dx < dy ? dx : dy) * (dx < dy ? dx : dy)) / (4.0 * nu);
    double coef_x = nu * dt / (dx * dx);
    double coef_y = nu * dt / (dy * dy);

    size_t total = (size_t)N * N;
    double *u = (double *)malloc(total * sizeof(double));
    double *u_new = (double *)malloc(total * sizeof(double));
    double *v = (double *)malloc(total * sizeof(double));
    double *v_new = (double *)malloc(total * sizeof(double));

    inicializar_campos(N, N, dx, dy, lx, ly, u, v);

    printf("====================================================================\n");
    printf(" PROGRAMACAO PARALELA - TAREFA 11: #pragma omp sections\n");
    printf("====================================================================\n");
    printf("Malha: %dx%d | Passos: %d | Threads configuradas: %d\n", N, N, passos, threads);
    printf("Decomposicao Funcional: Secao 1 (Componente U) | Secao 2 (Componente V)\n");

    double t0 = omp_get_wtime();

    for (int p = 0; p < passos; p++) {
        #pragma omp parallel sections shared(u, u_new, v, v_new)
        {
            #pragma omp section
            {
                atualizar_u_stencil(N, N, u, u_new, coef_x, coef_y);
            }

            #pragma omp section
            {
                atualizar_v_stencil(N, N, v, v_new, coef_x, coef_y);
            }
        }

        /* Troca de ponteiros */
        double *tmp_u = u; u = u_new; u_new = tmp_u;
        double *tmp_v = v; v = v_new; v_new = tmp_v;
    }

    double tempo_total = omp_get_wtime() - t0;

    /* Checksum para validar integridade matemática */
    double soma_u = 0.0, max_u = 0.0;
    for (size_t k = 0; k < total; k++) {
        soma_u += u[k];
        if (u[k] > max_u) max_u = u[k];
    }

    printf("\nResultado da Execucao com Sections:\n");
    printf("  Tempo de Execucao  : %.6f segundos\n", tempo_total);
    printf("  Pico Final de U    : %.6f m/s\n", max_u);
    printf("  Soma Total de U    : %.6f\n", soma_u);
    printf("  Observacao Didatica: O speedup de 'sections' e limitado a no maximo 2x,\n");
    printf("                       pois ha apenas 2 tarefas independentes (U e V).\n");
    printf("====================================================================\n");

    free(u); free(u_new); free(v); free(v_new);
    return 0;
}
