/**
 * @file simulacao_viscosidade.c
 * @brief Simulação do movimento de um fluido usando a equação de Navier-Stokes
 *        considerando apenas os efeitos da viscosidade (difusão viscosa).
 *
 * Equação de evolução da velocidade (desprezando pressão e forças externas):
 *   ∂u/∂t = ν ∇²u = ν (∂²u/∂x² + ∂²u/∂y²)
 *   ∂v/∂t = ν ∇²v = ν (∂²v/∂x² + ∂²v/∂y²)
 *
 * Discretização espacial por diferenças finitas centrais de 2ª ordem e
 * discretização temporal por Euler explícito (FTCS - Forward-Time Central-Space).
 *
 * Critério de estabilidade CFL (Von Neumann):
 *   Δt <= (h²) / (4 * ν), onde h = min(Δx, Δy)
 *
 * Cenários implementados:
 *   1. Fluido em repouso (u = 0, v = 0) -> verificação de estabilidade (drift = 0).
 *   2. Fluido com velocidade constante (u = U0, v = V0) -> estabilidade do campo.
 *   3. Perturbação localizada (pulso gaussiano) -> observação da difusão suave.
 *   4. Demonstração completa de todos os cenários e medição de desempenho com OpenMP.
 *
 * Compilação:
 *   gcc -std=c17 -O3 -fopenmp simulacao_viscosidade.c -o simulacao_viscosidade -lm
 *
 * Execução:
 *   ./simulacao_viscosidade [cenario: 1, 2, 3 ou 4] [tamanho_malha N] [passos_tempo] [num_threads]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>

#define PI 3.14159265358979323846

/* Paleta de caracteres para visualização ASCII */
static const char DENSITY_CHARS[] = " .:-=+*#%@";
static const int NUM_DENSITY_CHARS = 10;

/* Cores ANSI para visualização vibrante no terminal */
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_RED     "\033[31m"
#define ANSI_MAGENTA "\033[35m"

typedef struct {
    int nx;              /* Número de pontos em X */
    int ny;              /* Número de pontos em Y */
    double lx;           /* Comprimento do domínio em X */
    double ly;           /* Comprimento do domínio em Y */
    double dx;           /* Espaçamento da malha em X */
    double dy;           /* Espaçamento da malha em Y */
    double nu;           /* Viscosidade cinemática */
    double dt;           /* Passo de tempo */
    int num_passos;      /* Quantidade de passos de tempo */
    int threads;         /* Número de threads OpenMP */
    double *u_atual;     /* Componente X da velocidade (instante n) */
    double *u_prox;      /* Componente X da velocidade (instante n+1) */
    double *v_atual;     /* Componente Y da velocidade (instante n) */
    double *v_prox;      /* Componente Y da velocidade (instante n+1) */
} GradeFluido;

/* Alocação e inicialização das estruturas da grade */
GradeFluido *criar_grade(int nx, int ny, double lx, double ly, double nu, int threads) {
    GradeFluido *grade = (GradeFluido *)malloc(sizeof(GradeFluido));
    if (!grade) {
        fprintf(stderr, "Erro ao alocar memoria para GradeFluido.\n");
        exit(EXIT_FAILURE);
    }

    grade->nx = nx;
    grade->ny = ny;
    grade->lx = lx;
    grade->ly = ly;
    grade->dx = lx / (nx - 1);
    grade->dy = ly / (ny - 1);
    grade->nu = nu;
    grade->threads = threads;

    /* Condição de estabilidade CFL para difusão 2D: dt <= h² / (4 * nu) */
    double h_min = (grade->dx < grade->dy) ? grade->dx : grade->dy;
    double dt_estavel = (h_min * h_min) / (4.0 * nu);
    grade->dt = 0.8 * dt_estavel; /* Fator de segurança de 80% */
    grade->num_passos = 400;      /* Padrão */

    size_t total_pontos = (size_t)nx * ny;
    grade->u_atual = (double *)calloc(total_pontos, sizeof(double));
    grade->u_prox  = (double *)calloc(total_pontos, sizeof(double));
    grade->v_atual = (double *)calloc(total_pontos, sizeof(double));
    grade->v_prox  = (double *)calloc(total_pontos, sizeof(double));

    if (!grade->u_atual || !grade->u_prox || !grade->v_atual || !grade->v_prox) {
        fprintf(stderr, "Erro ao alocar memoria para os campos de velocidade.\n");
        exit(EXIT_FAILURE);
    }

    return grade;
}

