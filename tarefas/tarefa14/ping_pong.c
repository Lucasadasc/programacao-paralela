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
    char *mensagem = malloc(tamanhoMaximo);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numeroProcessos);

    if (numeroProcessos != 2) {
        if (rank == 0)
            fprintf(stderr, "Use exatamente 2 processos.\n");
        MPI_Finalize();
        return 1;
    }

    // Preenche a mensagem com 'a' para completar o tamanho máximo, 
    // garantindo que a memória esteja alocada e inicializada.
    memset(mensagem, 'a', tamanhoMaximo);
    if (rank == 0)
        puts("tamanhoBytes,tempoTotalSegundos");

    for (int tamanho = tamanhoInicial; tamanho <= tamanhoMaximo; tamanho *= 2) {
        MPI_Barrier(MPI_COMM_WORLD);
        double inicio = MPI_Wtime();

        for (int i = 0; i < numeroTrocas; i++) {
            if (rank == 0) {
                MPI_Send(
                    mensagem, // Envia a mensagem para o processo 1
                    tamanho, // Tamanho da mensagem em bytes
                    MPI_CHAR, // Tipo de dado da mensagem
                    1, // Destino da mensagem (processo 1)
                    0, // Tag da mensagem
                    MPI_COMM_WORLD // Comunicador global
                );
                MPI_Recv(
                    mensagem, // Recebe a mensagem do processo 1
                    tamanho, // Tamanho da mensagem em bytes
                    MPI_CHAR, // Tipo de dado da mensagem
                    1, // Origem da mensagem (processo 1)
                    0, // Tag da mensagem
                    MPI_COMM_WORLD, // Comunicador global
                    MPI_STATUS_IGNORE // Status da operação (não é usado)
                );
            } else {
                MPI_Recv(mensagem, tamanho, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(mensagem, tamanho, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        if (rank == 0)
            printf("%d,%.9f\n", tamanho, MPI_Wtime() - inicio);
    }

    free(mensagem);
    MPI_Finalize();
    return 0;
}
