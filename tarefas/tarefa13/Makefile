CC = gcc
CFLAGS = -O3 -std=c17 -fopenmp -ffp-contract=off -Wall -Wextra -Wpedantic
LDLIBS = -lm
.PHONY: all validar rapido
all: navierStokes afinidade
navierStokes: navierStokes.c navierStokesV3.c navierStokes.h
	$(CC) $(CFLAGS) -DVERSAO=3 navierStokes.c navierStokesV3.c -o $@ $(LDLIBS)
afinidade: afinidade.c
	$(CC) $(CFLAGS) $< -o $@
.PHONY: pascal
pascal: navierPascal
navierPascal: navierStokes.c navierStokesV3.c navierStokes.h
	$(CC) $(CFLAGS) $(PASCAL_CPPFLAGS) -DUSAR_PASCAL -DVERSAO=3 navierStokes.c navierStokesV3.c -o $@ $(PASCAL_LDFLAGS) -lmpascalops $(LDLIBS)
validar: all
	./navierStokes --validar
rapido: validar
	python3 executarExperimentos.py --threads 1 2 --lados 32 64 --base-fraca 1024 --passos 10 --repeticoes 2 --saida /tmp/tarefa13-rapido
