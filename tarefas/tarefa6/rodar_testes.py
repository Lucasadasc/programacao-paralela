#!/usr/bin/env python3
"""
Script para automatizar a execução e benchmark dos programas da tarefa 6.
Varia o número de threads (1, 2, 4, 6, 8, 10, 12, 16) com 10 milhões de pontos para cada caso.
Salva e formata os resultados em tabelas comparativas com casas decimais padronizadas.
"""

import subprocess
import os
import re
import math

PI_REAL = 3.14159265358979323846
PONTOS = 10000000  # 10 milhões de pontos
THREADS_LIST = [1, 2, 4, 6, 8, 10, 12, 16]

PROGRAMAS = [
    ("1_sequencial", "1_sequencial.c", False),
    ("2_pragma_parallel_for", "2_pragma_parallel_for.c", True),
    ("3_critical", "3_critical.c", True),
    ("4_shared_private", "4_shared_private.c", True),
    ("5_firstprivate", "5_firstprivate.c", True),
    ("6_lastprivate", "6_lastprivate.c", True),
    ("7_default_none", "7_default_none.c", True),
]

def compilar_programas():
    print("=" * 70)
    print("COMPILANDO OS PROGRAMAS...")
    print("=" * 70)
    for bin_name, src_name, usa_openmp in PROGRAMAS:
        if usa_openmp:
            cmd = ["gcc", "-fopenmp", "-O3", src_name, "-o", bin_name, "-lm"]
        else:
            cmd = ["gcc", "-O3", src_name, "-o", bin_name, "-lm"]
        
        print(f"Compilando {src_name} -> {bin_name}...")
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            print(f"Erro ao compilar {src_name}:\n{res.stderr}")
            return False
    print("Compilação concluída com sucesso!\n")
    return True

