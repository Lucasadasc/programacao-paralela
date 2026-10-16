/* Evolucao do solver viscoso FTCS da tarefa11: dominio [0,1]^2, nu=0.05. */
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USAR_PASCAL
#include "pascalops.h"
#define INICIAR_REGIAO(id) pascal_start(id)
#define TERMINAR_REGIAO(id) pascal_stop(id)
#else
#define INICIAR_REGIAO(id) ((void)0)
#define TERMINAR_REGIAO(id) ((void)0)
#endif

#include "navierStokes.h"
#ifndef VERSAO
#error Defina VERSAO ao compilar com uma implementacao navierStokesV1, V2 ou V3.
#endif
static int inteiro(const char *s, int minimo, int maximo) {
    char *fim; errno = 0; long x = strtol(s, &fim, 10);
    if (errno || !*s || *fim || x < minimo || x > maximo) {
        fprintf(stderr, "Argumento invalido: %s\n", s); exit(EXIT_FAILURE);
    }
    return (int)x;
}
static Grade criar(int n) {
    Grade g = {.n=n}; size_t bytes = (size_t)n*n*sizeof(double);
    g.u=malloc(bytes); g.v=malloc(bytes); g.un=malloc(bytes); g.vn=malloc(bytes);
    if (!g.u || !g.v || !g.un || !g.vn) { fputs("Falha de alocacao\n",stderr); exit(1); }
    return g;
}
static void liberar(Grade g) { free(g.u); free(g.v); free(g.un); free(g.vn); }
static void inicializar(Grade g, int paralelo, int cenario) {
    int n=g.n;
    #pragma omp parallel for if(paralelo) schedule(static)
    for (int i=0;i<n;i++) for (int j=0;j<n;j++) {
        size_t k=(size_t)i*n+j;
        double x=(double)i/(n-1)-0.5, y=(double)j/(n-1)-0.5;
        double u=cenario==0 ? 0 : cenario==1 ? 1.5 : 2*exp(-(x*x+y*y)/(2*0.08*0.08));
        double v=cenario==1 ? 0.5 : u*0.5;
        if (cenario!=1 && (i==0 || j==0 || i==n-1 || j==n-1)) u=v=0;
        g.u[k]=g.un[k]=u; g.v[k]=g.vn[k]=v;
    }
}
static void referencia(Grade *g, int passos) {
    int n=g->n; double inv=(double)(n-1)*(n-1), fator=0.2/inv;
    double *u=g->u,*v=g->v,*un=g->un,*vn=g->vn;
    for (int p=0;p<passos;p++) {
        for (int i=1;i<n-1;i++) for (int j=1;j<n-1;j++) { ATUALIZAR; }
        double *t=u;u=un;un=t;t=v;v=vn;vn=t;
    }
    g->u=u;g->v=v;g->un=un;g->vn=vn;
}
static int validar(void) {
    int falhas=0;
    for (int c=0;c<3;c++) for (int passos=1;passos<=12;passos+=11) {
        Grade ref=criar(31);omp_set_num_threads(1);inicializar(ref,0,c);referencia(&ref,passos);
        for (int th=1;th<=4;th*=2) {
            Grade g=criar(31);omp_set_num_threads(th);inicializar(g,VERSAO==3,c);evoluir(&g,passos);
            for (size_t k=0;k<31*31;k++) {
                if (!isfinite(g.u[k]) || !isfinite(g.v[k]) || fabs(g.u[k]-ref.u[k])>1e-12 || fabs(g.v[k]-ref.v[k])>1e-12) falhas++;
                if (c<2 && (g.u[k]!=(c==1?1.5:0) || g.v[k]!=(c==1?0.5:0))) falhas++;
                if (c==2 && (g.u[k]<0 || g.u[k]>2 || fabs(g.v[k]-g.u[k]*0.5)>1e-12)) falhas++;
            }
            liberar(g);
        }
        liberar(ref);
    }
    printf("Validacao V%d: %s (%d falhas)\n",VERSAO,falhas?"FALHOU":"OK",falhas);return falhas?1:0;
}
int main(int argc,char **argv) {
    omp_set_dynamic(0);
    if (argc==2 && strcmp(argv[1], "--validar")==0) return validar();
#ifdef USAR_PASCAL
    if (argc!=3) {
        fprintf(stderr,"Uso: %s PONTOS_INTERNOS PASSOS; threads via OMP_NUM_THREADS\n",argv[0]);return 1;
    }
    int pontos=inteiro(argv[1],36,268369924);
    int lado=(int)sqrt((double)pontos);
    if ((long long)lado*lado!=pontos) {
        fputs("PONTOS_INTERNOS deve ser um quadrado perfeito\n",stderr);return 1;
    }
    int ver=VERSAO,n=lado+2,passos=inteiro(argv[2],1,1000000),th=omp_get_max_threads();
#else
    if (argc!=4) { fprintf(stderr,"Uso: %s N(8..16384) PASSOS THREADS | --validar\n",argv[0]);return 1; }
    int ver=VERSAO,n=inteiro(argv[1],8,16384),passos=inteiro(argv[2],1,1000000),th=inteiro(argv[3],1,4096);
#endif
    omp_set_num_threads(th);int reais=0;
    #pragma omp parallel
    {
        #pragma omp single
        reais=omp_get_num_threads();
    }
    if (reais!=th) { fprintf(stderr,"Solicitadas %d threads, obtidas %d\n",th,reais);return 1; }
    Grade g=criar(n);inicializar(g,ver==3,2);evoluir(&g,5); /* aquecimento */
    INICIAR_REGIAO(1);
    inicializar(g,ver==3,2);
    TERMINAR_REGIAO(1);
    INICIAR_REGIAO(2);
    double inicio=omp_get_wtime();evoluir(&g,passos);double tempo=omp_get_wtime()-inicio;
    TERMINAR_REGIAO(2);
    double soma=0,pico=0;
    for (size_t k=0;k<(size_t)n*n;k++) { soma+=g.u[k]+g.v[k];if(g.u[k]>pico)pico=g.u[k]; }
    if (!isfinite(soma) || pico>2 || pico<0) { fputs("Resultado numerico invalido\n",stderr);liberar(g);return 1; }
    printf("%d,%d,%d,%d,%.9f,%.17g,%.17g\n",ver,n,passos,reais,tempo,soma,pico);
    liberar(g);return 0;
}
