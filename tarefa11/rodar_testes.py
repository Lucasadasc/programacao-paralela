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

def executar_balanceamento():
    print("Executando benchmark de balanceamento com a simulacao de Navier-Stokes...")
    proc = subprocess.run(
        ["make", "-C", "navier_stokes", "balanceamento",
         f"N={GRID_SIZE}", f"PASSOS={TIME_STEPS}"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    if proc.returncode != 0:
        print(f"Erro no benchmark de balanceamento: {proc.stderr}")
        return ""
    return proc.stdout

def executar_benchmark():
    print("[2/3] Executando os programas da Tarefa 11...")
    resultados = {}

    for nome_programa in ["1_static", "2_static_collapse", "3_dynamic",
                          "4_dynamic_collapse", "5_guided", "6_guided_collapse"]:
        proc = subprocess.run(
            [f"./{nome_programa}", str(GRID_SIZE), str(TIME_STEPS), "4"],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        if proc.returncode != 0:
            print(f"Erro ao rodar {nome_programa}: {proc.stderr}")
            continue
        resultados[nome_programa] = proc.stdout

    return resultados

def salvar_relatorio(resultados, balanceamento, output_path="relatorio_desempenho.md"):
    print(f"[3/3] Gerando relatorio consolidado em '{output_path}'...")
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("# Relatório de Desempenho - Tarefa 11 (OpenMP Avançado)\n\n")
        f.write(f"- **Malha:** {GRID_SIZE}x{GRID_SIZE} ({GRID_SIZE * GRID_SIZE} pontos)\n")
        f.write(f"- **Passos de Tempo:** {TIME_STEPS}\n")
        f.write("- **Configurações Testadas:** `schedule(static)`, `schedule(dynamic)` e `schedule(guided)`, com e sem `collapse(2)`.\n\n")

        f.write("## Balanceamento da Simulacao de Navier-Stokes\n\n")
        f.write("Benchmark executado diretamente de `navier_stokes/simulacao_viscosidade.c`:\n\n")
        f.write("```text\n")
        f.write(balanceamento)
        f.write("```\n\n")

        for nome_programa, saida in resultados.items():
            f.write(f"## Execução de `{nome_programa}`\n\n")
            f.write("```text\n")
            f.write(saida)
            f.write("```\n\n")

    print(f"Relatório gerado com sucesso em: {output_path}")

if __name__ == "__main__":
    compilar()
    res = executar_benchmark()
    balanceamento = executar_balanceamento()
    salvar_relatorio(res, balanceamento)
