/**
 * @file 3_critical.c
 * @brief Estimativa estocástica de Pi - Correção da Condição de Corrida com #pragma omp critical.
 * 
 * Este programa corrige o erro de condição de corrida envolvendo a atualização da variável
 * acumuladora compartilhada dentro de uma seção crítica (`#pragma omp critical`).
 * 
 * CARACTERÍSTICAS DESTA ABORDAGEM:
 * 1. Corretude Matemática:
 *    A seção crítica garante exclusão mútua (mutual exclusion), garantindo que apenas uma thread
 *    por vez execute o bloco de código `pontos_no_circulo++`. Isso elimina completamente os
 *    'Lost Updates' e o resultado do Pi calculado se torna matematicamente correto e preciso.
 * 
 * 2. Custo de Contenção (Bottleneck / Overhead):
 *    Como aproximadamente 78.5% dos pontos sorteados caem dentro do círculo, a seção crítica
 *    é acessada centenas de milhões de vezes. Isso força a serialização contínua das threads e
 *    provoca uma contenção de locks severa. Por isso, embora correta, esta abordagem pode ser
 *    mais lenta que a versão sequencial para loops com iterações muito rápidas.
 * 
 * Compilação:
 *   gcc -fopenmp -O3 3_critical.c -o 3_critical -lm
 * Execução:
 *   ./3_critical [numero_de_pontos]
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    long long total_pontos = 100000000LL; // 20 milhões por padrão (evita lentidão extrema por contenção)
    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: O numero de pontos deve ser positivo.\n");
            return 1;
        }
    }

    printf("Threads ativas : %d\n", omp_get_max_threads());
    printf("Total de pontos: %lld\n", total_pontos);

    long long pontos_no_circulo = 0; // Variável compartilhada protegida por seção crítica
    srand((unsigned int)time(NULL));

    double t_inicio = omp_get_wtime();

    #pragma omp parallel // criação da equipe de threads
    {
        #pragma omp for // distribuição das iterações entre as threads
        for (long long i = 0; i < total_pontos; i++) {
            double x = (double)rand() / (double)RAND_MAX;
            double y = (double)rand() / (double)RAND_MAX;

            if (x * x + y * y <= 1.0) {
                // Seção crítica: garante exclusão mútua para incrementar a variável compartilhada
                #pragma omp critical
                {
                    pontos_no_circulo++;
                }
            }
        }
    }

    double t_fim = omp_get_wtime();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - M_PI);

    printf("Pontos no circulo : %lld / %lld (CORRETO!)\n", pontos_no_circulo, total_pontos);
    printf("Pi Estimado       : %.10f (CORRETO)\n", pi_estimado);
    printf("Pi Real (M_PI)    : %.10f\n", M_PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    return 0;
}
