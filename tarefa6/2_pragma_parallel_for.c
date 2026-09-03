/**
 * @file 2_pragma_parallel_for.c
 * @brief Estimativa estocástica de Pi - Paralelização com Condição de Corrida (Data Race).
 * 
 * Este programa demonstra o erro clássico de paralelização ingênua com `#pragma omp parallel for`.
 * 
 * POR QUE O RESULTADO É INCORRETO?
 * 1. Condição de Corrida (Race Condition / Data Race):
 *    A operação `pontos_no_circulo++` NÃO É ATÔMICA. No nível de instruções de máquina (Assembly),
 *    ela envolve 3 passos distintos:
 *      (a) READ: Ler o valor de 'pontos_no_circulo' da memória compartilhada para um registrador.
 *      (b) MODIFY: Incrementar o valor no registrador (reg = reg + 1).
 *      (c) WRITE: Gravar o valor atualizado de volta na memória compartilhada.
 *    Quando múltiplas threads executam esse trecho em paralelo sem sincronização, ocorre sobreposição
 *    (interleaving): duas ou mais threads leem o mesmo valor antes que qualquer uma consiga gravar
 *    a atualização. Como resultado, apenas uma das gravações persiste e as demais são SOBRESCRITAS
 *    (Lost Updates). Milhões de contagens são perdidas, fazendo com que o Pi estimado fique
 *    drasticamente abaixo do valor real (ex: ~0.8 a ~1.8 em vez de ~3.14).
 * 
 * Compilação:
 *   gcc -fopenmp -O3 2_pragma_parallel_for.c -o 2_pragma_parallel_for -lm
 * Execução:
 *   ./2_pragma_parallel_for [numero_de_pontos]
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    long long total_pontos = 100000000LL; // Padrão: 100 milhões de pontos
    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: O numero de pontos deve ser positivo.\n");
            return 1;
        }
    }

    printf("Threads ativas : %d\n", omp_get_max_threads());
    printf("Total de pontos: %lld\n", total_pontos);

    long long pontos_no_circulo = 0; // Variável compartilhada não protegida!
    // Adicionando semente para a geração de números aleatórios
    srand((unsigned int)time(NULL)); // (sem semente os números aleatórios serão os mesmos em todas as execuções)

    double t_inicio = omp_get_wtime();

    #pragma omp parallel for
    for (long long i = 0; i < total_pontos; i++) {
        double x = (double)rand() / (double)RAND_MAX;
        double y = (double)rand() / (double)RAND_MAX;

        if (x * x + y * y <= 1.0) {
            // Múltiplas threads leem, incrementam e escrevem em 'pontos_no_circulo' ao mesmo tempo.
            pontos_no_circulo++;
        }
    }

    double t_fim = omp_get_wtime();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - M_PI);

    printf("Pontos no circulo : %lld / %lld (Contagens perdidas!)\n", pontos_no_circulo, total_pontos);
    printf("Pi Estimado       : %.10f (Incorreto!)\n", pi_estimado);
    printf("Pi Real (M_PI)    : %.10f\n", M_PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    return 0;
}
