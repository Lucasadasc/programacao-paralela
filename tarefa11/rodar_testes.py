#!/usr/bin/env python3
"""
rodar_testes.py - Automação de Benchmarks e Geração de Tabelas Markdown
Disciplina: Programação Paralela - UFRN
Tarefa 11: Navier-Stokes (Apenas Viscosidade) - Estudo de OpenMP
"""

import subprocess
import os
import sys

THREAD_COUNTS = [1, 2, 4, 8, 16]
GRID_SIZE = 256
TIME_STEPS = 200

def compilar():
    print("[1/3] Compilando programas com make...")
    ret = subprocess.run(["make", "all"], cwd=".")
    if ret.returncode != 0:
        print("Erro na compilacao.")
        sys.exit(1)
    print("Compilacao concluida com sucesso.\n")

def executar_benchmark():
    print(f"[2/3] Executando benchmark completo para threads {THREAD_COUNTS}...")
    resultados = {}

    for th in THREAD_COUNTS:
        print(f"  -> Testando com {th} thread(s)...")
        cmd = ["./benchmark_completo", str(GRID_SIZE), str(TIME_STEPS), str(th)]
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if proc.returncode != 0:
            print(f"Erro ao rodar benchmark para {th} threads: {proc.stderr}")
            continue
        resultados[th] = proc.stdout

    return resultados

def salvar_relatorio(resultados, output_path="relatorio_desempenho.md"):
    print(f"[3/3] Gerando relatorio consolidado em '{output_path}'...")
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("# Relatório de Desempenho - Tarefa 11 (OpenMP Avançado)\n\n")
        f.write(f"- **Malha:** {GRID_SIZE}x{GRID_SIZE} ({GRID_SIZE * GRID_SIZE} pontos)\n")
        f.write(f"- **Passos de Tempo:** {TIME_STEPS}\n")
        f.write("- **Configurações Testadas:** `#pragma omp sections`, `schedule(static)`, `schedule(dynamic)`, `schedule(guided)`, `collapse(2)`, `#pragma omp simd`.\n\n")

        for th in THREAD_COUNTS:
            if th in resultados:
                f.write(f"## Execução com {th} Thread(s)\n\n")
                # Extrai apenas a tabela do output
                linhas = resultados[th].split("\n")
                tabela = [l for l in linhas if l.startswith("|")]
                f.write("\n".join(tabela) + "\n\n")

    print(f"Relatório gerado com sucesso em: {output_path}")

if __name__ == "__main__":
    compilar()
    res = executar_benchmark()
    salvar_relatorio(res)