void liberar_grade(GradeFluido *grade) {
    if (grade) {
        free(grade->u_atual);
        free(grade->u_prox);
        free(grade->v_atual);
        free(grade->v_prox);
        free(grade);
    }
}

/* Macro de indexação linear para acesso rápido em matriz 2D (Row-Major) */
#define IDX(i, j, ny) ((i) * (ny) + (j))

/* Inicializa fluido em repouso */
void inicializar_repouso(GradeFluido *g) {
    size_t total = (size_t)g->nx * g->ny;
    #pragma omp parallel for schedule(static)
    for (size_t k = 0; k < total; k++) {
        g->u_atual[k] = 0.0;
        g->v_atual[k] = 0.0;
        g->u_prox[k]  = 0.0;
        g->v_prox[k]  = 0.0;
    }
}

/* Inicializa fluido com velocidade uniforme */
void inicializar_velocidade_constante(GradeFluido *g, double u0, double v0) {
    size_t total = (size_t)g->nx * g->ny;
    #pragma omp parallel for schedule(static)
    for (size_t k = 0; k < total; k++) {
        g->u_atual[k] = u0;
        g->v_atual[k] = v0;
        g->u_prox[k]  = u0;
        g->v_prox[k]  = v0;
    }
}

/* Inicializa perturbação gaussiana localizada no centro do domínio */
void inicializar_perturbacao_gaussiana(GradeFluido *g, double u_base, double v_base,
                                      double amplitude, double sigma) {
    double cx = g->lx / 2.0;
    double cy = g->ly / 2.0;
    double dois_sigma_sq = 2.0 * sigma * sigma;

    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < g->nx; i++) {
        for (int j = 0; j < g->ny; j++) {
            double x = i * g->dx;
            double y = j * g->dy;
            double dist_sq = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            double pert = amplitude * exp(-dist_sq / dois_sigma_sq);

            int idx = IDX(i, j, g->ny);
            g->u_atual[idx] = u_base + pert;
            g->v_atual[idx] = v_base + (pert * 0.5); /* Perturbação também em V */
            g->u_prox[idx]  = g->u_atual[idx];
            g->v_prox[idx]  = g->v_atual[idx];
        }
    }
}

/* Aplica condições de contorno de Dirichlet mantendo a velocidade base nas bordas */
void aplicar_condicoes_contorno_dirichlet(GradeFluido *g, double u_base, double v_base) {
    int nx = g->nx;
    int ny = g->ny;

    /* Bordas superior e inferior (j = 0 e j = ny - 1) */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < nx; i++) {
        g->u_prox[IDX(i, 0, ny)]      = u_base;
        g->v_prox[IDX(i, 0, ny)]      = v_base;
        g->u_prox[IDX(i, ny - 1, ny)] = u_base;
        g->v_prox[IDX(i, ny - 1, ny)] = v_base;
    }

    /* Bordas esquerda e direita (i = 0 e i = nx - 1) */
    #pragma omp parallel for schedule(static)
    for (int j = 0; j < ny; j++) {
        g->u_prox[IDX(0, j, ny)]      = u_base;
        g->v_prox[IDX(0, j, ny)]      = v_base;
        g->u_prox[IDX(nx - 1, j, ny)] = u_base;
        g->v_prox[IDX(nx - 1, j, ny)] = v_base;
    }
}

/**
 * Passo de avanço temporal usando Diferenças Finitas Centrais (FTCS):
 *
 * u^{n+1}_{i,j} = u^n_{i,j} + ν * Δt * [
 *     (u^n_{i+1,j} - 2*u^n_{i,j} + u^n_{i-1,j}) / Δx² +
 *     (u^n_{i,j+1} - 2*u^n_{i,j} + u^n_{i,j-1}) / Δy²
 * ]
 */
