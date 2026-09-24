#include "calorComum.h"

#define INTERVALO_MPI_TEST 100

#define TEXTO_INTERNO(valor) #valor
#define TEXTO(valor) TEXTO_INTERNO(valor)

static int recebimentosConcluidos(MPI_Request requisicoes[4], int quantidadeRecebimentos) {
    int todosConcluidos = 1;
    
    for (int indice = 0; indice < quantidadeRecebimentos; ++indice) {
        int concluiu;
        MPI_Test(&requisicoes[indice], &concluiu, MPI_STATUS_IGNORE);
        todosConcluidos &= concluiu;
    }
    
    return todosConcluidos;
}

static void executarPassoComSobreposicao(
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
    
    prepararProximoPasso(simulacao, &inicioAtualizacao, &fimAtualizacao);

    int inicioInterno = inicioAtualizacao > 2 ? inicioAtualizacao : 2;
    
    int fimInterno;
    if (fimAtualizacao < simulacao->quantidadeCelulasLocais - 1){
        fimInterno = fimAtualizacao;
    } else {
        fimInterno = simulacao->quantidadeCelulasLocais - 1;
    }
    
    int indiceLocal = inicioInterno;
    int halosRecebidos = 0;
    while (indiceLocal <= fimInterno && !halosRecebidos) {
        int fimBloco = indiceLocal + INTERVALO_MPI_TEST - 1;
        if (fimBloco > fimInterno) {
            fimBloco = fimInterno;
        }

        calcularIntervalo(simulacao, indiceLocal, fimBloco);
        indiceLocal = fimBloco + 1;
        halosRecebidos = recebimentosConcluidos(requisicoes, quantidadeRecebimentos);
    }

    calcularIntervalo(simulacao, indiceLocal, fimInterno);

    if (!halosRecebidos) {
        for (int indice = 0; indice < quantidadeRequisicoes; ++indice) {
            MPI_Wait(&requisicoes[indice], MPI_STATUS_IGNORE);
        }
    }
    if (inicioAtualizacao <= 1 && fimAtualizacao >= 1){
        calcularCelula(simulacao, 1);
    }
    if (inicioAtualizacao <= simulacao->quantidadeCelulasLocais &&
        fimAtualizacao >= simulacao->quantidadeCelulasLocais) {
            calcularCelula(simulacao, simulacao->quantidadeCelulasLocais);
        }
    if (halosRecebidos) {
        for (int indice = 0; indice < quantidadeRequisicoes; ++indice) {
            MPI_Wait(&requisicoes[indice], MPI_STATUS_IGNORE);
        }
    }
        
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
    
    inicializarSimulacao(
        &simulacao, 
        quantidadeCelulas, 
        fatorDifusao, 
        MPI_COMM_WORLD
    );

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    for (int passo = 0; passo < quantidadePassos; ++passo){
        executarPassoComSobreposicao(
            &simulacao, 
            identificadorProcesso, 
            quantidadeProcessos, 
            MPI_COMM_WORLD
        );
    }
        
    mostrarTempo(
        &simulacao,
        MPI_Wtime() - inicio,
        "MPI_Test a cada " TEXTO(INTERVALO_MPI_TEST),
        MPI_COMM_WORLD
    );
    liberarSimulacao(&simulacao);
    MPI_Finalize();
    
    return 0;
}
