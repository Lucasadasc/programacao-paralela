#!/usr/bin/env python3
"""
visualizar.py - Gerador de Gráficos para a Simulação de Navier-Stokes (Apenas Viscosidade)

Lê os dados gerados pela simulação em C ('difusao_simulacao.csv') e gera:
1. Comparação dos perfis de corte central (y = 0.5) ao longo do tempo.
2. Mapas de calor 2D (heatmap) do campo de velocidade no instante inicial, intermediário e final.
3. Gráfico de decaimento do pico máximo e da energia cinética total.

Uso:
    python3 visualizar.py [arquivo_csv] [arquivo_saida_png]
"""

import sys
import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

def gerar_graficos(csv_path="difusao_simulacao.csv", saida_png="evolucao_difusao.png"):
    if not os.path.exists(csv_path):
        print(f"Erro: Arquivo '{csv_path}' nao encontrado.")
        print("Execute primeiro a simulacao em C (cenario 3 ou 4) para gerar os dados:")
        print("  ./simulacao_viscosidade 3 64 300 4")
        return 1

    try:
        matplotlib.use("Agg")  # Backend sem interface gráfica (headless)
        
    except ImportError:
        print("Aviso: 'matplotlib' ou 'numpy' nao estao instalados.")
        print("Instale-os com: pip install matplotlib numpy")
        return 1

    print(f"Lendo dados de '{csv_path}'...")

    # Leitura manual ou via numpy/pandas
    dados_por_passo = {}
    passos_ordenados = []

    with open(csv_path, "r") as f:
        linhas = f.readlines()

    cabecalho = linhas[0].strip().split(",")
    idx_passo = cabecalho.index("passo")
    idx_tempo = cabecalho.index("tempo")
    idx_x = cabecalho.index("x")
    idx_y = cabecalho.index("y")
    idx_u = cabecalho.index("u")
    idx_v = cabecalho.index("v")
    idx_mag = cabecalho.index("magnitude")

    for linha in linhas[1:]:
        partes = linha.strip().split(",")
        if len(partes) < 7:
            continue
        p = int(partes[idx_passo])
        t = float(partes[idx_tempo])
        x = float(partes[idx_x])
        y = float(partes[idx_y])
        u = float(partes[idx_u])
        v = float(partes[idx_v])
        mag = float(partes[idx_mag])

        if p not in dados_por_passo:
            dados_por_passo[p] = {"tempo": t, "x": [], "y": [], "u": [], "mag": []}
            passos_ordenados.append(p)

        dados_por_passo[p]["x"].append(x)
        dados_por_passo[p]["y"].append(y)
        dados_por_passo[p]["u"].append(u)
        dados_por_passo[p]["mag"].append(mag)

    if not passos_ordenados:
        print("Erro: Nenhum dado valido encontrado no CSV.")
        return 1

    passo_final = max(passos_ordenados)
    print(f"Encontrados {len(passos_ordenados)} estados gravados: {passos_ordenados}")
    print(f"Quantidade total de passos executados: {passo_final}")

    fig = plt.figure(figsize=(16, 10))
    fig.suptitle("Simulação de Navier-Stokes - Difusão Viscosa Pura (∂u/∂t = ν ∇²u)\n"
                 f"Total de passos executados: {passo_final}",
                 fontsize=16, fontweight="bold", y=0.98)

    # 1. Gráfico de corte central (y próximo de 0.5)
    ax1 = fig.add_subplot(2, 3, 1)
    ax1.set_title("Perfil de Velocidade no Corte Central (y ≈ 0.5)", fontweight="bold")
    ax1.set_xlabel("Posição x (m)")
    ax1.set_ylabel("Velocidade u (m/s)")
    ax1.grid(True, linestyle="--", alpha=0.6)

    cores = plt.cm.viridis(np.linspace(0, 0.9, len(passos_ordenados)))

    for i, p in enumerate(passos_ordenados):
        d = dados_por_passo[p]
        xs = np.array(d["x"])
        ys = np.array(d["y"])
        us = np.array(d["u"])

        # Identificar o valor central de y
        y_meio = (np.min(ys) + np.max(ys)) / 2.0
        filtro_meio = np.abs(ys - y_meio) < 0.02
        if np.any(filtro_meio):
            x_corte = xs[filtro_meio]
            u_corte = us[filtro_meio]
            ordem = np.argsort(x_corte)
            ax1.plot(x_corte[ordem], u_corte[ordem],
                     label=f"passo {p} (t = {d['tempo']:.3f} s)",
                     color=cores[i], linewidth=2.0)

    ax1.legend(loc="upper right", fontsize=9)

    # 2. Gráfico de decaimento do pico máximo e energia cinética
    ax2 = fig.add_subplot(2, 3, 2)
    ax2.set_title("Decaimento do Pico e da Energia Cinética", fontweight="bold")
    ax2.set_xlabel("Tempo (s)")
    ax2.set_ylabel("Velocidade Máxima (m/s)", color="tab:red")

    tempos = [dados_por_passo[p]["tempo"] for p in passos_ordenados]
    picos = [np.max(dados_por_passo[p]["mag"]) for p in passos_ordenados]
    energias = [0.5 * np.mean(np.array(dados_por_passo[p]["mag"])**2) for p in passos_ordenados]

    l1 = ax2.plot(tempos, picos, "o-", color="tab:red", label="Pico Máximo (m/s)", linewidth=2.0)
    ax2.tick_params(axis="y", labelcolor="tab:red")
    ax2.grid(True, linestyle="--", alpha=0.6)

    ax2_twin = ax2.twinx()
    ax2_twin.set_ylabel("Energia Cinética Média (J/kg)", color="tab:blue")
    l2 = ax2_twin.plot(tempos, energias, "s--", color="tab:blue", label="Energia Cinética (J/kg)", linewidth=2.0)
    ax2_twin.tick_params(axis="y", labelcolor="tab:blue")

    linhas = l1 + l2
    rotulos = [l.get_label() for l in linhas]
    ax2.legend(linhas, rotulos, loc="center right", fontsize=9)

    # 3. Informações Teóricas e Físicas
    ax3 = fig.add_subplot(2, 3, 3)
    ax3.axis("off")
    texto_teoria = (
        "FORMULAÇÃO FÍSICA E NUMÉRICA\n"
        "───────────────────────────────────\n"
        "• Equação Governante:\n"
        "  ∂u/∂t = ν ∇²u\n"
        "  ∂v/∂t = ν ∇²v\n\n"
        "• Hipóteses Físicas:\n"
        "  - Efeitos de pressão desprezados (∇p = 0)\n"
        "  - Forças externas desprezadas (f = 0)\n"
        "  - Regime dominado por viscosidade pura\n\n"
        "• Discretização:\n"
        "  - Diferenças Centrais no espaço (2ª ordem)\n"
        "  - Euler Explícito no tempo (FTCS)\n\n"
        "• Critério de Estabilidade:\n"
        "  Δt ≤ h² / (4 ν)\n"
        "  Garante estabilidade estrita de Von Neumann.\n\n"
        f"• Passos executados: {passo_final}\n"
        f"  Estados gravados: {len(passos_ordenados)}"
    )
    ax3.text(
        0.05, 
        0.95,
        texto_teoria, 
        fontsize=10, 
        verticalalignment="top",
        family="monospace", 
        bbox=dict(boxstyle="round,pad=0.8", facecolor="#f4f6f8", edgecolor="#ccd1d9")
    )

    # Mapas de calor 2D para 3 instantes representativos
    instantes_idx = [0, len(passos_ordenados)//2, len(passos_ordenados)-1]
    titulos_mapa = [f"(a) Passo {passos_ordenados[instantes_idx[0]]} - T = 0.000s (Pulso Inicial)",
                    f"(b) Passo {passos_ordenados[instantes_idx[1]]} - T = {dados_por_passo[passos_ordenados[instantes_idx[1]]]['tempo']:.3f}s (Difundindo)",
                    f"(c) Passo {passos_ordenados[instantes_idx[2]]} - T = {dados_por_passo[passos_ordenados[instantes_idx[2]]]['tempo']:.3f}s (Dissipado)"]

    vmax_global = np.max(dados_por_passo[passos_ordenados[0]]["mag"])

    for idx_subplot, idx_p in enumerate(instantes_idx):
        p = passos_ordenados[idx_p]
        d = dados_por_passo[p]
        xs = np.array(d["x"])
        ys = np.array(d["y"])
        mags = np.array(d["mag"])

        # Reconstruir grade 2D
        x_unicos = np.unique(xs)
        y_unicos = np.unique(ys)
        nx_u = len(x_unicos)
        ny_u = len(y_unicos)

        grade_2d = mags.reshape((nx_u, ny_u)).T

        ax = fig.add_subplot(2, 3, 4 + idx_subplot)
        ax.set_title(titulos_mapa[idx_subplot], fontweight="bold")
        im = ax.imshow(grade_2d, origin="lower", extent=[x_unicos[0], x_unicos[-1], y_unicos[0], y_unicos[-1]],
                       cmap="plasma", vmin=0.0, vmax=vmax_global, interpolation="bicubic")
        ax.set_xlabel("x (m)")
        ax.set_ylabel("y (m)")
        cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
        cbar.set_label("|u| (m/s)")

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig(saida_png, dpi=200)
    print(f"Gráficos salvos com sucesso em: '{saida_png}'")
    return 0

if __name__ == "__main__":
    csv = sys.argv[1] if len(sys.argv) > 1 else "difusao_simulacao.csv"
    png = sys.argv[2] if len(sys.argv) > 2 else "evolucao_difusao.png"
    sys.exit(gerar_graficos(csv, png))
