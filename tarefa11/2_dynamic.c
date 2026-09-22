#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "navier_stokes/fluido.h"

#ifndef CHUNK
#define CHUNK 32
#endif

int main(int argc, char *argv[]) {
    int tamanho_malha = (argc > 1) ? atoi(argv[1]) : 256;
    int quantidade_passos = (argc > 2) ? atoi(argv[2]) : 200;
    int quantidade_threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();
    int tamanho_chunk = (argc > 4) ? atoi(argv[4]) : CHUNK;

    omp_set_num_threads(quantidade_threads);

    GradeFluido *grade = criar_grade(tamanho_malha, tamanho_malha, 1.0, 1.0,
                                     0.05, quantidade_threads);
    grade->num_passos = quantidade_passos;
    inicializar_perturbacao_gaussiana(grade, 0.0, 0.0, 2.0, 0.08);

    double coef_x = grade->nu * grade->dt / (grade->dx * grade->dx);
    double coef_y = grade->nu * grade->dt / (grade->dy * grade->dy);
    double * restrict campo_atual = grade->u_atual;
    double * restrict campo_proximo = grade->u_prox;

    printf("TAREFA 11: schedule(dynamic)\n");
    printf("Malha: %dx%d | Passos: %d | Threads: %d | Chunk: %d\n",
           tamanho_malha, tamanho_malha, quantidade_passos,
           quantidade_threads, tamanho_chunk);

    double tempo_inicial = omp_get_wtime();

    for (int passo = 0; passo < grade->num_passos; passo++) {
        #pragma omp parallel for schedule(dynamic, tamanho_chunk)
        for (int i = 1; i < tamanho_malha - 1; i++) {
            for (int j = 1; j < tamanho_malha - 1; j++) {
                int indice_centro = FLUIDO_IDX(i, j, tamanho_malha);
                double laplaciano_x = (campo_atual[FLUIDO_IDX(i + 1, j, tamanho_malha)] - 2.0 * campo_atual[indice_centro] + campo_atual[FLUIDO_IDX(i - 1, j, tamanho_malha)]) * coef_x;
                double laplaciano_y = (campo_atual[FLUIDO_IDX(i, j + 1, tamanho_malha)] - 2.0 * campo_atual[indice_centro] + campo_atual[FLUIDO_IDX(i, j - 1, tamanho_malha)]) * coef_y;
                double laplaciano = laplaciano_x + laplaciano_y;
                campo_proximo[indice_centro] = campo_atual[indice_centro] + laplaciano;
            }
        }
        
        // esse loop serve para aplicar as condições de contorno, garantindo que a velocidade nas bordas da malha seja zero.
        aplicar_condicoes_contorno(tamanho_malha, campo_proximo);

        double *ponteiro_temporario = campo_atual;
        campo_atual = campo_proximo;
        campo_proximo = ponteiro_temporario;
    }

    double tempo_total = omp_get_wtime() - tempo_inicial;
    printf("Tempo de execucao: %.6f s | Pico central: %.6f m/s\n",
           tempo_total,
           campo_atual[FLUIDO_IDX(tamanho_malha / 2, tamanho_malha / 2, tamanho_malha)]);

    liberar_grade(grade);
    return 0;
}
