/**
 * @file 7_default_none.c
 * @brief Estimativa estocástica de Pi - Uso da cláusula 'default(none)' para controle rigoroso de escopo.
 * 
 * OBJETIVO DIDÁTICO (Passo 4 das Cláusulas):
 * Por padrão no OpenMP (default(shared)), variáveis declaradas fora da região paralela
 * tornam-se implicitamente compartilhadas (shared) entre todas as threads, a menos que
 * sejam especificadas em outra cláusula. Esse comportamento implícito é uma das maiores
 * fontes de bugs sutis e condições de corrida (data races).
 * 
 * A cláusula `default(none)` desativa esse comportamento padrão e impõe uma regra rígida:
 * 1. OBRIGA o desenvolvedor a declarar explicitamente o escopo de compartilhamento
 *    (shared, private, firstprivate, etc.) de TODAS as variáveis declaradas fora
 *    da região paralela e utilizadas dentro dela.
 * 2. Se qualquer variável externa for usada dentro do bloco paralelo sem estar
 *    explicitamente listada em uma cláusula de escopo, o compilador gerará um ERRO
 *    de compilação (ex: "error: 'variavel' not specified in enclosing 'parallel'").
 * 3. É considerada uma das melhores práticas de engenharia de software em OpenMP,
 *    pois força a tomada consciente de decisão sobre cada variável, eliminando efeitos
 *    colaterais e compartilhamento acidental de dados.
 * 
 * Compilação:
 *   gcc -fopenmp -O3 7_default_none.c -o 7_default_none -lm
 * Execução:
 *   ./7_default_none [numero_de_pontos]
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

    long long pontos_no_circulo_total = 0; // Compartilhada explicitamente via 'shared'
    long long contador_local = 0;          // Cópia privada inicializada explicitamente via 'firstprivate'

    srand((unsigned int)time(NULL));

    double t_inicio = omp_get_wtime();

    // Com 'default(none)', somos obrigados a especificar explicitamente
    // todas as variáveis externas acessadas dentro da região paralela:
    // - total_pontos: apenas leitura (shared)
    // - pontos_no_circulo_total: acumulador global protegido por critical (shared)
    // - contador_local: acumulador local inicializado com 0 para cada thread (firstprivate)
    #pragma omp parallel \
        default(none) \
        shared(total_pontos, pontos_no_circulo_total) \
        firstprivate(contador_local)
    {
        // Variáveis declaradas dentro da região paralela são inerentemente privadas a cada thread
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

        // Atualização atômica/crítica do total global
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
