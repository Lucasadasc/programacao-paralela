/*
 * Tarefa 14 - medicao de latencia/banda com MPI (ping-pong).
 *
 * Executar com exatamente dois processos:
 *   mpirun -np 2 ./ping_pong
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    tamanhoInicial = 8,
    tamanhoMaximo = 1024 * 1024,
    numeroTrocas = 10000
};

int main(int argc, char **argv) {
    int rank, numeroProcessos;
    MPI_Init(&argc, &argv); // Inicializa o ambiente MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtém o identificador do processo (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &numeroProcessos); // Obtém o número total de processos

    if (numeroProcessos != 2) {
        if (rank == 0)
            fprintf(stderr, "Este programa requer exatamente 2 processos (recebidos: %d).\n", numeroProcessos);
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
        puts("tamanho_bytes,trocas,tempo_total_s,tempo_medio_rtt_us,taxa_mensagem_MBps");

    for (long tamanho = tamanhoInicial; tamanho <= tamanhoMaximo;) {
        char *mensagem = malloc((size_t)tamanho);
        if (mensagem == NULL) {
            fprintf(stderr, "Falha ao alocar %ld bytes.\n", tamanho);
            MPI_Finalize();
            return 1;
        }

        // o memset abaixo garante que a mensagem nao seja otimizada pelo compilador
        // e que o trafego de rede seja contabilizado.
        memset(mensagem, 0x5A, (size_t)tamanho); 

        // Sincroniza os processos antes de iniciar a medição
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) { // Envia a primeira mensagem para o processo 1
            MPI_Send(
                mensagem, // mensagem a ser enviada
                (int)tamanho, // tamanho da mensagem em bytes
                MPI_BYTE, // tipo de dado da mensagem
                1, // destino
                0, // tag (identificador da mensagem)
                MPI_COMM_WORLD // comunicador
            );
            MPI_Recv(
                mensagem, // mensagem a ser recebida
                (int)tamanho, // tamanho da mensagem em bytes
                MPI_BYTE, // tipo de dado da mensagem
                1, // origem
                0, // tag (identificador da mensagem)
                MPI_COMM_WORLD, // comunicador
                MPI_STATUS_IGNORE // status da operação
            );
        } else {
            // Recebe a primeira mensagem do processo 0
            MPI_Recv(mensagem, (int)tamanho, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(mensagem, (int)tamanho, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        }
        
        // Sincroniza os processos antes de iniciar a medição
        MPI_Barrier(MPI_COMM_WORLD);
        double inicio = MPI_Wtime();
        for (int i = 0; i < numeroTrocas; ++i) {
            if (rank == 0) {
                MPI_Send(mensagem, (int)tamanho, MPI_BYTE, 1, 1, MPI_COMM_WORLD);
                MPI_Recv(mensagem, (int)tamanho, MPI_BYTE, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Recv(mensagem, (int)tamanho, MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(mensagem, (int)tamanho, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
            }
        }
        double fim = MPI_Wtime();

        if (rank == 0) {
            double tempoTotal = fim - inicio;
            double rttUs = tempoTotal * 1.0e6 / (double)numeroTrocas;
            /* Cada troca transporta a mensagem duas vezes: ida e volta. */
            double mbps = (2.0 * (double)tamanho * (double)numeroTrocas) / tempoTotal / 1.0e6;
            printf("%ld,%d,%.9f,%.3f,%.3f\n", tamanho, numeroTrocas, tempoTotal, rttUs, mbps);
        }

        free(mensagem);
       
        if (tamanho > tamanhoMaximo / 2) {
            break;
        }
        
        tamanho *= 2;
    }

    MPI_Finalize();
    return 0;
}
