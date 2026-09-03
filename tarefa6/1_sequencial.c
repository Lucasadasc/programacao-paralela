/**
 * @file 1_sequencial.c
 * @brief Estimativa estocástica de Pi (Método de Monte Carlo) - Versão Sequencial.
 * 
 * O método de Monte Carlo sorteia N pontos aleatórios (x, y) no quadrado [0, 1] x [0, 1].
 * A fração de pontos que caem dentro do quadrante do círculo unitário (x^2 + y^2 <= 1)
 * aproxima a razão entre a área do quarto de círculo (pi/4) e a área do quadrado (1).
 * Logo, Pi ~= 4 * (pontos_no_circulo / total_pontos).
 * 
 * Compilação:
 *   gcc -O3 1_sequencial.c -o 1_sequencial -lm
 * Execução:
 *   ./1_sequencial [numero_de_pontos]
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PI 3.14159265358979323846

// Função para medir tempo de alta precisão
double tempo_agora(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
    long long total_pontos = 100000000LL; // Padrão: 100 milhões de pontos
    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: O numero de pontos deve ser positivo.\n");
            return 1;
        }
    }

    long long pontos_no_circulo = 0;
    srand((unsigned int)time(NULL));

    double t_inicio = tempo_agora();

    for (long long i = 0; i < total_pontos; i++) {
        // Gera coordenadas x e y normalizadas no intervalo [0.0, 1.0] usando rand() padrao
        double x = (double)rand() / (double)RAND_MAX;
        double y = (double)rand() / (double)RAND_MAX;

        if (x * x + y * y <= 1.0) {
            pontos_no_circulo++;
        }
    }

    double t_fim = tempo_agora();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - PI);

    printf("Total de pontos: %lld\n", total_pontos);
    printf("Pontos no circulo : %lld / %lld\n", pontos_no_circulo, total_pontos);
    printf("Pi Estimado       : %.10f\n", pi_estimado);
    printf("Pi Real (PI)    : %.10f\n", PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    return 0;
}
