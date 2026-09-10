/**
 * @file formato_4.c
 * @brief Estimativa estocástica de Pi - Vetor compartilhado, acumulação serial e 'rand_r'.
 * 
 * OBJETIVO DIDÁTICO (Formato 4):
 * 1. Baseado no Formato 2 (vetor compartilhado 'acertos[tid]' com incremento direto).
 * 
 * 2. Observação do Falso Compartilhamento (False Sharing):
 *    - Cada acerto é contabilizado diretamente com `acertos[tid]++` no laço paralelo.
 *    - Como elementos vizinhos do vetor residem na mesma linha de cache de 64 bytes,
 *      esta versão reproduz intencionalmente o falso compartilhamento clássico
 *      (semelhante ao código sem 'padding' da apostila).
 * 
 * 3. Substituição de rand() por rand_r():
 *    - Permite isolar o efeito da coerência de cache/falso compartilhamento,
 *      uma vez que o gerador rand_r() não utiliza travas globais.
 * 
 * Compilação:
 *   gcc -fopenmp -O3 formato_4.c -o formato_4 -lm
 * Execução:
 *   ./formato_4 [numero_de_pontos]
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define PI 3.14159265358979323846

int main(int argc, char *argv[]) {
    long long total_pontos = 10000000LL; // 10 milhões de pontos
    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: O numero de pontos deve ser positivo.\n");
            return 1;
        }
    }

    int max_threads = omp_get_max_threads();
    printf("Threads ativas : %d\n", max_threads);
    printf("Total de pontos: %lld\n", total_pontos);

    // Vetor compartilhado onde cada thread armazenará seus acertos
    long long *acertos = (long long *)calloc(max_threads, sizeof(long long));
    if (acertos == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para o vetor de acertos.\n");
        return 1;
    }

    long long pontos_no_circulo_total = 0;

    double t_inicio = omp_get_wtime();

    // Cada thread escreve diretamente na sua posição do vetor compartilhado dentro do loop
    #pragma omp parallel shared(total_pontos, acertos)
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)time(NULL) + tid;

        double x, y, dist_sq; // Variáveis locais à thread (privadas)

        // Distribuindo as iterações do loop entre as threads
        #pragma omp for private(x, y, dist_sq)
        for (long long i = 0; i < total_pontos; i++) {
            x = (double)rand_r(&seed) / (double)RAND_MAX;
            y = (double)rand_r(&seed) / (double)RAND_MAX;
            dist_sq = x * x + y * y;

            if (dist_sq <= 1.0) {
                acertos[tid]++; // Incremento direto no vetor compartilhado (provoca falso compartilhamento)
            }
        }
    }

    // Acumulação em um laço serial após a região paralela
    for (int t = 0; t < max_threads; t++) {
        pontos_no_circulo_total += acertos[t];
    }

    double t_fim = omp_get_wtime();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo_total / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - PI);

    printf("Pontos no circulo : %lld / %lld (Correto!)\n", pontos_no_circulo_total, total_pontos);
    printf("Pi Estimado       : %.10f (Correto!)\n", pi_estimado);
    printf("Pi Real (PI)      : %.10f\n", PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    free(acertos);

    return 0;
}