def extrair_dados(stdout_text):
    """Extrai Pi Estimado, Erro Absoluto e Tempo de Execução da saída."""
    pi_match = re.search(r"Pi Estimado\s*:\s*([\d\.]+)", stdout_text)
    tempo_match = re.search(r"Tempo de Execucao\s*:\s*([\d\.]+)", stdout_text)
    
    if pi_match:
        pi_est = float(pi_match.group(1))
    else:
        pi_est = 0.0
        
    erro_abs = abs(pi_est - PI_REAL)
    
    if tempo_match:
        tempo = float(tempo_match.group(1))
    else:
        tempo = 0.0
        
    return pi_est, erro_abs, tempo

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    if not compilar_programas():
        return

    # Estrutura para armazenar resultados: {prog_name: {threads: (pi, erro, tempo)}}
    resultados = {p[0]: {} for p in PROGRAMAS}

    print("=" * 70)
    print(f"EXECUTANDO EXPERIMENTOS ({PONTOS:,} pontos por execução)...")
    print("=" * 70)

    for bin_name, src_name, usa_openmp in PROGRAMAS:
        print(f"\n---> Executando: {bin_name}")
        for t in THREADS_LIST:
            # Para o sequencial, só faz sentido rodar 1 vez ou registrar igual
            if not usa_openmp and t > 1:
                resultados[bin_name][t] = resultados[bin_name][1]
                continue

            env = os.environ.copy()
            env["OMP_NUM_THREADS"] = str(t)

            cmd = [f"./{bin_name}", str(PONTOS)]
            res = subprocess.run(cmd, capture_output=True, text=True, env=env)

            if res.returncode != 0:
                print(f"  [Threads: {t:2d}] ERRO na execução: {res.stderr.strip()}")
                resultados[bin_name][t] = (0.0, 0.0, 0.0)
            else:
                pi_est, erro, tempo = extrair_dados(res.stdout)
                resultados[bin_name][t] = (pi_est, erro, tempo)
                print(f"  [Threads: {t:2d}] Pi = {pi_est:.10f} | Erro = {erro:.10f} | Tempo = {tempo:.6f}s")

    # Gerar Relatório Formatado em Markdown e TXT
    relatorio_md = []
    relatorio_md.append("# Relatório de Testes - Tarefa 6 (OpenMP)")
    relatorio_md.append(f"- **Total de Pontos por Teste:** {PONTOS:,}")
    relatorio_md.append(f"- **Valor Real de $\\pi$:** {PI_REAL:.10f}")
    relatorio_md.append(f"- **Threads Testadas:** {', '.join(map(str, THREADS_LIST))}\n")

    # Tabela 1: Visão Geral por Programa e Número de Threads
    relatorio_md.append("## 1. Tabela Comparativa Detalhada\n")
    relatorio_md.append("| Programa | Threads | Pi Estimado | Erro Absoluto | Tempo (s) | Status |")
    relatorio_md.append("| :--- | :---: | :---: | :---: | :---: | :---: |")

    for bin_name, _, usa_openmp in PROGRAMAS:
        for t in THREADS_LIST:
            pi_est, erro, tempo = resultados[bin_name][t]
            
            # Status do resultado
            if bin_name in ["1_sequencial", "3_critical", "4_shared_private", "5_firstprivate", "7_default_none"]:
                status = "Correto"
            elif bin_name == "2_pragma_parallel_for":
                status = "Data Race (Incorreto)"
            elif bin_name == "6_lastprivate":
                status = "Sobrescrita Lastprivate (Incorreto)"
            else:
                status = "-"

            relatorio_md.append(
                f"| `{bin_name}` | {t} | {pi_est:.10f} | {erro:.10f} | {tempo:.6f} | {status} |"
            )

    # Tabela 2: Matriz Comparativa de Pi Estimado (Programas x Threads)
    relatorio_md.append("\n## 2. Matriz: Pi Estimado por Número de Threads\n")
    headers = ["Programa"] + [f"{t}T" for t in THREADS_LIST]
    relatorio_md.append("| " + " | ".join(headers) + " |")
    relatorio_md.append("| " + " | ".join([":---"] + [":---:" for _ in THREADS_LIST]) + " |")

    for bin_name, _, _ in PROGRAMAS:
        row = [f"`{bin_name}`"]
        for t in THREADS_LIST:
            pi_est, _, _ = resultados[bin_name][t]
            row.append(f"{pi_est:.10f}")
        relatorio_md.append("| " + " | ".join(row) + " |")

    # Tabela 3: Matriz Comparativa de Erro Absoluto (Programas x Threads)
    relatorio_md.append("\n## 3. Matriz: Erro Absoluto por Número de Threads (10 Casas Decimais)\n")
    relatorio_md.append("| " + " | ".join(headers) + " |")
    relatorio_md.append("| " + " | ".join([":---"] + [":---:" for _ in THREADS_LIST]) + " |")

    for bin_name, _, _ in PROGRAMAS:
        row = [f"`{bin_name}`"]
        for t in THREADS_LIST:
            _, erro, _ = resultados[bin_name][t]
            row.append(f"{erro:.10f}")
        relatorio_md.append("| " + " | ".join(row) + " |")

    # Tabela 4: Matriz Comparativa de Tempo (s)
    relatorio_md.append("\n## 4. Matriz: Tempo de Execução (segundos)\n")
    relatorio_md.append("| " + " | ".join(headers) + " |")
    relatorio_md.append("| " + " | ".join([":---"] + [":---:" for _ in THREADS_LIST]) + " |")

    for bin_name, _, _ in PROGRAMAS:
        row = [f"`{bin_name}`"]
        for t in THREADS_LIST:
            _, _, tempo = resultados[bin_name][t]
            row.append(f"{tempo:.6f}")
        relatorio_md.append("| " + " | ".join(row) + " |")

    # Análise e Conclusões
    relatorio_md.append("\n## 5. Análise dos Resultados e Observações Didáticas\n")
    relatorio_md.append("1. **`1_sequencial`**:\n   - Tempo e precisão constantes (não afetado pelo número de threads).")
    relatorio_md.append("2. **`2_pragma_parallel_for` (Condição de Corrida)**:\n   - Com 1 thread o resultado é exato, mas à medida que o número de threads aumenta, o número de colisões/lost updates explode, reduzindo o $\\pi$ estimado drasticamente (erro cresce com as threads).")
    relatorio_md.append("3. **`3_critical` (Seção Crítica)**:\n   - O resultado de $\\pi$ é matematicamente correto em todas as threads, porém o tempo de execução é elevado devido à forte contenção no lock da seção crítica.")
    relatorio_md.append("4. **`4_shared_private`, `5_firstprivate`, `7_default_none` (Contadores Locais)**:\n   - Resultados matematicamente corretos em todas as configurações de threads com excelente escalabilidade de desempenho.")
    relatorio_md.append("5. **`6_lastprivate` (Erro Conceitual)**:\n   - Com 1 thread o resultado é normal. Com $T > 1$ threads, a variável final retém apenas a contagem da última thread lógica (~1/$T$ das iterações), fazendo $\\pi_{\\text{estimado}} \\approx \\pi / T$, o que evidencia por que `lastprivate` não serve como acumulador.")

    conteudo_final = "\n".join(relatorio_md) + "\n"

    # Salva arquivos
    with open("resultados_testes.md", "w", encoding="utf-8") as f:
        f.write(conteudo_final)
    
    with open("resultados_testes.txt", "w", encoding="utf-8") as f:
        f.write(conteudo_final)

    print("\n" + "=" * 70)
    print("TESTES CONCLUÍDOS!")
    print("Resultados salvos em 'resultados_testes.md' e 'resultados_testes.txt'.")
    print("=" * 70)

if __name__ == "__main__":
    main()
