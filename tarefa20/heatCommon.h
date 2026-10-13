// Interface comum às três versões da tarefa 20.
// serve para evitar múltiplas inclusões do mesmo arquivo de cabeçalho.
#pragma once

void initial_value(int n, double dx, double length, double * restrict u);
void zero(int n, double * restrict u);
double solution(double t, double x, double y, double alpha, double length);
double l2norm(int n, const double * restrict u, int nsteps, double dt,
              double alpha, double dx, double length);

// Cada versão implementa a evolução e devolve os dois ponteiros atualizados.
void evolve(int n, int nsteps, double alpha, double dx, double dt,
            double **campo, double **auxiliar);
