#ifndef CALOR_COMUM_H
#define CALOR_COMUM_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

enum { tagParaEsquerda = 10, tagParaDireita = 11 };

typedef struct {
    int quantidadeCelulas;
    int quantidadeCelulasLocais;
    int primeiraCelulaGlobal;
    double fatorDifusao;
    double temperaturaEsquerda;
    double temperaturaDireita;
    double *temperaturaAtual;
    double *proximaTemperatura;
} SimulacaoCalor;

static inline void encerrarComErro(int condicao, const char *mensagem, MPI_Comm comunicador) {
    if (condicao) { 
        fprintf(stderr, "%s\n", mensagem); 
        MPI_Abort(comunicador, 1); 
    }
}

static inline void lerArgumentos(
    int quantidadeArgumentos, 
    char **argumentos, 
    int *quantidadeCelulas, 
    int *quantidadePassos, 
    double *fatorDifusao, 
    MPI_Comm comunicador
) {
    *quantidadeCelulas = quantidadeArgumentos > 1 ? atoi(argumentos[1]) : 1000000;
    *quantidadePassos = quantidadeArgumentos > 2 ? atoi(argumentos[2]) : 1000;
    *fatorDifusao = quantidadeArgumentos > 3 ? atof(argumentos[3]) : 0.24;
    
    encerrarComErro(
        *quantidadeCelulas < 4 || *quantidadePassos < 1 || *fatorDifusao <= 0.0 || *fatorDifusao > 0.5, 
        "Uso: programa [celulas>=4] [passos>=1] [alpha em (0,0.5)]", 
        comunicador
    );
}

static inline void inicializarSimulacao(SimulacaoCalor *simulacao, int quantidadeCelulas, double fatorDifusao, MPI_Comm comunicador) {
    int identificadorProcesso, quantidadeProcessos;
    MPI_Comm_rank(comunicador, &identificadorProcesso); MPI_Comm_size(comunicador, &quantidadeProcessos);
    encerrarComErro(quantidadeProcessos < 2, "Execute com pelo menos 2 processos MPI.", comunicador);
    encerrarComErro(quantidadeCelulas < 2 * quantidadeProcessos, "N deve ser pelo menos duas vezes o numero de processos.", comunicador);
    int tamanhoBase = quantidadeCelulas / quantidadeProcessos;
    int celulasRestantes = quantidadeCelulas % quantidadeProcessos;
    simulacao->quantidadeCelulas = quantidadeCelulas;
    simulacao->quantidadeCelulasLocais = tamanhoBase + (identificadorProcesso < celulasRestantes);
    simulacao->primeiraCelulaGlobal = identificadorProcesso * tamanhoBase + (identificadorProcesso < celulasRestantes ? identificadorProcesso : celulasRestantes);
    simulacao->fatorDifusao = fatorDifusao; simulacao->temperaturaEsquerda = 100.0; simulacao->temperaturaDireita = 0.0;
    simulacao->temperaturaAtual = calloc((size_t)simulacao->quantidadeCelulasLocais + 2, sizeof *simulacao->temperaturaAtual);
    simulacao->proximaTemperatura = calloc((size_t)simulacao->quantidadeCelulasLocais + 2, sizeof *simulacao->proximaTemperatura);
    encerrarComErro(!simulacao->temperaturaAtual || !simulacao->proximaTemperatura, "Falha ao alocar memoria.", comunicador);
    for (int indiceLocal = 1; indiceLocal <= simulacao->quantidadeCelulasLocais; ++indiceLocal) {
        int indiceGlobal = simulacao->primeiraCelulaGlobal + indiceLocal - 1;
        simulacao->temperaturaAtual[indiceLocal] = 20.0 + 10.0 * indiceGlobal / (double)(quantidadeCelulas - 1);
    }
    if (simulacao->primeiraCelulaGlobal == 0) simulacao->temperaturaAtual[1] = simulacao->temperaturaEsquerda;
    if (simulacao->primeiraCelulaGlobal + simulacao->quantidadeCelulasLocais == quantidadeCelulas) simulacao->temperaturaAtual[simulacao->quantidadeCelulasLocais] = simulacao->temperaturaDireita;
}

