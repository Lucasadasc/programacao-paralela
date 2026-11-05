/* Versao 1: collapse(2) e copia dos contornos a cada passo. */
#include "navierStokes.h"

void evoluir(Grade *g, int passos) {
    int n=g->n; double inv=(double)(n-1)*(n-1), fator=0.2/inv;
    double *u=g->u,*v=g->v,*un=g->un,*vn=g->vn;
    for (int p=0;p<passos;p++) {
        #pragma omp parallel for collapse(2) schedule(static)
        for (int i=1;i<n-1;i++) for (int j=1;j<n-1;j++) { ATUALIZAR; }
        /* Duas regioes de contorno como na tarefa11, sem corrida nos cantos. */
        #pragma omp parallel for schedule(static)
        for (int i=1;i<n-1;i++) {
            un[(size_t)i*n]=u[(size_t)i*n]; vn[(size_t)i*n]=v[(size_t)i*n];
            un[(size_t)i*n+n-1]=u[(size_t)i*n+n-1]; vn[(size_t)i*n+n-1]=v[(size_t)i*n+n-1];
        }
        #pragma omp parallel for schedule(static)
        for (int j=0;j<n;j++) {
            un[j]=u[j];vn[j]=v[j];size_t k=(size_t)(n-1)*n+j;un[k]=u[k];vn[k]=v[k];
        }
        double *t=u;u=un;un=t;t=v;v=vn;vn=t;
    }
    g->u=u;g->v=v;g->un=un;g->vn=vn;
}
