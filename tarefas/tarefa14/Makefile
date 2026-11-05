MPICC ?= mpicc
CFLAGS ?= -O3 -std=c17 -Wall -Wextra -Wpedantic

.PHONY: all clean executar

all: ping_pong

ping_pong: ping_pong.c
	$(MPICC) $(CFLAGS) $< -o $@

# Use MPIEXEC, se necessario, para adaptar ao launcher local (mpirun, srun etc.).
executar: ping_pong
	$${MPIEXEC:-mpirun} -np 2 ./ping_pong

clean:
	rm -f ping_pong resultados.csv
