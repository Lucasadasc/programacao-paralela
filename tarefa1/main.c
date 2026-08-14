#include <math.h>
#include <stdio.h>
#include <time.h>

#define PI 3.14159265358979323846


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

    const unsigned long long quantidade_iteracoes[] = {100, 1000, 10000, 100000, 1000000, 10000000};
    const int quantidade_testes = 6;
    const double valor_real_pi = PI;

    for (int i = 0; i < quantidade_testes; ++i) {
        const double valor_calculado = calcular_valor_pi(quantidade_iteracoes[i]);

        const double erro_absoluto = fabs(valor_calculado - valor_real_pi);

        printf("\nIteracoes: %lld\n", quantidade_iteracoes[i]);
        printf("- Erro absoluto: %.15f\n", erro_absoluto);
        printf("- Valor calculado: %.15f\n", valor_calculado);
    }

    return 0;
}

// gcc -O2 -Wall -Wextra -std=c11 main.c -lm -o main && ./main
// time ./main