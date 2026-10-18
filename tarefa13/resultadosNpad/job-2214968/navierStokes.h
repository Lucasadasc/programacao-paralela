#ifndef NAVIER_STOKES_H
#define NAVIER_STOKES_H
#include <stddef.h>
typedef struct { int n; double *u, *v, *un, *vn; } Grade;
void evoluir(Grade *g, int passos);

/* Mesma ordem aritmetica do solver original. CFL: sigma_x+sigma_y=0.4. */
#define ATUALIZAR \
    size_t k=(size_t)i*n+j; \
    un[k]=u[k]+fator*((u[k+n]-2*u[k]+u[k-n])*inv+(u[k+1]-2*u[k]+u[k-1])*inv); \
    vn[k]=v[k]+fator*((v[k+n]-2*v[k]+v[k-n])*inv+(v[k+1]-2*v[k]+v[k-1])*inv)
#endif
