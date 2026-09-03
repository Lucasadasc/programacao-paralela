/**
 * @file 6_lastprivate.c
 * @brief Estimativa estocástica de Pi - Tentativa ingênua de usar 'lastprivate' como acumulador total.
 * 
 * OBJETIVO DIDÁTICO (O Erro Conceitual do 'lastprivate'):
 * Um erro muito comum ao aprender OpenMP é imaginar que a combinação `firstprivate + lastprivate`
 * funcionaria como uma redução/acumulador automático:
 * 
 * 1. O pensamento ingênuo:
 *    - "O `firstprivate(pontos_no_circulo)` inicializa a contagem de cada thread com 0."
 *    - "Cada thread conta seus pontos em paralelo sem condição de corrida."
 *    - "O `lastprivate(pontos_no_circulo)` pega o valor no fim e entrega para a variável externa!"
 * 
 * 2. Por que isso NÃO funciona (O Erro Lógico):
 *    - O `lastprivate` NÃO SOMA nem combina os resultados das threads.
 *    - Ele simplesmente copia o valor que a variável tinha na ÚLTIMA ITERAÇÃO SEQUENCIAL (i = total_pontos - 1)
 *      de volta para a variável original fora do laço.
 *    - Ou seja, ele retorna APENAS a contagem parcial da thread que executou o último bloco de iterações!
 *    - Os pontos contados por TODAS as outras threads são COMPLETAMENTE DESCARTADOS!
 *    - Resultado: com T threads, a contagem final será de apenas ~1/T do total real,
 *      fazendo o Pi estimado ficar em torno de Pi / T (ex: ~0.20 com 16 threads em vez de ~3.14).
 * 
 * Para combinar resultados entre threads, deve-se usar acumulação manual (`critical`/`atomic`)
 * ou a cláusula projetada para isso: `reduction(+ : variavel)`.
 * 
 * Compilação:
 *   gcc -fopenmp -O3 6_lastprivate.c -o 6_lastprivate -lm
 * Execução:
 *   ./6_lastprivate [numero_de_pontos]
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

    int num_threads = omp_get_max_threads();
    printf("Threads ativas : %d\n", num_threads);
    printf("Total de pontos: %lld\n", total_pontos);

    long long pontos_no_circulo = 0; // Inicializada com 0
    
    // Variáveis para registrar o último ponto sorteado no loop
    double ultimo_x = -1.0;
    double ultimo_y = -1.0;

    srand((unsigned int)time(NULL));

    double t_inicio = omp_get_wtime();

    // TENTATIVA: usar firstprivate para iniciar com 0 e lastprivate para "obter o total" ao final
    #pragma omp parallel for \
        firstprivate(pontos_no_circulo) \
        lastprivate(pontos_no_circulo, ultimo_x, ultimo_y)
    for (long long i = 0; i < total_pontos; i++) {
        double x = (double)rand() / (double)RAND_MAX;
        double y = (double)rand() / (double)RAND_MAX;
        double dist_sq = x * x + y * y;

        if (dist_sq <= 1.0) {
            pontos_no_circulo++;
        }

        ultimo_x = x;
        ultimo_y = y;
    }

    double t_fim = omp_get_wtime();
    double tempo_total = t_fim - t_inicio;

    double pi_estimado = 4.0 * ((double)pontos_no_circulo / (double)total_pontos);
    double erro_absoluto = fabs(pi_estimado - PI);

    printf("\n--- RESULTADO DA TENTATIVA COM LASTPRIVATE ---\n");
    printf("Pontos no circulo : %lld / %lld (Incorreto! Apenas a contagem da ultima thread)\n", 
           pontos_no_circulo, total_pontos);
    printf("Pi Estimado       : %.10f (Incorreto! Aproximadamente Pi / %d threads = %.4f)\n", 
           pi_estimado, num_threads, PI / num_threads);
    printf("Pi Real (PI)      : %.10f\n", PI);
    printf("Erro Absoluto     : %.10e\n", erro_absoluto);
    printf("Tempo de Execucao : %.6f segundos\n", tempo_total);

    return 0;
}
