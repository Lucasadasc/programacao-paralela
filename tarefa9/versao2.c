// compilar: gcc -std=c17 -fopenmp -o versao2 versao2.c
// rodar: export OMP_NUM_THREADS=4; ./versao2 N NUMERO_DE_LISTAS

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

static void imprimir_lista(size_t numero, const No *lista) {
	printf("Lista %zu (%zu elementos):", numero, tamanho_lista(lista));

	while (lista != NULL) {
		printf(" %d", lista->valor);
		lista = lista->proximo;
	}

	putchar('\n');
}

int main(int argc, char **argv) {
	char *fim;
	long n;
	long numero_listas;
	No **listas;

	if (argc != 3) {
		fprintf(stderr, "Uso: %s N NUMERO_DE_LISTAS\n", argv[0]);
		return EXIT_FAILURE;
	}

	n = strtol(argv[1], &fim, 10);
	if (*argv[1] == '\0' || *fim != '\0' || n < 0) {
		fprintf(stderr, "N deve ser um inteiro nao negativo.\n");
		return EXIT_FAILURE;
	}

	numero_listas = strtol(argv[2], &fim, 10);
	if (*argv[2] == '\0' || *fim != '\0' || numero_listas <= 0) {
		fprintf(stderr, "O numero de listas deve ser um inteiro positivo.\n");
		return EXIT_FAILURE;
	}

	listas = calloc((size_t)numero_listas, sizeof(*listas));
	if (listas == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para as listas.\n");
		free(listas);
		return EXIT_FAILURE;
	}

	#pragma omp parallel shared(listas, numero_listas)
	{
		#pragma omp single
		{
			for (long i = 0; i < n; i++) {
				unsigned int semente = (unsigned int)time(NULL) ^
					(unsigned int)(i * 2654435761UL);

				#pragma omp task firstprivate(i, semente) shared(listas, numero_listas)
				{
					No *novo_no = criar_no((int)i);
					long escolha;

					if (novo_no == NULL) {
						fprintf(stderr, "Erro ao alocar memoria para uma insercao.\n");
						exit(EXIT_FAILURE);
					}

					escolha = rand_r(&semente) % numero_listas;
					novo_no->proximo = listas[escolha];
					listas[escolha] = novo_no;
				}
			}
		}
	}

	for (long i = 0; i < numero_listas; i++) {
		imprimir_lista((size_t)(i + 1), listas[i]);
		liberar_lista(listas[i]);
	}

	free(listas);
	return EXIT_SUCCESS;
}
