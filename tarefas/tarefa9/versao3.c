// compilar: gcc -std=c17 -fopenmp -o versao3 versao3.c
// rodar: export OMP_NUM_THREADS=4; ./versao3 N NUMERO_DE_LISTAS

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
	omp_lock_t *travas;

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
	travas = malloc((size_t)numero_listas * sizeof(*travas));
	if (listas == NULL || travas == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para as listas.\n");
		free(listas);
		free(travas);
		return EXIT_FAILURE;
	}

	for (long i = 0; i < numero_listas; i++) {
		omp_init_lock(&travas[i]);
	}

	#pragma omp parallel shared(listas, travas, numero_listas)
	{
		#pragma omp single
		{
			for (long i = 0; i < n; i++) {
				unsigned int semente = (unsigned int)time(NULL) ^
					(unsigned int)(i * 2654435761UL);

				#pragma omp task firstprivate(i, semente) shared(listas, travas, numero_listas)
				{
					No *novo_no = criar_no((int)i);
					long escolha;

					if (novo_no == NULL) {
						fprintf(stderr, "Erro ao alocar memoria para uma insercao.\n");
						exit(EXIT_FAILURE);
					}

					escolha = rand_r(&semente) % numero_listas;
					omp_set_lock(&travas[escolha]);
					novo_no->proximo = listas[escolha];
					listas[escolha] = novo_no;
					omp_unset_lock(&travas[escolha]);
				}
			}
		}
	}

	for (long i = 0; i < numero_listas; i++) {
		imprimir_lista((size_t)(i + 1), listas[i]);
		liberar_lista(listas[i]);
		omp_destroy_lock(&travas[i]);
	}

	free(listas);
	free(travas);
	return EXIT_SUCCESS;
}
