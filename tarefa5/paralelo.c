#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// Verifica se um número n é primo
int eh_primo(int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    
    for (int i = 3; i < n; i += 1) {
        if (n % i == 0) return 0;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    int a = 1000000; // Valor máximo padrão
    if (argc > 1) {
        a = atoi(argv[1]);
        if (a < 2) {
            fprintf(stderr, "Erro: O valor de 'a' deve ser no mínimo 2.\n");
            return 1;
        }
    }

    printf("Calculando quantidade de primos entre 2 e %d...\n", a);

    int count = 0;
    double inicio = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 2; i <= a; i++) {
        if (eh_primo(i)) {
            count++;
        }
    }

    double fim = omp_get_wtime();
    double tempo_total = fim - inicio;

    printf("Resultado: %d primos encontrados.\n", count);
    printf("Tempo de execucao: %f segundos\n", tempo_total);

    return 0;
}

// gcc -fopenmp paralelo.c -o paralelo 