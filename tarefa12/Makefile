CC = gcc
CFLAGS = -O3 -std=c17 -fopenmp -ffp-contract=off -Wall -Wextra -Wpedantic
LDLIBS = -lm
.PHONY: all validar rapido clean
all: navierStokes
navierStokes: navierStokes.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)
validar: navierStokes
	./navierStokes --validar
rapido: validar
	python3 executarExperimentos.py --tamanhos 64 128 --threads 1 2 4 --base-fraca 32 --passos 30 --repeticoes 3 --saida resultadosLocal
clean:
	rm -f navierStokes navierPascal1 navierPascal2 navierPascal3

# O env.sh do PaScal deve tornar pascalops.h e libmpascalops acessiveis.
# PASCAL_CPPFLAGS/PASCAL_LDFLAGS permitem indicar -I/-L explicitamente.
.PHONY: pascal
pascal: navierPascal1 navierPascal2 navierPascal3
navierPascal%: navierStokes.c
	$(CC) $(CFLAGS) $(PASCAL_CPPFLAGS) -DUSAR_PASCAL -DPASCAL_VERSAO=$* $< -o $@ $(PASCAL_LDFLAGS) -lmpascalops $(LDLIBS)
