/* Versao 3: Regiao paralela persistente com ponteiros privados e SIMD. */
#include "navierStokes.h"

void evoluir(Grade *g, int passos) {
    int n=g->n; double inv=(double)(n-1)*(n-1), fator=0.2/inv;
    double *u=g->u,*v=g->v,*un=g->un,*vn=g->vn;
    #pragma omp parallel firstprivate(u,v,un,vn)
    {
        for (int p=0;p<passos;p++) {
            #pragma omp for schedule(static)
            for (int i=1;i<n-1;i++) {
                #pragma omp simd
                for (int j=1;j<n-1;j++) { ATUALIZAR; }
            }
            /* Barreira do omp for termina todas as escritas antes da troca.
               Ponteiros privados dispensam single e segunda barreira. */
            double *t=u;u=un;un=t;t=v;v=vn;vn=t;
        }
    }
    if (passos%2) { double *t=u;u=un;un=t;t=v;v=vn;vn=t; }
    g->u=u;g->v=v;g->un=un;g->vn=vn;
}
