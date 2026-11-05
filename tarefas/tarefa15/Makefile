MPICC ?= mpicc
CFLAGS ?= -O3 -std=c11 -Wall -Wextra -Wpedantic
PROGRAMS = calorBloqueante calorNaoBloqueante calorComSobreposicao

all: $(PROGRAMS)

calorBloqueante: calorBloqueante.c calorComum.h
	$(MPICC) $(CFLAGS) -o $@ calorBloqueante.c
calorNaoBloqueante: calorNaoBloqueante.c calorComum.h
	$(MPICC) $(CFLAGS) -o $@ calorNaoBloqueante.c
calorComSobreposicao: calorComSobreposicao.c calorComum.h
	$(MPICC) $(CFLAGS) -o $@ calorComSobreposicao.c


clean:
	rm -f $(PROGRAMS)
.PHONY: all clean
