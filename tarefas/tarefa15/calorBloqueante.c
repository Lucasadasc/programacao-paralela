#include "calorComum.h"

static void trocarHalosBloqueante(
    SimulacaoCalor *simulacao,
    int identificadorProcesso,
    int quantidadeProcessos,
    MPI_Comm comunicador
) {
    int vizinhoEsquerda = identificadorProcesso - 1;
    int vizinhoDireita = identificadorProcesso + 1;
    int ultimaCelula = simulacao->quantidadeCelulasLocais;

    /* Pares e impares seguem ordens opostas para evitar espera circular. */
    // Se o processo for par, ele envia primeiro para a direita e depois para a esquerda.
    if (identificadorProcesso % 2 == 0) { 
        // verifica se o vizinho da direita existe antes de enviar e receber
        if (vizinhoDireita < quantidadeProcessos)
            MPI_Send(&simulacao->temperaturaAtual[ultimaCelula], 1, MPI_DOUBLE,
                     vizinhoDireita, tagParaEsquerda, comunicador);
        // verifica se o vizinho da direita existe antes de enviar e receber
        if (vizinhoDireita < quantidadeProcessos)
            MPI_Recv(&simulacao->temperaturaAtual[ultimaCelula + 1], 1, MPI_DOUBLE,
                     vizinhoDireita, tagParaDireita, comunicador, MPI_STATUS_IGNORE);
        // verifica se o vizinho da esquerda existe antes de enviar e receber
        if (vizinhoEsquerda >= 0)
            MPI_Send(&simulacao->temperaturaAtual[1], 1, MPI_DOUBLE,
                     vizinhoEsquerda, tagParaDireita, comunicador);
        if (vizinhoEsquerda >= 0)
            MPI_Recv(&simulacao->temperaturaAtual[0], 1, MPI_DOUBLE,
                     vizinhoEsquerda, tagParaEsquerda, comunicador, MPI_STATUS_IGNORE);
    // Se o processo for ímpar, ele recebe primeiro da esquerda e depois da direita
    } else {
        // verifica se o vizinho da esquerda existe antes de enviar e receber
        if (vizinhoEsquerda >= 0)
            MPI_Recv(&simulacao->temperaturaAtual[0], 1, MPI_DOUBLE,
                     vizinhoEsquerda, tagParaEsquerda, comunicador, MPI_STATUS_IGNORE);
        if (vizinhoEsquerda >= 0)
            MPI_Send(&simulacao->temperaturaAtual[1], 1, MPI_DOUBLE,
                     vizinhoEsquerda, tagParaDireita, comunicador);
        // verifica se o vizinho da direita existe antes de enviar e receber
        if (vizinhoDireita < quantidadeProcessos)
            MPI_Recv(&simulacao->temperaturaAtual[ultimaCelula + 1], 1, MPI_DOUBLE,
                     vizinhoDireita, tagParaDireita, comunicador, MPI_STATUS_IGNORE);
        if (vizinhoDireita < quantidadeProcessos)
            MPI_Send(&simulacao->temperaturaAtual[ultimaCelula], 1, MPI_DOUBLE,
                     vizinhoDireita, tagParaEsquerda, comunicador);
    }
}

static void executarPassoBloqueante(
    SimulacaoCalor *simulacao,
    int identificadorProcesso,
    int quantidadeProcessos,
    MPI_Comm comunicador
) {
    int inicioAtualizacao;
    int fimAtualizacao;

    trocarHalosBloqueante(simulacao, identificadorProcesso, quantidadeProcessos, comunicador);
    prepararProximoPasso(simulacao, &inicioAtualizacao, &fimAtualizacao);
    calcularIntervalo(simulacao, inicioAtualizacao, fimAtualizacao);
    trocarTemperaturas(simulacao);
}

int main(int quantidadeArgumentos, char **argumentos) {
    MPI_Init(&quantidadeArgumentos, &argumentos);
    
    int identificadorProcesso, quantidadeProcessos, quantidadeCelulas, quantidadePassos;
    double fatorDifusao;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &identificadorProcesso);
    MPI_Comm_size(MPI_COMM_WORLD, &quantidadeProcessos);
    
    lerArgumentos(
        quantidadeArgumentos, 
        argumentos, 
        &quantidadeCelulas,
        &quantidadePassos, 
        &fatorDifusao, 
        MPI_COMM_WORLD
    );
    SimulacaoCalor simulacao;
    
    inicializarSimulacao(&simulacao, quantidadeCelulas, fatorDifusao, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    
    double inicio = MPI_Wtime();
    for (int passo = 0; passo < quantidadePassos; ++passo){
        executarPassoBloqueante(
            &simulacao, 
            identificadorProcesso, 
            quantidadeProcessos,
            MPI_COMM_WORLD
        );
    }
        
    mostrarTempo(&simulacao, inicio, "MPI_Send/MPI_Recv", MPI_COMM_WORLD);
    liberarSimulacao(&simulacao);
    MPI_Finalize();
    
    return 0;
}
