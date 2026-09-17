#ifndef FLUIDO_H
#define FLUIDO_H

#include <stddef.h>

typedef struct {
    int nx;
    int ny;
    double lx;
    double ly;
    double dx;
    double dy;
    double nu;
    double dt;
    int num_passos;
    int threads;
    double *u_atual;
    double *u_prox;
    double *v_atual;
    double *v_prox;
} GradeFluido;

#define FLUIDO_IDX(i, j, ny) ((i) * (ny) + (j))

GradeFluido *criar_grade(int nx, int ny, double lx, double ly, double nu, int threads);
void liberar_grade(GradeFluido *grade);
void aplicar_condicoes_contorno(int tamanho_malha, double *campo);
void inicializar_campo_gaussiano(int nx, int ny, double lx, double ly,
                                 double dx, double dy, double amplitude,
                                 double sigma, double *campo);
void inicializar_perturbacao_gaussiana(GradeFluido *grade, double u_base, double v_base,
                                        double amplitude, double sigma);

#endif
