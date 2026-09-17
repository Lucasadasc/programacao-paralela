#include "fluido.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

GradeFluido *criar_grade(int nx, int ny, double lx, double ly, double nu, int threads) {
    GradeFluido *grade = malloc(sizeof(*grade));
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

    double h_min = (grade->dx < grade->dy) ? grade->dx : grade->dy;
    grade->dt = 0.8 * (h_min * h_min) / (4.0 * nu);
    grade->num_passos = 400;

    size_t total = (size_t)nx * ny;
    grade->u_atual = calloc(total, sizeof(double));
    grade->u_prox = calloc(total, sizeof(double));
    grade->v_atual = calloc(total, sizeof(double));
    grade->v_prox = calloc(total, sizeof(double));

    if (!grade->u_atual || !grade->u_prox || !grade->v_atual || !grade->v_prox) {
        fprintf(stderr, "Erro ao alocar memoria para os campos de velocidade.\n");
        exit(EXIT_FAILURE);
    }

    return grade;
}

void liberar_grade(GradeFluido *grade) {
    if (!grade) return;
    free(grade->u_atual);
    free(grade->u_prox);
    free(grade->v_atual);
    free(grade->v_prox);
    free(grade);
}

void aplicar_condicoes_contorno(int tamanho_malha, double *campo) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < tamanho_malha; i++) {
        campo[FLUIDO_IDX(i, 0, tamanho_malha)] = 0.0;
        campo[FLUIDO_IDX(i, tamanho_malha - 1, tamanho_malha)] = 0.0;
        campo[FLUIDO_IDX(0, i, tamanho_malha)] = 0.0;
        campo[FLUIDO_IDX(tamanho_malha - 1, i, tamanho_malha)] = 0.0;
    }
}

void inicializar_campo_gaussiano(int nx, int ny, double lx, double ly,
                                 double dx, double dy, double amplitude,
                                 double sigma, double *campo) {
    double cx = lx / 2.0;
    double cy = ly / 2.0;
    double dois_sigma_sq = 2.0 * sigma * sigma;

    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            double x = i * dx;
            double y = j * dy;
            double dist_sq = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            campo[FLUIDO_IDX(i, j, ny)] = amplitude * exp(-dist_sq / dois_sigma_sq);
        }
    }
}

void inicializar_perturbacao_gaussiana(GradeFluido *grade, double u_base, double v_base,
                                        double amplitude, double sigma) {
    size_t total = (size_t)grade->nx * grade->ny;
    inicializar_campo_gaussiano(grade->nx, grade->ny, grade->lx, grade->ly,
                                grade->dx, grade->dy, amplitude, sigma,
                                grade->u_atual);
    for (size_t idx = 0; idx < total; idx++) {
            double pert = grade->u_atual[idx];
            grade->u_atual[idx] = u_base + pert;
            grade->v_atual[idx] = v_base + pert * 0.5;
            grade->u_prox[idx] = grade->u_atual[idx];
            grade->v_prox[idx] = grade->v_atual[idx];
    }
}
