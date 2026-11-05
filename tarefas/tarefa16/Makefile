MPICC ?= mpicc
CFLAGS ?= -O3 -std=c11 -Wall -Wextra -Wpedantic

all: matrizVetor

matrizVetor: matrizVetor.c
	$(MPICC) $(CFLAGS) -o $@ $<

clean:
	rm -f matrizVetor

.PHONY: all clean