void passo_tempo_ftcs(GradeFluido *g, double u_base, double v_base) {
    int nx = g->nx;
    int ny = g->ny;
    double nu = g->nu;
    double dt = g->dt;
    double inv_dx2 = 1.0 / (g->dx * g->dx);
    double inv_dy2 = 1.0 / (g->dy * g->dy);

    double * restrict u = g->u_atual;
    double * restrict u_new = g->u_prox;
    double * restrict v = g->v_atual;
    double * restrict v_new = g->v_prox;

    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 1; i < nx - 1; i++) {
        for (int j = 1; j < ny - 1; j++) {
            int c = IDX(i, j, ny);
            int e = IDX(i + 1, j, ny);
            int w = IDX(i - 1, j, ny);
            int n = IDX(i, j + 1, ny);
            int s = IDX(i, j - 1, ny);

            /* Laplaciano da componente u */
            double lap_u = (u[e] - 2.0 * u[c] + u[w]) * inv_dx2 +
                           (u[n] - 2.0 * u[c] + u[s]) * inv_dy2;

            /* Laplaciano da componente v */
            double lap_v = (v[e] - 2.0 * v[c] + v[w]) * inv_dx2 +
                           (v[n] - 2.0 * v[c] + v[s]) * inv_dy2;

            u_new[c] = u[c] + nu * dt * lap_u;
            v_new[c] = v[c] + nu * dt * lap_v;
        }
    }

    /* Atualização das condições de contorno */
    aplicar_condicoes_contorno_dirichlet(g, u_base, v_base);

    /* Troca rápida de buffers (double-buffering com troca de ponteiros) */
    double *temp_u = g->u_atual;
    g->u_atual = g->u_prox;
    g->u_prox  = temp_u;

    double *temp_v = g->v_atual;
    g->v_atual = g->v_prox;
    g->v_prox  = temp_v;
}

/* Calcula a velocidade máxima, média e o desvio em relação ao valor de referência */
void calcular_estatisticas(const GradeFluido *g, double u_ref, double v_ref,
                           double *max_u, double *min_u, double *desvio_max, double *energia_cinetica) {
    int nx = g->nx;
    int ny = g->ny;
    double local_max_u = -1e30;
    double local_min_u = 1e30;
    double local_desvio = 0.0;
    double local_ec = 0.0;

    #pragma omp parallel for collapse(2) reduction(max: local_max_u, local_desvio) reduction(min: local_min_u) reduction(+: local_ec)
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            int idx = IDX(i, j, ny);
            double u = g->u_atual[idx];
            double v = g->v_atual[idx];

            if (u > local_max_u) local_max_u = u;
            if (u < local_min_u) local_min_u = u;

            double diff_u = fabs(u - u_ref);
            double diff_v = fabs(v - v_ref);
            double diff = (diff_u > diff_v) ? diff_u : diff_v;
            if (diff > local_desvio) local_desvio = diff;

            local_ec += 0.5 * (u * u + v * v);
        }
    }

    *max_u = local_max_u;
    *min_u = local_min_u;
    *desvio_max = local_desvio;
    *energia_cinetica = local_ec / (nx * ny);
}

