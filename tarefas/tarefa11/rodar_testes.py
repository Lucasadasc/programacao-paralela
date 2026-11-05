#!/usr/bin/env python3
"""
rodar_testes.py - Automação de Benchmarks e Geração de Tabelas Markdown
Disciplina: Programação Paralela - UFRN
Tarefa 11: Navier-Stokes (Apenas Viscosidade) - Estudo de OpenMP
"""

import subprocess
import re
import statistics
import sys

THREADS = 8
ITERATIONS = 10
CHUNKS = [16]
GRID_SIZE = 512
TIME_STEPS = 20000
PROGRAMAS = [
    ("1_static", "Versao 1 - static"),
    ("2_static_collapse", "Versao 2 - static + collapse(2)"),
    ("3_dynamic", "Versao 3 - dynamic"),
    ("4_dynamic_collapse", "Versao 4 - dynamic + collapse(2)"),
    ("5_guided", "Versao 5 - guided"),
    ("6_guided_collapse", "Versao 6 - guided + collapse(2)"),
]

def compilar():
    print("[1/3] Compilando programas com make...")
    ret = subprocess.run(["make", "all"], cwd=".")
    if ret.returncode != 0:
        print("Erro na compilacao.")
        sys.exit(1)
    print("Compilacao concluida com sucesso.\n")

def executar_benchmark():
    print("[2/3] Executando os programas da Tarefa 11...")
    resultados = {nome: {} for nome, _ in PROGRAMAS}

    for indice, (nome_programa, _) in enumerate(PROGRAMAS):
        for chunk in CHUNKS:
            tempos = []
            for _ in range(ITERATIONS):
                proc = subprocess.run(
                    [f"./{nome_programa}", str(GRID_SIZE), str(TIME_STEPS),
                     str(THREADS), str(chunk)],
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
                )
                if proc.returncode != 0:
                    print(f"Erro ao rodar {nome_programa}, chunk {chunk}: {proc.stderr}")
                    sys.exit(1)

                correspondencia = re.search(r"Tempo de execucao: ([0-9.]+) s", proc.stdout)
                if correspondencia is None:
                    print(f"Nao foi possivel ler o tempo de {nome_programa}, chunk {chunk}.")
                    sys.exit(1)
                tempos.append(float(correspondencia.group(1)))
            resultados[nome_programa][chunk] = statistics.median(tempos)

        if indice < len(PROGRAMAS) - 1:
            subprocess.run(["./pausar_entre_versoes.sh"], check=True)

    return resultados

def salvar_relatorio(resultados, output_path="resultados_chunks.md"):
    print(f"[3/3] Gerando tabela de resultados em '{output_path}'...")
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("# Resultados de chunks - Tarefa 11\n\n")
        f.write(f"- **Malha:** {GRID_SIZE}x{GRID_SIZE}\n")
        f.write(f"- **Passos:** {TIME_STEPS}\n")
        f.write(f"- **Threads fixas:** {THREADS}\n")
        f.write(f"- **Iteracoes por situacao:** {ITERATIONS}\n")
        f.write("- **Chunk fixo:** 16\n\n")
        f.write("Mediana do tempo de execucao em segundos:\n\n")
        f.write("| Chunk | " + " | ".join(descricao for _, descricao in PROGRAMAS) + " |\n")
        f.write("|---:|" + "---:" * len(PROGRAMAS) + "|\n")
        for chunk in CHUNKS:
            f.write("| " + str(chunk) + " | " + " | ".join(
                f"{resultados[nome_programa][chunk]:.6f}".replace(".", ",")
                for nome_programa, _ in PROGRAMAS
            ) + " |\n")

    print(f"Tabela gerada com sucesso em: {output_path}")

if __name__ == "__main__":
    compilar()
    res = executar_benchmark()
    salvar_relatorio(res)
