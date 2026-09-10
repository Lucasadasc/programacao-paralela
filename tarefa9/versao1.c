// compilar: gcc -std=c17 -fopenmp -o versao1 versao1.c
// rodar: export OMP_NUM_THREADS=4; ./versao1 N

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct No {
	int valor;
	struct No *proximo;
} No;

static No *criar_no(int valor) {
	No *novo_no = malloc(sizeof(*novo_no));

	if (novo_no != NULL) {
		novo_no->valor = valor;
		novo_no->proximo = NULL;
	}

	return novo_no;
}

static void liberar_lista(No *lista) {
	while (lista != NULL) {
		No *proximo = lista->proximo;
		free(lista);
		lista = proximo;
	}
}

static size_t tamanho_lista(const No *lista) {
	size_t tamanho = 0;

	while (lista != NULL) {
		tamanho++;
		lista = lista->proximo;
	}

	return tamanho;
}

static void imprimir_lista(const char *nome, const No *lista) {
	printf("%s (%zu elementos):", nome, tamanho_lista(lista));

	while (lista != NULL) {
		printf(" %d", lista->valor);
		lista = lista->proximo;
	}

	putchar('\n');
}

int main(int argc, char **argv) {
	char *fim;
	long n;
	No *lista1 = NULL;
	No *lista2 = NULL;

	if (argc != 2) {
		fprintf(stderr, "Uso: %s N\n", argv[0]);
		return EXIT_FAILURE;
	}

	n = strtol(argv[1], &fim, 10);
	if (*argv[1] == '\0' || *fim != '\0' || n < 0) {
		fprintf(stderr, "N deve ser um inteiro nao negativo.\n");
		return EXIT_FAILURE;
	}

	#pragma omp parallel shared(lista1, lista2)
	{
		#pragma omp single // (cada uma associada a uma thread)
		{
			for (long i = 0; i < n; i++) {
				unsigned int semente = (unsigned int)time(NULL) ^ (unsigned int)(i * 2654435761UL); // Gera uma semente diferente para cada iteração

				#pragma omp task firstprivate(i, semente) shared(lista1, lista2)
				{
					No *novo_no = criar_no((int)i);
					int escolha;

					if (novo_no == NULL) {
						fprintf(stderr, "Erro ao alocar memoria para uma insercao.\n");
						exit(EXIT_FAILURE);
					}

					escolha = (int)(rand_r(&semente) % 2);
					if (escolha == 0) {
                        //usando o critical para proteger a lista1
                        // sem o critical, a lista1 pode ser corrompida se duas threads tentarem inserir ao mesmo tempo
						#pragma omp critical(lista1) 
						{
							novo_no->proximo = lista1;
							lista1 = novo_no;
						}
					} else {
						#pragma omp critical(lista2)
						{
							novo_no->proximo = lista2;
							lista2 = novo_no;
						}
					}
				}
			}

		}
	}

	imprimir_lista("Lista 1", lista1);
	imprimir_lista("Lista 2", lista2);

	liberar_lista(lista1);
	liberar_lista(lista2);
	return EXIT_SUCCESS;
}
