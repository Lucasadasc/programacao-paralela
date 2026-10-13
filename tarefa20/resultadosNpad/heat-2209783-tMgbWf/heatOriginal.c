// Tarefa 20: Cópias a cada passo e ordem original dos laços.
// Stencil adaptado do código de Tom Deakin (2018); veja heatCommon.c.
#include <stddef.h>
#include "heatCommon.h"

// Atualiza um passo pelo método explícito de diferenças finitas.
// A entrada é somente leitura; cada iteração escreve uma célula da saída.
static void solve(const int n, const double alpha, const double dx, const double dt, const double * restrict u, double * restrict u_tmp) {

  // Peso dos vizinhos: r; peso da célula central: 1 - 4*r.
  const double r = alpha * dt / (dx * dx);
  const double r2 = 1.0 - 4.0*r;

  // Cada iteração calcula uma célula; os mapas presentes são reutilizados.
  #pragma omp target teams loop device(0) collapse(2) map(to: u[0:n*n]) map(from: u_tmp[0:n*n])
  // Ordem original: j interno avança n posições em i+j*n.
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      // Mesmo stencil e mesmas bordas nulas em todas as versões.
      u_tmp[i+j*n] =  r2 * u[i+j*n] +
        r * ((i < n-1) ? u[i+1+j*n] : 0.0) +
        r * ((i > 0)   ? u[i-1+j*n] : 0.0) +
        r * ((j < n-1) ? u[i+(j+1)*n] : 0.0) +
        r * ((j > 0)   ? u[i+(j-1)*n] : 0.0);
    }
  }
}

void evolve(
  int n, 
  int nsteps, 
  double alpha, 
  double dx, 
  double dt,
  double **campo, 
  double **auxiliar
) {
  double *u = *campo;
  double *u_tmp = *auxiliar;
  for (int t = 0; t < nsteps; ++t) {
    // O kernel termina antes de trocar os campos e iniciar outro passo.
    solve(n, alpha, dx, dt, u, u_tmp);
    double *tmp = u;
    u = u_tmp;
    u_tmp = tmp;
  }
  *campo = u;
  *auxiliar = u_tmp;
}
