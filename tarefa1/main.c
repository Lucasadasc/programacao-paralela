#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <quantidade_iteracoes>\n", argv[0]);
        return 1;
    }

    // o strtoull() converte uma string para um unsigned long long  
    const unsigned long long quantidade_iteracoes = strtoull(argv[1], NULL, 10); 
    const double valor_calculado = calcular_valor_pi(quantidade_iteracoes);
    const double erro_absoluto = fabs(valor_calculado - PI);

    printf("Iteracoes: %llu\n", quantidade_iteracoes);
    printf("- Valor calculado: %.15f\n", valor_calculado);
    printf("- Erro absoluto: %.15f\n", erro_absoluto);

    return 0;
}

// gcc -O2 -Wall -Wextra -std=c11 main.c -lm -o main && ./main
// time ./main