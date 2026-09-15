/**
 * @file benchmark_completo.c
 * @brief Benchmark Unificado de Cláusulas OpenMP na Simulação de Navier-Stokes.
 *
 * Disciplina: Programação Paralela - UFRN
 * Tarefa 11: Navier-Stokes (Apenas Viscosidade)
 *
 * Avalia de forma sistemática e padronizada:
 * - Sequencial de Referência
 * - #pragma omp sections (U e V)
 * - schedule(static) com chunks 1, 8, 32, 64 e padrão
 * - schedule(dynamic) com chunks 1, 8, 32, 64
 * - schedule(guided) com chunks 8, 32 e padrão
 * - collapse(2)
 * - #pragma omp simd
 * - collapse(2) + simd
 *
 * Gera saída formatada em tabela Markdown para inclusão direta em relatórios.
 *
 * Compilação:
 *   gcc -std=c17 -O3 -fopenmp -mavx2 -mfma benchmark_completo.c -o benchmark_completo -lm
 * Execução:
 *   ./benchmark_completo [tamanho_malha N] [passos_tempo] [num_threads]
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <omp.h>

#define IDX(i, j, ny) ((i) * (ny) + (j))

typedef struct {
    int nx, ny, passos, threads;
    double dx, dy, nu, dt, coef_x, coef_y;
    double *u, *u_new, *v, *v_new;
} Ambiente;

Ambiente *criar_ambiente(int n, int passos, int threads) {
    Ambiente *env = (Ambiente *)malloc(sizeof(Ambiente));
    env->nx = n; env->ny = n;
    env->passos = passos; env->threads = threads;
    env->dx = 1.0 / (n - 1); env->dy = 1.0 / (n - 1);
    env->nu = 0.05;
    env->dt = 0.8 * (env->dx * env->dx) / (4.0 * env->nu);
    env->coef_x = env->nu * env->dt / (env->dx * env->dx);
    env->coef_y = env->nu * env->dt / (env->dy * env->dy);

    size_t total = (size_t)n * n;
    env->u = (double *)malloc(total * sizeof(double));
    env->u_new = (double *)malloc(total * sizeof(double));
    env->v = (double *)malloc(total * sizeof(double));
    env->v_new = (double *)malloc(total * sizeof(double));
    return env;
}

void liberar_ambiente(Ambiente *env) {
    free(env->u); free(env->u_new); free(env->v); free(env->v_new);
    free(env);
}

void resetar_campos(Ambiente *env) {
    double cx = 0.5, cy = 0.5;
    double sigma2 = 2.0 * (0.08) * (0.08);
    for (int i = 0; i < env->nx; i++) {
        double x = i * env->dx;
        for (int j = 0; j < env->ny; j++) {
            double y = j * env->dy;
            double dist2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            double val = 2.0 * exp(-dist2 / sigma2);
            int idx = IDX(i, j, env->ny);
            env->u[idx] = val;
            env->v[idx] = val * 0.5;
            env->u_new[idx] = val;
            env->v_new[idx] = val * 0.5;
        }
    }
}

/* 1. Sequencial de Referência */
double bench_sequencial(Ambiente *env) {
    resetar_campos(env);
    int nx = env->nx, ny = env->ny;
    double cx = env->coef_x, cy = env->coef_y;
    double *u = env->u, *u_new = env->u_new;

    double t0 = omp_get_wtime();
    for (int p = 0; p < env->passos; p++) {
        for (int i = 1; i < nx - 1; i++) {
            for (int j = 1; j < ny - 1; j++) {
                int c = IDX(i, j, ny);
                u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                  (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
            }
        }
        for (int i = 0; i < nx; i++) { u_new[IDX(i, 0, ny)] = 0.0; u_new[IDX(i, ny - 1, ny)] = 0.0; }
        for (int j = 0; j < ny; j++) { u_new[IDX(0, j, ny)] = 0.0; u_new[IDX(nx - 1, j, ny)] = 0.0; }
        double *tmp = u; u = u_new; u_new = tmp;
    }
    return omp_get_wtime() - t0;
}

/* 2. Sections (U e V) */
double bench_sections(Ambiente *env) {
    resetar_campos(env);
    int nx = env->nx, ny = env->ny;
    double cx = env->coef_x, cy = env->coef_y;
    double *u = env->u, *u_new = env->u_new;
    double *v = env->v, *v_new = env->v_new;

    double t0 = omp_get_wtime();
    for (int p = 0; p < env->passos; p++) {
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                for (int i = 1; i < nx - 1; i++) {
                    for (int j = 1; j < ny - 1; j++) {
                        int c = IDX(i, j, ny);
                        u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                          (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                    }
                }
                for (int i = 0; i < nx; i++) { u_new[IDX(i, 0, ny)] = 0.0; u_new[IDX(i, ny-1, ny)] = 0.0; }
                for (int j = 0; j < ny; j++) { u_new[IDX(0, j, ny)] = 0.0; u_new[IDX(nx-1, j, ny)] = 0.0; }
            }
            #pragma omp section
            {
                for (int i = 1; i < nx - 1; i++) {
                    for (int j = 1; j < ny - 1; j++) {
                        int c = IDX(i, j, ny);
                        v_new[c] = v[c] + (v[IDX(i+1, j, ny)] - 2.0 * v[c] + v[IDX(i-1, j, ny)]) * cx +
                                          (v[IDX(i, j+1, ny)] - 2.0 * v[c] + v[IDX(i, j-1, ny)]) * cy;
                    }
                }
                for (int i = 0; i < nx; i++) { v_new[IDX(i, 0, ny)] = 0.0; v_new[IDX(i, ny-1, ny)] = 0.0; }
                for (int j = 0; j < ny; j++) { v_new[IDX(0, j, ny)] = 0.0; v_new[IDX(nx-1, j, ny)] = 0.0; }
            }
        }
        double *tmp_u = u; u = u_new; u_new = tmp_u;
        double *tmp_v = v; v = v_new; v_new = tmp_v;
    }
    return omp_get_wtime() - t0;
}

