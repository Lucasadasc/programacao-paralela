/**
 * @file limitado_cpu.c
 * @brief Programa paralelo em C com OpenMP limitado por capacidade de processamento (Compute-Bound).
 * 
 * ======================================================================================
 * IMPORTÂNCIA DAS CARACTERÍSTICAS DA CPU (AMD Ryzen 7 5700X):
 * 
 * 1. Arquitetura de Cores e Threads (8 Cores / 16 Threads):
 *    - O processador possui 8 núcleos físicos independentes, cada um contendo suas próprias
 *      unidades de execução de ponto flutuante (FPUs) e lógica aritmética (ALUs).
 *    - Quando executamos o programa com até 8 threads, cada thread é mapeada para um núcleo
 *      físico diferente. A aceleração deve ser quase linear (ex: 8 threads rodando ~8x mais
 *      rápido que 1 thread).
 *    - Ao subir de 8 para 16 threads, ativamos o SMT (2 threads lógicas por núcleo físico).
 *      No SMT, as duas threads lógicas em um mesmo núcleo compartilham as mesmas FPUs e ALUs.
 *      Como este programa é puramente limitado por CPU e satura completamente as FPUs com
 *      cálculos de ponto flutuante intensivos (sin, cos, sqrt), colocar uma segunda thread
 *      no mesmo núcleo não aumentará o rendimento dos cálculos matemáticos. O gargalo passa a
 *      ser a capacidade de execução do próprio núcleo físico.
 *    - Com isso, esperamos que o desempenho de 8 para 16 threads estabilize ou até piore
 *      devido aos custos de concorrência e chaveamento de contexto no hardware.
 * 
 * 2. Caches do Processador:
 *    - Cache L1d: 32 KiB por núcleo (total de 256 KiB)
 *    - Cache L2: 512 KiB por núcleo (total de 4 MiB)
 *    - IMPORTANTE: Para que o programa seja estritamente limitado por CPU, a quantidade de
 *      dados acessados deve caber perfeitamente nos caches mais rápidos do processador.
 *      Definimos N = 10.000 elementos do tipo double (8 bytes):
 *      1 vetor * 10.000 * 8 bytes ≈ 80 KiB.
 *      Dividido entre as threads de execução, os dados de trabalho de cada thread cabem
 *      inteiramente em seu cache local (L1d de 32 KiB ou L2 de 512 KiB). Isso elimina qualquer
 *      gargalo ou atraso vindo do barramento com a memória RAM.
 * ======================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// N pequeno para caber nos caches locais (L1/L2)
#define N 10000
// M alto para gerar bilhões de operações aritméticas pesadas
#define M 50000

int main() {
    double *A = (double *)malloc(N * sizeof(double));
    if (A == NULL) {
        fprintf(stderr, "Erro: Falha na alocação de memória.\n");
        return 1;
    }

    // Inicialização do vetor (fora da medição)
    for (size_t i = 0; i < N; i++) {
        A[i] = (double)i * 0.001;
    }

    int total_threads = 0;
    double tempo_inicio = omp_get_wtime();

    // Paralelização com OpenMP
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Usamos o pragma single para evitar que todas as threads façam a mesma chamada
            total_threads = omp_get_num_threads();
        }

        #pragma omp for schedule(static)
        for (size_t i = 0; i < N; i++) {
            double val = A[i];
            // Laço intensivo de processamento matemático (FPUs)
            for (int j = 0; j < M; j++) {
                val = sin(val) * cos(val) + sqrt(val + 1.0) - tan(val * 0.001);
            }
            A[i] = val;
        }
    }

    double tempo_fim = omp_get_wtime();
    double tempo_total = tempo_fim - tempo_inicio;

    // Impede o compilador de otimizar e descartar as computações matemáticas
    printf("--- Execucao Compute-Bound ---\n");
    printf("Threads ativas:        %d\n", total_threads);
    printf("Tempo total de calculo: %f segundos\n", tempo_total);
    printf("Checksum de validacao: A[%d] = %.4f\n", N - 1, A[N - 1]);
    printf("\n");

    free(A);
    return 0;
}