/* Renderiza mapa ASCII no terminal com cores ANSI para acompanhar a difusão */
void renderizar_mapa_ascii(const GradeFluido *g, const char *titulo, int passo) {
    int linhas_term = 18;
    int colunas_term = 36;
    int nx = g->nx;
    int ny = g->ny;

    double max_mag = 0.0;
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            int idx = IDX(i, j, ny);
            double mag = sqrt(g->u_atual[idx] * g->u_atual[idx] + g->v_atual[idx] * g->v_atual[idx]);
            if (mag > max_mag) max_mag = mag;
        }
    }
    if (max_mag < 1e-12) max_mag = 1.0;

    printf("\n%s--- %s (Passo: %d | Tempo: %.4f s) ---%s\n", ANSI_BOLD, titulo, passo, passo * g->dt, ANSI_RESET);
    printf("┌");
    for (int c = 0; c < colunas_term; c++) printf("─");
    printf("┐\n");

    for (int r = 0; r < linhas_term; r++) {
        printf("│");
        int j = (int)((double)(linhas_term - 1 - r) / (linhas_term - 1) * (ny - 1));
        for (int c = 0; c < colunas_term; c++) {
            int i = (int)((double)c / (colunas_term - 1) * (nx - 1));
            int idx = IDX(i, j, ny);
            double mag = sqrt(g->u_atual[idx] * g->u_atual[idx] + g->v_atual[idx] * g->v_atual[idx]);
            double norm = mag / max_mag;
            if (norm > 1.0) norm = 1.0;
            if (norm < 0.0) norm = 0.0;

            int char_idx = (int)(norm * (NUM_DENSITY_CHARS - 1));
            char ch = DENSITY_CHARS[char_idx];

            /* Escolha de cor baseada na magnitude */
            if (norm > 0.75) {
                printf("%s%c%s", ANSI_RED, ch, ANSI_RESET);
            } else if (norm > 0.5) {
                printf("%s%c%s", ANSI_YELLOW, ch, ANSI_RESET);
            } else if (norm > 0.25) {
                printf("%s%c%s", ANSI_GREEN, ch, ANSI_RESET);
            } else if (norm > 0.05) {
                printf("%s%c%s", ANSI_CYAN, ch, ANSI_RESET);
            } else {
                printf("%s%c%s", ANSI_BLUE, ch, ANSI_RESET);
            }
        }
        printf("│\n");
    }

    printf("└");
    for (int c = 0; c < colunas_term; c++) printf("─");
    printf("┘\n");
}

/* Exporta os dados do campo para arquivo CSV para geração de gráficos */
void exportar_csv(const GradeFluido *g, const char *caminho, int passo) {
    FILE *fp = (passo == 0) ? fopen(caminho, "w") : fopen(caminho, "a");
    if (!fp) {
        fprintf(stderr, "Aviso: nao foi possivel abrir %s para escrita.\n", caminho);
        return;
    }
    setvbuf(fp, NULL, _IOFBF, 1024 * 1024);

    if (passo == 0) {
        fprintf(fp, "passo,tempo,i,j,x,y,u,v,magnitude\n");
    }

    int stride = (g->nx > 128) ? (g->nx / 64) : 1;
    double t = passo * g->dt;
    for (int i = 0; i < g->nx; i += stride) {
        double x = i * g->dx;
        for (int j = 0; j < g->ny; j += stride) {
            double y = j * g->dy;
            int idx = IDX(i, j, g->ny);
            double u = g->u_atual[idx];
            double v = g->v_atual[idx];
            double mag = sqrt(u * u + v * v);
            fprintf(fp, "%d,%.6f,%d,%d,%.4f,%.4f,%.6f,%.6f,%.6f\n",
                    passo, t, i, j, x, y, u, v, mag);
        }
    }

    fclose(fp);
}

/* ========================================================================= */
/* CENÁRIOS DE SIMULAÇÃO                                                     */
/* ========================================================================= */

/**
 * Cenário 1: Fluido em Repouso
 * Verifica se um campo inicialmente parado permanece perfeitamente estável (u = 0, v = 0).
 */
void executar_cenario_repouso(GradeFluido *g) {
    printf("\n==================================================================\n");
    printf("  CENARIO 1: FLUIDO EM REPOUSO (u = 0, v = 0)\n");
    printf("  Objetivo: Verificar se o campo permanece identicamente nulo e estavel.\n");
    printf("==================================================================\n");

    inicializar_repouso(g);

    double max_u, min_u, desvio, ec;
    calcular_estatisticas(g, 0.0, 0.0, &max_u, &min_u, &desvio, &ec);
    printf("Estado Inicial : Desvio maximo = %.10e | Energia Cinetica Media = %.10e\n", desvio, ec);

    double t_inicio = omp_get_wtime();
    for (int passo = 1; passo <= g->num_passos; passo++) {
        passo_tempo_ftcs(g, 0.0, 0.0);
    }
    double tempo_exec = omp_get_wtime() - t_inicio;

    calcular_estatisticas(g, 0.0, 0.0, &max_u, &min_u, &desvio, &ec);
    printf("Apos %d passos (tempo fisico = %.4f s):\n", g->num_passos, g->num_passos * g->dt);
    printf("  - Velocidade Maxima: %.10e m/s\n", max_u);
    printf("  - Desvio Maximo    : %.10e m/s\n", desvio);
    printf("  - Energia Cinetica : %.10e J/kg\n", ec);
    printf("  - Tempo Computacao : %.6f s\n", tempo_exec);

    if (desvio < 1e-12) {
        printf("%s[SUCESSO] O campo em repouso permaneceu rigorosamente estavel (drift nulo).%s\n", ANSI_GREEN, ANSI_RESET);
    } else {
        printf("%s[FALHA] Houve instabilidade no repouso: desvio = %.4e%s\n", ANSI_RED, desvio, ANSI_RESET);
    }
}

