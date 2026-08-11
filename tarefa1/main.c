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
    // const long long quantidade_iteracoes = 100000000;

    // const double valor_calculado = calcular_valor_pi(quantidade_iteracoes);

    // printf("Valor calculado de pi com %lld iteracoes: %.15f\n", quantidade_iteracoes, valor_calculado);

    const unsigned long long iteracoes[] = {100, 1000, 10000, 100000, 1000000, 10000000};
    const int quantidade_testes = 6;

    for (int i = 0; i < quantidade_testes; ++i) {
        const double valor_calculado = calcular_valor_pi(iteracoes[i]);

        printf("Valor calculado de pi com %lld iteracoes: %.15f\n", iteracoes[i], valor_calculado);
    }

    return 0;
}

// gcc -O2 -Wall -Wextra -std=c11 main.c -lm -o main && ./main
// time ./main