static inline void prepararProximoPasso(SimulacaoCalor *simulacao, int *inicioAtualizacao, int *fimAtualizacao) {
    *inicioAtualizacao = 1; *fimAtualizacao = simulacao->quantidadeCelulasLocais;
    if (simulacao->primeiraCelulaGlobal == 0) { simulacao->proximaTemperatura[1] = simulacao->temperaturaEsquerda; *inicioAtualizacao = 2; }
    if (simulacao->primeiraCelulaGlobal + simulacao->quantidadeCelulasLocais == simulacao->quantidadeCelulas) { simulacao->proximaTemperatura[simulacao->quantidadeCelulasLocais] = simulacao->temperaturaDireita; *fimAtualizacao = simulacao->quantidadeCelulasLocais - 1; }
}

static inline void calcularCelula(SimulacaoCalor *simulacao, int indiceLocal) {
    double *temperaturas = simulacao->temperaturaAtual;
    simulacao->proximaTemperatura[indiceLocal] = temperaturas[indiceLocal] + simulacao->fatorDifusao * (temperaturas[indiceLocal - 1] - 2.0 * temperaturas[indiceLocal] + temperaturas[indiceLocal + 1]);
}
static inline void calcularIntervalo(SimulacaoCalor *simulacao, int inicio, int fim) { for (int indiceLocal = inicio; indiceLocal <= fim; ++indiceLocal) calcularCelula(simulacao, indiceLocal); }
static inline void trocarTemperaturas(SimulacaoCalor *simulacao) { double *temporario = simulacao->temperaturaAtual; simulacao->temperaturaAtual = simulacao->proximaTemperatura; simulacao->proximaTemperatura = temporario; }

static inline int iniciarTrocasNaoBloqueantes(
    SimulacaoCalor *simulacao, 
    int identificadorProcesso, 
    int quantidadeProcessos, 
    MPI_Request requisicoes[4], 
    int *quantidadeRecebimentos, 
    MPI_Comm comunicador
) {
    int quantidadeRequisicoes = 0;
    
    // Se o processo não for o primeiro, ele deve receber a temperatura do processo à esquerda.
    if (identificadorProcesso > 0){
        MPI_Irecv(
            &simulacao->temperaturaAtual[0], 
            1, 
            MPI_DOUBLE, 
            identificadorProcesso - 1, 
            tagParaEsquerda, 
            comunicador, 
            &requisicoes[quantidadeRequisicoes++]
        );
    }
    
    // Se o processo não for o último, ele deve receber a temperatura do processo à direita.
    if (identificadorProcesso < quantidadeProcessos - 1){
        MPI_Irecv(
            &simulacao->temperaturaAtual[simulacao->quantidadeCelulasLocais + 1], 
            1, 
            MPI_DOUBLE, 
            identificadorProcesso + 1, 
            tagParaDireita, 
            comunicador, 
            &requisicoes[quantidadeRequisicoes++]
        );
    }
    
    // Se o processo não for o primeiro, ele deve enviar a temperatura da primeira célula local para o processo à esquerda.
    *quantidadeRecebimentos = quantidadeRequisicoes;
    if (identificadorProcesso > 0) {
        MPI_Isend(
            &simulacao->temperaturaAtual[1], 
            1, 
            MPI_DOUBLE, 
            identificadorProcesso - 1, 
            tagParaDireita, 
            comunicador, 
            &requisicoes[quantidadeRequisicoes++]
        );
    }
    // Se o processo não for o último, ele deve enviar a temperatura da última célula local para o processo à direita.
    if (identificadorProcesso < quantidadeProcessos - 1) {
        MPI_Isend(
            &simulacao->temperaturaAtual[simulacao->quantidadeCelulasLocais], 
            1, 
            MPI_DOUBLE, 
            identificadorProcesso + 1, 
            tagParaEsquerda, 
            comunicador, 
            &requisicoes[quantidadeRequisicoes++]
        );
    }
    
    return quantidadeRequisicoes;
}

static inline void mostrarTempo(const SimulacaoCalor *simulacao, double inicio, const char *nomeVersao, MPI_Comm comunicador) {
    int identificadorProcesso;
    MPI_Comm_rank(comunicador, &identificadorProcesso);
    MPI_Barrier(comunicador);
    if (identificadorProcesso == 0) printf("%-22s tempo_total = %.6f s | T[0] = %.6f\n", nomeVersao, MPI_Wtime() - inicio, simulacao->temperaturaAtual[1]);
}
static inline void liberarSimulacao(SimulacaoCalor *simulacao) { free(simulacao->temperaturaAtual); free(simulacao->proximaTemperatura); }

#endif
