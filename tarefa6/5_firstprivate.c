/**
 * @file 5_firstprivate.c
 * @brief Estimativa estocástica de Pi - Evolução com a cláusula 'firstprivate'.
 * 
 * OBJETIVO DIDÁTICO (Passo 2 das Cláusulas):
 * No programa anterior (4_shared_private.c), vimos que variáveis declaradas como
 * `private` iniciam com valor indefinido (lixo de memória), forçando uma inicialização
 * manual (`contador_local = 0;`) dentro da região paralela.
 * 
 * A cláusula `firstprivate(contador_local)` resolve isso:
 * 1. Cria uma cópia privada de `contador_local` para cada thread.
 * 2. INICIALIZA automaticamente a cópia de cada thread com o valor que a variável
 *    possuía na thread mestre imediatamente antes de entrar no bloco paralelo (valor 0).
 * 
 * Compilação:
 *   gcc -fopenmp -O3 5_firstprivate.c -o 5_firstprivate -lm
 * Execução:
 *   ./5_firstprivate [numero_de_pontos]
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define PI 3.14159265358979323846

int main(int argc, char *argv[]) {
    long long total_pontos = 100000000LL; // 100 milhões de pontos
    if (argc > 1) {
        total_pontos = atoll(argv[1]);
        if (total_pontos <= 0) {
            fprintf(stderr, "Erro: O numero de pontos deve ser positivo.\n");
            return 1;
        }
    }

    printf("Threads ativas : %d\n", omp_get_max_threads());
    printf("Total de pontos: %lld\n", total_pontos);

    long long pontos_no_circulo_total = 0; // Compartilhada
    long long contador_local = 0;          // Inicializada aqui e herdada via 'firstprivate'

    srand((unsigned int)time(NULL));

    double t_inicio = omp_get_wtime();

    // firstprivate: cada thread recebe sua própria cópia já inicializada com 0
    #pragma omp parallel \
        shared(total_pontos, pontos_no_circulo_total) \
        firstprivate(contador_local)
    {
        // 'contador_local' já inicia valendo 0 em todas as threads (sem precisar de inicialização manual)
        double x, y, dist_sq;

        // Distribuição das iterações do loop
        #pragma omp for private(x, y, dist_sq)
        for (long long i = 0; i < total_pontos; i++) {
            x = (double)rand() / (double)RAND_MAX;
            y = (double)rand() / (double)RAND_MAX;
            dist_sq = x * x + y * y;

            if (dist_sq <= 1.0) {
                contador_local++;
            }
        }

        // 3 Atualizando o total global apenas uma vez por thread
        #pragma omp critical
        {
            pontos_no_circulo_total += contador_local;
        }
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

    return 0;
}
