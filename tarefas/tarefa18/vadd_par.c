#include <stdio.h>
#include <omp.h>
#define tamanhoVetor 100000000
#define tolerancia 0.0000001f

int main()
{

    float vetorA[tamanhoVetor], vetorB[tamanhoVetor];
    float vetorSoma[tamanhoVetor], vetorEsperado[tamanhoVetor];
    int quantidadeErros = 0;

    double tempoInicializacao, tempoCalculo, tempoTeste;
    tempoInicializacao = -omp_get_wtime();

   // Inicializa os vetores de entrada, soma e resultado esperado.
   #pragma omp parallel for
   for (int indice = 0; indice < tamanhoVetor; indice++){
      vetorA[indice] = (float)indice;
      vetorB[indice] = 2.0f * (float)indice;
      vetorSoma[indice] = 0.0f;
      vetorEsperado[indice] = indice + 2 * indice;
   }

   tempoInicializacao += omp_get_wtime();
   tempoCalculo = -omp_get_wtime();
   
   // Soma os vetores de entrada e armazena o resultado.
   #pragma omp parallel for
   for (int indice = 0; indice < tamanhoVetor; indice++){
      vetorSoma[indice] = vetorA[indice] + vetorB[indice];
   }

   tempoCalculo += omp_get_wtime();
   tempoTeste = -omp_get_wtime();

   // testando se o resultado da soma está correto
   #pragma omp parallel for reduction(+:quantidadeErros)
   for (int indice = 0; indice < tamanhoVetor; indice++) {
      float diferenca = vetorSoma[indice] - vetorEsperado[indice];
      diferenca = diferenca * diferenca;
      if (diferenca > tolerancia) quantidadeErros++;
   }

   tempoTeste += omp_get_wtime();
   
   printf("Vetores somados com %d erros\n", quantidadeErros);
   printf("Tempo de inicialização: %.3fs\n", tempoInicializacao);
   printf("Tempo de computação: %.3fs\n", tempoCalculo);
   printf("Tempo de teste: %.3fs\n", tempoTeste);
   printf("Tempo total: %.3fs\n", tempoInicializacao + tempoCalculo + tempoTeste);
   return 0;
}
