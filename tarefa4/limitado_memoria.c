/**
 * @file limitado_memoria.c
 * @brief Programa paralelo em C com OpenMP limitado por largura de banda de memória (Memory-Bound).
 * 
 * ======================================================================================
 * IMPORTÂNCIA DAS CARACTERÍSTICAS DA CPU (AMD Ryzen 7 5700X):
 * 
 * 1. Arquitetura de Cores e Threads (8 Cores / 16 Threads):
 *    - Este processador possui 8 núcleos físicos e 16 threads lógicas graças ao SMT
 *      (Simultaneous Multithreading).
 *    - Em programas limitados por memória (memory-bound), o gargalo é a velocidade de
 *      transferência de dados entre a memória RAM física (DDR4) e os registradores da CPU.
 *    - Quando aumentamos o número de threads além de um certo ponto (geralmente de 4 a 8),
 *      o canal de comunicação com a memória (bus/controlador de memória) fica completamente
 *      saturado. A partir desse ponto, adicionar mais threads não trará ganho de desempenho.
 * 
 * 2. Caches do Processador:
 *    - Cache L1d: 32 KiB por núcleo (total de 256 KiB)
 *    - Cache L2: 512 KiB por núcleo (total de 4 MiB)
 *    - Cache L3: 32 MiB unificado (1 instância compartilhada por todos os 8 núcleos)
 *    - IMPORTANTE: Para garantir que o programa seja de fato limitado por memória (DRAM),
 *      alocamos vetores cujo tamanho total supera amplamente o tamanho do Cache L3.
 *      Usamos N = 50.000.000 elementos do tipo double (8 bytes):
 *      3 vetores * 50.000.000 * 8 bytes ≈ 1.2 GB de dados.
 *      Como 1.2 GB >> 32 MiB (L3 Cache), os dados não cabem no cache e precisam ser
 *      constantemente trazidos da memória RAM principal, gerando cache misses intencionais.
 * 
 * 3. Papel do Multithreading de Hardware (SMT):
 *      O SMT (2 threads por núcleo) foi projetado para ocultar a latência de acesso à RAM.
 *      Se a Thread 0 de um núcleo físico está travada esperando dados da RAM, o núcleo pode
 *      alternar para a Thread 1 para realizar processamento. Contudo, em operações de vetores
 *      onde não há quase nenhum processamento (apenas somas simples), as threads adicionais
 *      do SMT não ajudam muito a melhorar o desempenho, pois a banda total de memória já está
 *      totalmente saturada. Pelo contrário, as 16 threads lógicas competindo pela mesma
 *      interface de memória física podem causar disputas de cache adicionais e custos de
 *      sincronização, fazendo com que o desempenho se estabilize ou até piore com 16 threads
 *      em comparação a 8.
 * ======================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// 50 milhões de elementos por vetor (aproximadamente 400 MB por vetor)
#define N 50000000
#define REPETICOES 10

int main() {
    // Alocação dinâmica para evitar estouro de pilha (stack overflow)
    double *A = (double *)malloc(N * sizeof(double));
    double *B = (double *)malloc(N * sizeof(double));
    double *C = (double *)malloc(N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Erro: Falha na alocação de memória.\n");
        return 1;
    }

    // Inicialização simples dos vetores (sem medir tempo)
    // Usamos um loop paralelo para acelerar e garantir a política de "first-touch"
    #pragma omp parallel for
    for (size_t i = 0; i < N; i++) {
        A[i] = (double)i * 0.1;
        B[i] = (double)i * 0.2;
        C[i] = 0.0;
    }

    int total_threads = 0;
    double tempo_inicio = omp_get_wtime();

    // Região paralela principal
    #pragma omp parallel
    {
        // Pega a quantidade de threads configurada apenas uma vez
        #pragma omp single
        {
            total_threads = omp_get_num_threads();
        }

        // Repetimos o cálculo para obter uma medição de tempo mais estável e robusta
        for (int r = 0; r < REPETICOES; r++) {
            #pragma omp for
            for (size_t i = 0; i < N; i++) {
                C[i] = A[i] + B[i];
            }
        }
    }

    double tempo_fim = omp_get_wtime();
    double tempo_total = tempo_fim - tempo_inicio;

    // Impede o compilador de otimizar e remover o loop de soma
    // (Consome uma parte de C para garantir que o cálculo foi executado)
    printf("--- Execucao Memory-Bound ---\n");
    printf("Threads ativas:        %d\n", total_threads);
    printf("Tempo total (%d rep):   %f segundos\n", REPETICOES, tempo_total);
    printf("Tempo por iteracao:    %f segundos\n", tempo_total / REPETICOES);
    printf("Checksum de validacao: C[%d] = %.2f\n", N - 1, C[N - 1]);
    printf("\n");

    // Liberação da memória
    free(A);
    free(B);
    free(C);

    return 0;
}
