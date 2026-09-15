# Makefile para a Tarefa 11 - Exploracao de OpenMP Avancado
# Disciplina: Programacao Paralela - UFRN

CC = gcc
CFLAGS = -std=c17 -O3 -fopenmp -mavx2 -mfma -Wall -Wextra -pedantic
LDFLAGS = -lm

PROGRAMAS = 1_sections 2_schedule_static 3_schedule_dynamic 4_schedule_guided 5_collapse 6_simd benchmark_completo

.PHONY: all clean benchmark rodar_todos

all: $(PROGRAMAS)

1_sections: 1_sections.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

2_schedule_static: 2_schedule_static.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

3_schedule_dynamic: 3_schedule_dynamic.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

4_schedule_guided: 4_schedule_guided.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

5_collapse: 5_collapse.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

6_simd: 6_simd.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

benchmark_completo: benchmark_completo.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

benchmark: benchmark_completo
	./benchmark_completo 256 200 4

rodar_todos: $(PROGRAMAS)
	@echo "=== 1. TESTE DE SECTIONS ==="
	./1_sections 256 200 4
	@echo "\n=== 2. TESTE DE SCHEDULE STATIC ==="
	./2_schedule_static 256 200 4
	@echo "\n=== 3. TESTE DE SCHEDULE DYNAMIC ==="
	./3_schedule_dynamic 256 200 4
	@echo "\n=== 4. TESTE DE SCHEDULE GUIDED ==="
	./4_schedule_guided 256 200 4
	@echo "\n=== 5. TESTE DE COLLAPSE ==="
	./5_collapse 256 200 4
	@echo "\n=== 6. TESTE DE SIMD ==="
	./6_simd 256 200 4

clean:
	rm -f $(PROGRAMAS)
