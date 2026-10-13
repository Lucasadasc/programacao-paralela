
/*
** PROGRAM: heat equation solve
**
** PURPOSE: This program will explore use of an explicit
**          finite difference method to solve the heat
**          equation under a method of manufactured solution (MMS)
**          scheme. The solution has been set to be a simple 
**          function based on exponentials and trig functions.
**
**          A finite difference scheme is used on a 1000x1000 cube.
**          A total of 0.5 units of time are simulated.
**
**          The MMS solution has been adapted from
**          G.W. Recktenwald (2011). Finite difference approximations
**          to the Heat Equation. Portland State University.
**
**
** USAGE:   Run with two arguments:
**          First is the number of cells.
**          Second is the number of timesteps.
**
**          For example, with 100x100 cells and 10 steps:
**
**          ./heat 100 10
**
**
** HISTORY: Written by Tom Deakin, Oct 2018
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <omp.h>

#include "heatCommon.h"

#define PI acos(-1.0)
#define LINE "--------------------\n"

// Configura o problema, executa os passos e verifica a solução na CPU.
int main(int argc, char *argv[]) {

  // Tempo total: inclui configuração, inicialização, solve e norma do erro.
  // omp_get_wtime retorna o tempo de parede em segundos.
  double start = omp_get_wtime();

  // Valores padrão: n células internas em cada direção e nsteps passos.
  int n = 1000;

  // O intervalo físico total permanece 0,5 ao variar o número de passos.
  int nsteps = 10;

  // Lê os mesmos parâmetros usados pelo script: lado da malha e passos.
  if (argc != 1 && argc != 3) {
    fprintf(stderr, "Uso: %s [n passos]\n", argv[0]);
    return EXIT_FAILURE;
  }
  if (argc == 3) {
    char *fim_n, *fim_passos;
    long valor_n = strtol(argv[1], &fim_n, 10);
    long valor_passos = strtol(argv[2], &fim_passos, 10);
    if (*fim_n || *fim_passos || valor_n < 1 || valor_n > 4096 ||
        valor_passos < 1 || valor_passos > 1000000) {
      fprintf(stderr, "Informe 1 <= n <= 4096 e 1 <= passos <= 1000000.\n");
      return EXIT_FAILURE;
    }
    n = (int)valor_n;
    nsteps = (int)valor_passos;
  }

  //
  // Parâmetros físicos e discretização.
  //
  double alpha = 0.1;          // Coeficiente de difusão térmica.
  double length = 1000.0;      // Lado do domínio quadrado.
  double dx = length / (n+1);  // Espaçamento entre pontos internos e bordas.
  double dt = 0.5 / nsteps;    // Passo temporal: tempo físico total de 0,5.


  // r é o coeficiente adimensional da discretização explícita.
  // Para este stencil 2D com espaçamentos iguais, a estabilidade exige r <= 1/4.
  double r = alpha * dt / (dx * dx);

  // Registra os parâmetros utilizados para permitir interpretar os resultados.
  printf("\n");
  printf(" MMS heat equation\n\n");
  printf(LINE);
  printf("Problem input\n\n");
  printf(" Grid size: %d x %d\n", n, n);
  printf(" Cell width: %E\n", dx);
  printf(" Grid length: %lf x %lf\n", length, length);
  printf("\n");
  printf(" Alpha: %E\n", alpha);
  printf("\n");
  printf(" Steps: %d\n", nsteps);
  printf(" Total time: %E\n", dt*(double)nsteps);
  printf(" Time step: %E\n", dt);
  printf(LINE);

  // Verifica o limite do esquema explícito bidimensional.
  printf("Stability\n\n");
  printf(" r value: %lf\n", r);
  if (r > 0.25)
    printf(" Warning: unstable\n");
  printf(LINE);


  // Aloca os dois campos na memória da CPU. A posição (i,j) fica em i+j*n.
  // u é a entrada do passo; u_tmp recebe a saída, sem sobrescrever a entrada.
  const size_t cells = (size_t)n * n;
  double *u     = malloc(sizeof(double)*cells);
  double *u_tmp = malloc(sizeof(double)*cells);
  if (!u || !u_tmp) {
    fprintf(stderr, "Falha ao alocar os campos.\n");
    free(u);
    free(u_tmp);
    return EXIT_FAILURE;
  }

  // Inicializa o campo senoidal e zera o segundo array na CPU.
  initial_value(n, dx, length, u);
  zero(n, u_tmp);

  //
  // Evolução temporal pelo esquema explícito.
  //

  // Mede somente o laço temporal: inclui cópias, kernels, sincronização
  // e custos do runtime, inclusive os da primeira chamada ao dispositivo.
  double tic = omp_get_wtime();
  evolve(n, nsteps, alpha, dx, dt, &u, &u_tmp);
  // A verificação numérica abaixo fica fora do tempo de solve.
  double toc = omp_get_wtime();

  //
  // Compara o campo final com a solução analítica da MMS.
  //
  double norm = l2norm(n, u, nsteps, dt, alpha, dx, length);

  // Encerra a medição total após a verificação da solução.
  double stop = omp_get_wtime();

  // Imprime o erro e os dois tempos, extraídos pelo script do experimento.
  printf("Results\n\n");
  printf("Error (L2norm): %E\n", norm);
  printf("Solve time (s): %lf\n", toc-tic);
  printf("Total time (s): %lf\n", stop-start);
  printf(LINE);

  // Libera ambas as alocações, independentemente das trocas de ponteiros.
  free(u);
  free(u_tmp);

}


// Condição inicial senoidal da solução manufaturada (MMS).
// Os pontos internos começam em dx; as bordas externas têm temperatura zero.
void initial_value(const int n, const double dx, const double length, double * restrict u) {

  double y = dx;
  for (int j = 0; j < n; ++j) {
    double x = dx; // Coordenada física da primeira célula interna da linha.
    for (int i = 0; i < n; ++i) {
      u[i+j*n] = sin(PI * x / length) * sin(PI * y / length);
      x += dx;
    }
    y += dx; // Avança para a próxima linha da malha.
  }

}


// Preenche o array com zeros na CPU antes do laço temporal.
void zero(const int n, double * restrict u) {

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      u[i+j*n] = 0.0;
    }
  }

}


// Solução analítica: o perfil senoidal decai exponencialmente com o tempo.
// Serve como referência para verificar o campo calculado numericamente.
double solution(const double t, const double x, const double y, const double alpha, const double length) {

  return exp(-2.0*alpha*PI*PI*t/(length*length)) * sin(PI*x/length) * sin(PI*y/length);

}


// Calcula na CPU a norma euclidiana do erro: sqrt(soma((u - solução)^2)).
// Não normaliza pelo número de células nem pondera pela área de cada célula.
double l2norm(const int n, const double * restrict u, const int nsteps, const double dt, const double alpha, const double dx, const double length) {

  // Instante físico final, igual a 0,5 para os parâmetros deste programa.
  double time = dt * (double)nsteps;

  // Acumula os quadrados das diferenças em todas as células internas.
  double l2norm = 0.0;

  // Percorre o campo e avalia a referência nas mesmas coordenadas físicas.
  double y = dx;
  for (int j = 0; j < n; ++j) {
    double x = dx;
    for (int i = 0; i < n; ++i) {
      double answer = solution(time, x, y, alpha, length);
      l2norm += (u[i+j*n] - answer) * (u[i+j*n] - answer);

      x += dx;
    }
    y += dx;
  }

  return sqrt(l2norm);

}
