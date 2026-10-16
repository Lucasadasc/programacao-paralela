CC = gcc
CFLAGS = -O3 -std=c17 -fopenmp -ffp-contract=off -Wall -Wextra -Wpedantic
LDLIBS = -lm
.PHONY: all validar rapido clean
all: navierStokesV1 navierStokesV2 navierStokesV3
navierStokesV%: navierStokesV%.c navierStokes.c navierStokes.h
	$(CC) $(CFLAGS) -DVERSAO=$* navierStokes.c $< -o $@ $(LDLIBS)
validar: all
	./navierStokesV1 --validar
	./navierStokesV2 --validar
	./navierStokesV3 --validar
rapido: validar
	python3 executarExperimentos.py --tamanhos 64 128 --threads 1 2 4 --base-fraca 32 --passos 30 --repeticoes 3 --saida resultadosLocal
clean:
	rm -f navierStokes navierStokesV1 navierStokesV2 navierStokesV3 navierPascal1 navierPascal2 navierPascal3

# O env.sh do PaScal deve tornar pascalops.h e libmpascalops acessiveis.
# PASCAL_CPPFLAGS/PASCAL_LDFLAGS permitem indicar -I/-L explicitamente.
.PHONY: pascal
pascal: navierPascal1 navierPascal2 navierPascal3
navierPascal%: navierStokesV%.c navierStokes.c navierStokes.h
	$(CC) $(CFLAGS) $(PASCAL_CPPFLAGS) -DUSAR_PASCAL -DVERSAO=$* navierStokes.c $< -o $@ $(PASCAL_LDFLAGS) -lmpascalops $(LDLIBS)