/**
 * Cenário 2: Fluido com Velocidade Uniforme
 * Com velocidade uniforme (u = U0, v = V0), o Laplaciano ∇²u = 0, logo ∂u/∂t = 0.
 * O campo deve permanecer perfeitamente uniforme.
 */
void executar_cenario_velocidade_constante(GradeFluido *g, double u0, double v0) {
    printf("\n==================================================================\n");
    printf("  CENARIO 2: FLUIDO COM VELOCIDADE CONSTANTE (u = %.2f, v = %.2f)\n", u0, v0);
    printf("  Objetivo: Como o gradiente espacial e nulo (Laplaciano = 0), a velocidade\n");
    printf("            deve permanecer constante e estavel ao longo de todo o tempo.\n");
    printf("==================================================================\n");

    inicializar_velocidade_constante(g, u0, v0);

    double max_u, min_u, desvio, ec;
    calcular_estatisticas(g, u0, v0, &max_u, &min_u, &desvio, &ec);
    printf("Estado Inicial : Max u = %.4f | Desvio = %.10e | Energia Cinetica = %.6f\n", max_u, desvio, ec);

    double t_inicio = omp_get_wtime();
    for (int passo = 1; passo <= g->num_passos; passo++) {
        passo_tempo_ftcs(g, u0, v0);
    }
    double tempo_exec = omp_get_wtime() - t_inicio;

    calcular_estatisticas(g, u0, v0, &max_u, &min_u, &desvio, &ec);
    printf("Apos %d passos (tempo fisico = %.4f s):\n", g->num_passos, g->num_passos * g->dt);
    printf("  - Velocidade Maxima : %.10f m/s\n", max_u);
    printf("  - Velocidade Minima : %.10f m/s\n", min_u);
    printf("  - Desvio Maximo     : %.10e m/s\n", desvio);
    printf("  - Energia Cinetica  : %.6f J/kg\n", ec);
    printf("  - Tempo Computacao  : %.6f s\n", tempo_exec);

    if (desvio < 1e-12) {
        printf("%s[SUCESSO] O campo uniforme permaneceu estavel sem oscilacoes ou perdas.%s\n", ANSI_GREEN, ANSI_RESET);
    } else {
        printf("%s[ALERTA] Desvio do campo uniforme: %.4e%s\n", ANSI_YELLOW, desvio, ANSI_RESET);
    }
}

/**
 * Cenário 3: Perturbação Localizada e Difusão Suave
 * Aplica um pulso gaussiano de velocidade e observa o decaimento e alargamento suave
 * do perfil de velocidade por difusão viscosa pura.
 */
