// Compilar: gcc -fopenmp tasks_corrigidas.c -o tasks_corrigidas
// Executar com número de threads definido: export OMP_NUM_THREADS=4; ./tasks_corrigidas

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

typedef struct No {
	char nome_arquivo[80];
	struct No *proximo;
} No;

No *criar_no(const char *nome_arquivo, No *proximo) {
	No *novo_no = malloc(sizeof(No));

	if (novo_no == NULL) {
		return NULL;
	}

	snprintf(novo_no->nome_arquivo, sizeof(novo_no->nome_arquivo), "%s", nome_arquivo);
	novo_no->proximo = proximo;
	return novo_no;
}

int inserir_no_inicio(No **lista, const char *nome_arquivo) {
	No *novo_no = criar_no(nome_arquivo, *lista);

	if (novo_no == NULL) {
		return 0;
	}

	*lista = novo_no;
	return 1;
}

void liberar_lista(No *lista) {
	while (lista != NULL) {
		No *proximo = lista->proximo;
		free(lista);
		lista = proximo;
	}
}

int main(void) {
	const char *musicas[] = {
		"1_Atrasadinha.mp3",
		"2_Pirata_e_Tesouro.mp3",
		"3_Eu_Juro.mp3",
		"4_Sinto_Sua_Falta.mp3",
		"5_Pra_Voce_Acreditar.mp3",
		"6_Climatizar.mp3",
		"7_Me_Perdoa.mp3",
		"8_Casa_do_Amor.mp3",
		"9_Tristinha.mp3",
		"10_To_Isso_Aqui_Pra_Te_Beijar.mp3"
	};
	const size_t quantidade = sizeof(musicas) / sizeof(musicas[0]);
	No *lista = NULL;

	for (size_t i = 0; i < quantidade; i++) {
		if (!inserir_no_inicio(&lista, musicas[i])) {
			fprintf(stderr, "Erro ao alocar memoria para a lista.\n");
			liberar_lista(lista);
			return 0;
		}
	}

	#pragma omp parallel
	{
		#pragma omp single
		{
			No *no = lista;
			printf("Thread %d percorrendo a lista encadeada.\n", omp_get_thread_num());

			while (no != NULL) {
				No *no_atual = no;
                
                // o omp task por padrão faz o comportamento de firstprivate, mas é uma boa prática explicitar.
                // isso garante que cada tarefa tenha sua própria cópia da variável no_atual, evitando problemas de concorrência.
				#pragma omp task firstprivate(no_atual)
				printf("Arquivo: %s | Thread: %d\n",
				       no_atual->nome_arquivo,
				       omp_get_thread_num());

				no = no->proximo;
			}
            
            // há um taskwait a seguir... não necessário, apenas garantia. 
            // Se houvesse um nowait, o programa aguardaria a conclusão de todas as tarefas antes de prosseguir.
			#pragma omp taskwait 
		}
	}

	liberar_lista(lista);
	return 0;
}
