#include "calorComum.h"

static void executarPassoComEspera(
    SimulacaoCalor *simulacao, 
    int identificadorProcesso,
    int quantidadeProcessos, 
    MPI_Comm comunicador
) {
    MPI_Request requisicoes[4];
    int quantidadeRecebimentos, inicioAtualizacao, fimAtualizacao;
    
    int quantidadeRequisicoes = iniciarTrocasNaoBloqueantes(
        simulacao, 
        identificadorProcesso,
        quantidadeProcessos, 
        requisicoes, 
        &quantidadeRecebimentos, 
        comunicador
    );

    (void)quantidadeRecebimentos;
    
    for (int indice = 0; indice < quantidadeRequisicoes; ++indice) {
        MPI_Wait(&requisicoes[indice], MPI_STATUS_IGNORE);
    }
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
    for (int passo = 0; passo < quantidadePassos; ++passo) {
        executarPassoComEspera(&simulacao, identificadorProcesso, quantidadeProcessos, MPI_COMM_WORLD);
    }
    mostrarTempo(&simulacao, MPI_Wtime() - inicio, "MPI_Isend/Irecv + Wait", MPI_COMM_WORLD);
    
    liberarSimulacao(&simulacao);
    MPI_Finalize();
    
    return 0;
}