void executar_cenario_perturbacao(GradeFluido *g, const char *arquivo_csv) {
    printf("\n==================================================================\n");
    printf("  CENARIO 3: DIFUSAO VISCOSA DE UMA PERTURBACAO LOCALIZADA\n");
    printf("  Objetivo: Injetar um pulso de velocidade no centro e observar a\n");
    printf("            difusao suave e amortecimento continuo do pico.\n");
    printf("==================================================================\n");

    double u_base = 0.0;
    double v_base = 0.0;
    double amplitude = 2.0;       /* Pico adicional de 2.0 m/s */
    double sigma = g->lx * 0.08;  /* Largura inicial do pulso */

    inicializar_perturbacao_gaussiana(g, u_base, v_base, amplitude, sigma);

    if (arquivo_csv) {
        exportar_csv(g, arquivo_csv, 0);
    }

    renderizar_mapa_ascii(g, "Estado Inicial com Perturbacao Gaussiana", 0);

    double max_u_ini, min_u_ini, desvio_ini, ec_ini;
    calcular_estatisticas(g, u_base, v_base, &max_u_ini, &min_u_ini, &desvio_ini, &ec_ini);
    printf("T = 0.000 s -> Pico Maximo de Velocidade = %.6f m/s | Energia = %.6f J/kg\n",
           max_u_ini, ec_ini);

    int intervalos_exibicao = 4;
    int passo_intervalo = g->num_passos / intervalos_exibicao;
    if (passo_intervalo < 1) passo_intervalo = 1;

    double max_u_ant = max_u_ini;
    bool estritamente_decrescente = true;

    double t_inicio = omp_get_wtime();
    for (int passo = 1; passo <= g->num_passos; passo++) {
        passo_tempo_ftcs(g, u_base, v_base);

        if (passo % passo_intervalo == 0 || passo == g->num_passos) {
            double max_u, min_u, desvio, ec;
            calcular_estatisticas(g, u_base, v_base, &max_u, &min_u, &desvio, &ec);

            printf("Passo %4d | T = %.4f s | Pico Max = %.6f m/s (Decaimento: %.2f%%) | Energia = %.6f J/kg\n",
                   passo, passo * g->dt, max_u, (1.0 - (max_u / max_u_ini)) * 100.0, ec);

            if (max_u > max_u_ant + 1e-9) {
                estritamente_decrescente = false;
            }
            max_u_ant = max_u;

            if (arquivo_csv) {
                exportar_csv(g, arquivo_csv, passo);
            }

            if (passo == g->num_passos / 2) {
                renderizar_mapa_ascii(g, "Evolucao Intermediaria (Difundindo)", passo);
            }
        }
    }
    double tempo_exec = omp_get_wtime() - t_inicio;

    renderizar_mapa_ascii(g, "Estado Final (Perturbacao Suavemente Dissipada)", g->num_passos);

    double max_u_fim, min_u_fim, desvio_fim, ec_fim;
    calcular_estatisticas(g, u_base, v_base, &max_u_fim, &min_u_fim, &desvio_fim, &ec_fim);

    printf("\n--- Resumo da Difusao Viscosa ---\n");
    printf("  - Pico Inicial      : %.6f m/s\n", max_u_ini);
    printf("  - Pico Final        : %.6f m/s\n", max_u_fim);
    printf("  - Reducao Total     : %.2f%%\n", (1.0 - (max_u_fim / max_u_ini)) * 100.0);
    printf("  - Tempo Computacao  : %.6f s\n", tempo_exec);

    if (estritamente_decrescente && max_u_fim < max_u_ini) {
        printf("%s[SUCESSO] A perturbacao se difundiu de maneira perfeitamente suave e amortecida!%s\n",
               ANSI_GREEN, ANSI_RESET);
    } else {
        printf("%s[ALERTA] Houve oscilacoes inesperadas durante a difusao.%s\n", ANSI_YELLOW, ANSI_RESET);
    }
}

/**
 * Benchmark de Escalabilidade OpenMP
 */
void benchmark_openmp(int nx, int ny, int passos) {
    printf("\n==================================================================\n");
    printf("  BENCHMARK DE ESCALABILIDADE COM OPENMP\n");
    printf("  Malha: %dx%d | Passos: %d\n", nx, ny, passos);
    printf("==================================================================\n");

    int thread_counts[] = {1, 2, 4, 8};
    int num_configs = sizeof(thread_counts) / sizeof(thread_counts[0]);
    int max_disp = omp_get_num_procs();
    double t_seq = 0.0;

    printf("%-10s | %-15s | %-12s | %-12s\n", "Threads", "Tempo (s)", "Speedup", "Eficiencia");
    printf("-------------------------------------------------------------\n");

    for (int c = 0; c < num_configs; c++) {
        int th = thread_counts[c];
        if (th > max_disp * 2) continue;

        omp_set_num_threads(th);
        GradeFluido *g = criar_grade(nx, ny, 1.0, 1.0, 0.1, th);
        inicializar_perturbacao_gaussiana(g, 0.0, 0.0, 2.0, 0.08);

        double t0 = omp_get_wtime();
        for (int p = 0; p < passos; p++) {
            passo_tempo_ftcs(g, 0.0, 0.0);
        }
        double t_total = omp_get_wtime() - t0;

        if (th == 1) t_seq = t_total;
        double speedup = t_seq / t_total;
        double efici = speedup / th;

        printf("%-10d | %-15.6f | %-12.2fx | %-12.2f%%\n",
               th, t_total, speedup, efici * 100.0);

        liberar_grade(g);
    }
    printf("-------------------------------------------------------------\n");
}