/* 3. Helper genérico para schedules */
typedef enum { SCHED_STATIC_DEF, SCHED_STATIC_CHUNK, SCHED_DYN_CHUNK, SCHED_GUIDED_CHUNK,
               SCHED_COLLAPSE, SCHED_SIMD, SCHED_COLLAPSE_SIMD } TipoTeste;

double bench_loop(Ambiente *env, TipoTeste tipo, int chunk) {
    resetar_campos(env);
    int nx = env->nx, ny = env->ny;
    double cx = env->coef_x, cy = env->coef_y;
    double * restrict u = env->u, * restrict u_new = env->u_new;

    double t0 = omp_get_wtime();
    for (int p = 0; p < env->passos; p++) {
        switch (tipo) {
            case SCHED_STATIC_DEF:
                #pragma omp parallel for schedule(static)
                for (int i = 1; i < nx - 1; i++) {
                    for (int j = 1; j < ny - 1; j++) {
                        int c = IDX(i, j, ny);
                        u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                          (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                    }
                }
                break;

            case SCHED_STATIC_CHUNK:
                if (chunk == 1) {
                    #pragma omp parallel for schedule(static, 1)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else if (chunk == 8) {
                    #pragma omp parallel for schedule(static, 8)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else if (chunk == 32) {
                    #pragma omp parallel for schedule(static, 32)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else {
                    #pragma omp parallel for schedule(static, 64)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                }
                break;

            case SCHED_DYN_CHUNK:
                if (chunk == 1) {
                    #pragma omp parallel for schedule(dynamic, 1)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else if (chunk == 8) {
                    #pragma omp parallel for schedule(dynamic, 8)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else if (chunk == 32) {
                    #pragma omp parallel for schedule(dynamic, 32)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else {
                    #pragma omp parallel for schedule(dynamic, 64)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                }
                break;

            case SCHED_GUIDED_CHUNK:
                if (chunk == 0) {
                    #pragma omp parallel for schedule(guided)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else if (chunk == 8) {
                    #pragma omp parallel for schedule(guided, 8)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                } else {
                    #pragma omp parallel for schedule(guided, 32)
                    for (int i = 1; i < nx - 1; i++) {
                        for (int j = 1; j < ny - 1; j++) {
                            int c = IDX(i, j, ny);
                            u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                              (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                        }
                    }
                }
                break;

            case SCHED_COLLAPSE:
                #pragma omp parallel for collapse(2) schedule(static)
                for (int i = 1; i < nx - 1; i++) {
                    for (int j = 1; j < ny - 1; j++) {
                        int c = IDX(i, j, ny);
                        u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                          (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                    }
                }
                break;

            case SCHED_SIMD:
                #pragma omp parallel for schedule(static)
                for (int i = 1; i < nx - 1; i++) {
                    #pragma omp simd
                    for (int j = 1; j < ny - 1; j++) {
                        int c = IDX(i, j, ny);
                        u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                          (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                    }
                }
                break;

            case SCHED_COLLAPSE_SIMD:
                #pragma omp parallel for collapse(2) schedule(static)
                for (int i = 1; i < nx - 1; i++) {
                    for (int j = 1; j < ny - 1; j++) {
                        int c = IDX(i, j, ny);
                        u_new[c] = u[c] + (u[IDX(i+1, j, ny)] - 2.0 * u[c] + u[IDX(i-1, j, ny)]) * cx +
                                          (u[IDX(i, j+1, ny)] - 2.0 * u[c] + u[IDX(i, j-1, ny)]) * cy;
                    }
                }
                break;
        }

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < nx; i++) { u_new[IDX(i, 0, ny)] = 0.0; u_new[IDX(i, ny-1, ny)] = 0.0; }
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < ny; j++) { u_new[IDX(0, j, ny)] = 0.0; u_new[IDX(nx-1, j, ny)] = 0.0; }

        double *tmp = u; u = u_new; u_new = tmp;
    }
    return omp_get_wtime() - t0;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 256;
    int passos = (argc > 2) ? atoi(argv[2]) : 200;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();

    omp_set_num_threads(threads);
    Ambiente *env = criar_ambiente(N, passos, threads);

    printf("# Benchmark de Desempenho OpenMP - Tarefa 11\n");
    printf("- **Malha 2D:** %d x %d (%d pontos)\n", N, N, N * N);
    printf("- **Passos Temporais:** %d\n", passos);
    printf("- **Threads OpenMP:** %d (Processadores lógicos disponíveis: %d)\n\n",
           threads, omp_get_num_procs());

    /* Linha de base sequencial */
    double t_seq = bench_sequencial(env);

    printf("| Configuração OpenMP | Cláusulas / Parâmetros | Threads | Tempo (s) | Speedup | Eficiência (%%) |\n");
    printf("| :--- | :--- | :---: | :---: | :---: | :---: |\n");
    printf("| **Sequencial (Baseline)** | `Nenhum` | 1 | %.6f | 1.00x | 100.0%% |\n", t_seq);

    struct {
        const char *nome;
        const char *clausula;
        TipoTeste tipo;
        int chunk;
        bool is_sections;
    } testes[] = {
        {"Sections", "#pragma omp sections (U e V)", SCHED_STATIC_DEF, 0, true},
        {"Static Padrão", "schedule(static)", SCHED_STATIC_DEF, 0, false},
        {"Static Chunk 64", "schedule(static, 64)", SCHED_STATIC_CHUNK, 64, false},
        {"Static Chunk 32", "schedule(static, 32)", SCHED_STATIC_CHUNK, 32, false},
        {"Static Chunk 8", "schedule(static, 8)", SCHED_STATIC_CHUNK, 8, false},
        {"Static Chunk 1", "schedule(static, 1)", SCHED_STATIC_CHUNK, 1, false},
        {"Dynamic Chunk 64", "schedule(dynamic, 64)", SCHED_DYN_CHUNK, 64, false},
        {"Dynamic Chunk 32", "schedule(dynamic, 32)", SCHED_DYN_CHUNK, 32, false},
        {"Dynamic Chunk 8", "schedule(dynamic, 8)", SCHED_DYN_CHUNK, 8, false},
        {"Dynamic Chunk 1", "schedule(dynamic, 1)", SCHED_DYN_CHUNK, 1, false},
        {"Guided Padrão", "schedule(guided)", SCHED_GUIDED_CHUNK, 0, false},
        {"Guided Chunk 32", "schedule(guided, 32)", SCHED_GUIDED_CHUNK, 32, false},
        {"Guided Chunk 8", "schedule(guided, 8)", SCHED_GUIDED_CHUNK, 8, false},
        {"Collapse", "collapse(2) schedule(static)", SCHED_COLLAPSE, 0, false},
        {"SIMD", "#pragma omp simd", SCHED_SIMD, 0, false},
        {"Collapse + SIMD", "collapse(2) + SIMD", SCHED_COLLAPSE_SIMD, 0, false},
    };

    int num_testes = sizeof(testes) / sizeof(testes[0]);
    for (int i = 0; i < num_testes; i++) {
        double t;
        if (testes[i].is_sections) {
            t = bench_sections(env);
        } else {
            t = bench_loop(env, testes[i].tipo, testes[i].chunk);
        }
        double sp = t_seq / t;
        double ef = (sp / threads) * 100.0;
        printf("| %-20s | `%-28s` | %d | %.6f | %5.2fx | %6.2f%% |\n",
               testes[i].nome, testes[i].clausula, threads, t, sp, ef);
    }

    liberar_ambiente(env);
    return 0;
}
