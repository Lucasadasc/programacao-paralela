# Makefile para a Tarefa 11 - Exploracao de OpenMP Avancado
# Disciplina: Programacao Paralela - UFRN

CC = gcc
CFLAGS = -std=c17 -O3 -fopenmp -mavx2 -mfma -Wall -Wextra -pedantic
LDFLAGS = -lm

PROGRAMAS = 1_static 2_static_collapse 3_dynamic 4_dynamic_collapse 5_guided 6_guided_collapse

.PHONY: all clean balanceamento rodar_todos

all: $(PROGRAMAS)

1_static: 1_static.c navier_stokes/fluido.c navier_stokes/fluido.h
	$(CC) $(CFLAGS) 1_static.c navier_stokes/fluido.c -o $@ $(LDFLAGS)

2_static_collapse: 2_static_collapse.c navier_stokes/fluido.c navier_stokes/fluido.h
	$(CC) $(CFLAGS) 2_static_collapse.c navier_stokes/fluido.c -o $@ $(LDFLAGS)

3_dynamic: 2_dynamic.c navier_stokes/fluido.c navier_stokes/fluido.h
	$(CC) $(CFLAGS) 2_dynamic.c navier_stokes/fluido.c -o $@ $(LDFLAGS)

4_dynamic_collapse: 4_dynamic_collapse.c navier_stokes/fluido.c navier_stokes/fluido.h
	$(CC) $(CFLAGS) 4_dynamic_collapse.c navier_stokes/fluido.c -o $@ $(LDFLAGS)

5_guided: 3_guided.c navier_stokes/fluido.c navier_stokes/fluido.h
	$(CC) $(CFLAGS) 3_guided.c navier_stokes/fluido.c -o $@ $(LDFLAGS)

6_guided_collapse: 6_guided_collapse.c navier_stokes/fluido.c navier_stokes/fluido.h
	$(CC) $(CFLAGS) 6_guided_collapse.c navier_stokes/fluido.c -o $@ $(LDFLAGS)

balanceamento:
	$(MAKE) -C navier_stokes balanceamento N=256 PASSOS=200

rodar_todos: $(PROGRAMAS)
	@echo "=== 1. TESTE DE STATIC ==="
	./1_static 256 200 4
	@echo "\n=== 2. TESTE DE STATIC + COLLAPSE ==="
	./2_static_collapse 256 200 4
	@echo "\n=== 3. TESTE DE DYNAMIC ==="
	./3_dynamic 256 200 4
	@echo "\n=== 4. TESTE DE DYNAMIC + COLLAPSE ==="
	./4_dynamic_collapse 256 200 4
	@echo "\n=== 5. TESTE DE GUIDED ==="
	./5_guided 256 200 4
	@echo "\n=== 6. TESTE DE GUIDED + COLLAPSE ==="
	./6_guided_collapse 256 200 4

clean:
	rm -f $(PROGRAMAS)