/* ========================================================================= */
/* FUNÇÃO PRINCIPAL (MAIN)                                                   */
/* ========================================================================= */

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int cenario = 4;           /* Padrão: Executar demonstração completa */
    int N = 64;                /* Malha 64x64 por padrão */
    int passos = 300;          /* 300 iterações temporais */
    int threads = omp_get_max_threads();
    double nu = 0.05;          /* Viscosidade cinemática */

    if (argc > 1) cenario = atoi(argv[1]);
    if (argc > 2) N = atoi(argv[2]);
    if (argc > 3) passos = atoi(argv[3]);
    if (argc > 4) threads = atoi(argv[4]);

    if (N < 8) {
        fprintf(stderr, "Erro: O tamanho da malha deve ser pelo menos 8.\n");
        return EXIT_FAILURE;
    }
    if (threads < 1) threads = 1;
    omp_set_num_threads(threads);

    GradeFluido *grade = criar_grade(N, N, 1.0, 1.0, nu, threads);
    grade->num_passos = passos;

    printf("%s", ANSI_BOLD);
    printf("====================================================================\n");
    printf(" PROGRAMACAO PARALELA - TAREFA 11\n");
    printf(" SIMULACAO DE FLUIDO COM NAVIER-STOKES (APENAS EFEITOS DA VISCOSIDADE)\n");
    printf("====================================================================\n");
    printf("%s", ANSI_RESET);

    printf("Parametros da Simulacao:\n");
    printf("  - Malha 2D           : %d x %d (%d pontos totais)\n", N, N, N * N);
    printf("  - Dominio Fisico     : [0.0, %.2f] x [0.0, %.2f]\n", grade->lx, grade->ly);
    printf("  - Espacamento (dx,dy): %.6f m, %.6f m\n", grade->dx, grade->dy);
    printf("  - Viscosidade (nu)   : %.4f m²/s\n", grade->nu);
    printf("  - Passo Temporal (dt): %.6f s (CFL de difusao garantido)\n", grade->dt);
    printf("  - Passos Simulados   : %d (Tempo fisico total = %.4f s)\n", passos, passos * grade->dt);
    printf("  - Threads OpenMP     : %d\n", threads);

    /* Verificação explícita do critério de estabilidade CFL */
    double cfl_x = grade->nu * grade->dt / (grade->dx * grade->dx);
    double cfl_y = grade->nu * grade->dt / (grade->dy * grade->dy);
    printf("  - Fator CFL (sigma)  : sigma_x = %.4f, sigma_y = %.4f (Soma = %.4f <= 0.5)\n",
           cfl_x, cfl_y, cfl_x + cfl_y);

    if (cfl_x + cfl_y > 0.5) {
        printf("%s[AVISO CRITICO] O passo de tempo viola a estabilidade de Von Neumann!%s\n",
               ANSI_RED, ANSI_RESET);
    } else {
        printf("%s[ESTABILIDADE] Criterio CFL respeitado com margem de seguranca.%s\n",
               ANSI_GREEN, ANSI_RESET);
    }

    const char *csv_file = "difusao_simulacao.csv";

    switch (cenario) {
        case 1:
            executar_cenario_repouso(grade);
            break;
        case 2:
            executar_cenario_velocidade_constante(grade, 1.5, 0.5);
            break;
        case 3:
            executar_cenario_perturbacao(grade, csv_file);
            break;
        case 4:
        default:
            executar_cenario_repouso(grade);
            executar_cenario_velocidade_constante(grade, 1.5, 0.5);
            executar_cenario_perturbacao(grade, csv_file);
            benchmark_openmp(N, N, passos);
            break;
    }

    liberar_grade(grade);

    printf("\n%sSimulacao concluida com sucesso!%s\n", ANSI_BOLD, ANSI_RESET);
    return EXIT_SUCCESS;
}
