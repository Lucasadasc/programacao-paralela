#define _POSIX_C_SOURCE 199309L

#include <math.h>
#include <stdio.h>
#include <time.h>


// pi/4 = 1 - 1/3 + 1/5 - 1/7 + 1/9 - 1/11 + ...
static double calcular_valor_pi(unsigned long long iteracoes) {
    double soma = 0.0;
    
    for (unsigned long long i = 0; i < iteracoes; ++i) {
        double termo = 1.0 / (2.0 * i + 1.0);
        if (i % 2 == 0) {
            soma += termo;
        } else {
            soma -= termo;
        }
    }
    
    return 4.0 * soma;
}

int main(void) {
    const unsigned long long iteracoes[] = {100, 1000, 10000, 100000, 1000000};
    const int qtd_testes = 5;
    const double pi_real = 3.14159265358979323846;

    printf("Aproximacao de pi usando a serie de Leibniz\n");
    printf("------------------------------------------------\n");
    printf("iteracoes | aproximacao | erro absoluto | tempo (ms)\n");
    printf("------------------------------------------------\n");

    for (int i = 0; i < qtd_testes; ++i) {
        struct timespec inicio, fim;
        clock_gettime(CLOCK_MONOTONIC, &inicio);

        double aproximacao = calcular_valor_pi(iteracoes[i]);

        clock_gettime(CLOCK_MONOTONIC, &fim);

        double tempo_ms = (fim.tv_sec - inicio.tv_sec) * 1000.0 +
                          (fim.tv_nsec - inicio.tv_nsec) / 1000000.0;
        double erro = fabs(aproximacao - pi_real);

        printf("%10llu | %11.10f | %12.10f | %12.6f\n",
               iteracoes[i], aproximacao, erro, tempo_ms);
    }

    printf("------------------------------------------------\n");
    printf("Analise: quanto mais iteracoes, mais perto da aproximacao de pi\n");
    printf("O erro tende a diminuir, mas o tempo de execucao cresce.\n");
    printf("Isso mostra o trade-off entre acuracia e custo computacional.\n");

    return 0;
}
