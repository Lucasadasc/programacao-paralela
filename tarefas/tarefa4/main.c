/**
 * @file main.c
 * @brief Ponto de entrada explicativo para a Tarefa 4.
 * 
 * Nesta tarefa, a implementação foi dividida em dois programas separados para melhor
 * isolamento dos testes de desempenho (Memory-Bound vs Compute-Bound):
 * 
 * 1. limitado_memoria.c - Programa que satura a largura de banda de memória (DRAM).
 * 2. limitado_cpu.c     - Programa com cálculos matemáticos intensivos (FPUs).
 * 
 * Para automatizar os benchmarks variando o número de threads (1, 2, 4, 8, 12, 16) e obter
 * a tabela comparativa de desempenho, você pode executar o script:
 *   $ ./rodar_testes.sh
 * 
 * Ou compilar e rodar manualmente:
 *   $ gcc -fopenmp -O2 limitado_memoria.c -o limitado_memoria
 *   $ export OMP_NUM_THREADS=4; ./limitado_memoria
 * 
 *   $ gcc -fopenmp -O2 limitado_cpu.c -o limitado_cpu -lm
 *   $ export OMP_NUM_THREADS=4; ./limitado_cpu
 */

#include <stdio.h>

int main() {
    printf("Tarefa 4: Programas paralelos em C com OpenMP.\n");
    printf("Por favor, execute './rodar_testes.sh' para rodar os benchmarks ou consulte:\n");
    printf("- limitado_memoria.c\n");
    printf("- limitado_cpu.c\n");
    return 0;
}
