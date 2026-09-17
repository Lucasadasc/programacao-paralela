# Makefile para a Tarefa 11 - Simulacao de Navier-Stokes (Apenas Viscosidade)
CC = gcc
CFLAGS = -std=c17 -O3 -fopenmp -Wall -Wextra -pedantic
LDFLAGS = -lm

TARGET = simulacao_viscosidade
SRC = simulacao_viscosidade.c

.PHONY: all clean run balanceamento cenario1 cenario2 cenario3 cenario4

N ?= 256
PASSOS ?= 200

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) 4 64 300 4

balanceamento: $(TARGET)
	./$(TARGET) 5 $(N) $(PASSOS) 1

cenario1: $(TARGET)
	./$(TARGET) 1 64 300 4

cenario2: $(TARGET)
	./$(TARGET) 2 64 300 4

cenario3: $(TARGET)
	./$(TARGET) 3 64 300 4

cenario4: $(TARGET)
	./$(TARGET) 4 64 300 4

clean:
	rm -f $(TARGET) difusao_simulacao.csv
