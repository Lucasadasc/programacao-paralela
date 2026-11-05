#include "calorComum.h"

#define INTERVALO_MPI_TEST 100

#define TEXTO_INTERNO(valor) #valor
#define TEXTO(valor) TEXTO_INTERNO(valor)

static int recebimentosConcluidos(MPI_Request requisicoes[4], int quantidadeRecebimentos) {
    int todosConcluidos = 1;
    
    for (int indice = 0; indice < quantidadeRecebimentos; ++indice) {
        int concluiu;
        // Testa se a requisição foi concluída sem bloquear a execução.
        MPI_Test(&requisicoes[indice], &concluiu, MPI_STATUS_IGNORE);
        // Se alguma requisição não foi concluída, marca como falso.
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
    // Se o fim da atualização for menor que a última célula local, 
    // define o fim interno como o fim da atualização. 
    // Caso contrário, define como a penúltima célula local.
    if (fimAtualizacao < simulacao->quantidadeCelulasLocais - 1){
        fimInterno = fimAtualizacao;
    } else {
        fimInterno = simulacao->quantidadeCelulasLocais - 1;
    }
    
    int indiceLocal = inicioInterno;
    int halosRecebidos = 0;
    // Enquanto não tiver recebido os halos, 
    // calcula blocos de células e verifica se os halos foram recebidos.
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

    // Se ainda não recebeu os halos, espera até que todas as requisições sejam concluídas.
    if (!halosRecebidos) {
        for (int indice = 0; indice < quantidadeRequisicoes; ++indice) {
            MPI_Wait(&requisicoes[indice], MPI_STATUS_IGNORE);
        }
    }

    // Calcula as células das bordas, se necessário.
    // Calcula a primeira célula local se estiver dentro do intervalo de atualização.
    if (inicioAtualizacao <= 1 && fimAtualizacao >= 1){
        calcularCelula(simulacao, 1);
    }
    // Calcula a última célula local se estiver dentro do intervalo de atualização.
    if (inicioAtualizacao <= simulacao->quantidadeCelulasLocais &&
        fimAtualizacao >= simulacao->quantidadeCelulasLocais) {
            calcularCelula(simulacao, simulacao->quantidadeCelulasLocais);
        }
    // Se ainda não recebeu os halos, espera até que todas as requisições sejam concluídas.
    if (halosRecebidos) {
        for (int indice = 0; indice < quantidadeRequisicoes; ++indice) {
            MPI_Wait(&requisicoes[indice], MPI_STATUS_IGNORE);
        }
    }
    
    // Troca os ponteiros das temperaturas atual e próxima.
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
        inicio,
        "MPI_Test a cada " TEXTO(INTERVALO_MPI_TEST),
        MPI_COMM_WORLD
    );
    liberarSimulacao(&simulacao);
    MPI_Finalize();
    
    return 0;
}
