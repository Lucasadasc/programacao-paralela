#define _POSIX_C_SOURCE 199309L // Necessário para clock_gettime() no Linux
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Tamanho do vetor: 5 milhões de elementos double (aprox. 40 MB)
#define N 5000000
    
// Recuperar tempo de relógio transcorrido (em segundos) conforme orientado na sala de aula
double recuperar_tempo_instantaneo() {
    struct timespec ts; // Estrutura para armazenar o tempo
    clock_gettime(CLOCK_MONOTONIC, &ts); // Obtém o tempo de relógio monotônico (não retrocede)
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Converte para segundos
}

int main() {
    // Alocação dinâmica do vetor para evitar estouro de pilha (stack overflow)
    double *vetor = (double *)malloc(N * sizeof(double));
    if (vetor == NULL) {
        fprintf(stderr, "Erro: Falha na alocação de memória.\n");
        return 1;
    }

    double inicio, fim;
    double tempo_inicializacao, tempo_soma_dependente, tempo_soma_independente;

    // --- Laço 1: Inicialização do vetor com um cálculo simples ---
    inicio = recuperar_tempo_instantaneo();
    for (size_t i = 0; i < N; i++) {
        vetor[i] = (double)i * 0.5;
    }
    fim = recuperar_tempo_instantaneo();
    tempo_inicializacao = fim - inicio;

    // --- Laço 2: Soma acumulativa com dependência forte entre iterações ---
    double soma_dependente = 0.0;
    inicio = recuperar_tempo_instantaneo();
    for (size_t i = 0; i < N; i++) {
        soma_dependente += vetor[i];
    }
    fim = recuperar_tempo_instantaneo();
    tempo_soma_dependente = fim - inicio;

    // --- Laço 3: Soma acumulativa quebrando a dependência com múltiplas variáveis ---
    // Desenrolado por um fator de 4 utilizando 4 acumuladores independentes (ILP)
    double soma_independente1 = 0.0;
    double soma_independente2 = 0.0;
    double soma_independente3 = 0.0;
    double soma_independente4 = 0.0;

    inicio = recuperar_tempo_instantaneo();
    for (size_t i = 0; i < N; i += 4) {
        soma_independente1 += vetor[i];
        if (i + 1 < N) soma_independente2 += vetor[i + 1];
        if (i + 2 < N) soma_independente3 += vetor[i + 2];
        if (i + 3 < N) soma_independente4 += vetor[i + 3];
    }
    fim = recuperar_tempo_instantaneo();
    tempo_soma_independente = fim - inicio;
    
    double soma_independente = soma_independente1 + soma_independente2 + soma_independente3 + soma_independente4;

    // Exibição dos tempos e dos resultados (evita que o compilador otimize/remova os laços)
    printf("Tamanho do vetor: %d elementos\n", N);
    printf("1) Tempo de Inicializacao:                %f segundos\n", tempo_inicializacao);
    printf("2) Tempo da Soma Acumulativa (dependente): %f segundos (Soma: %.2f)\n", tempo_soma_dependente, soma_dependente);
    printf("3) Tempo da Soma Multi-var (independente): %f segundos (Soma: %.2f)\n", tempo_soma_independente, soma_independente);

    // Libera a memória alocada
    free(vetor);
    return 0;
}
