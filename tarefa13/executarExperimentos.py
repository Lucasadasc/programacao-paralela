#!/usr/bin/env python3
"""Executa processos independentes por afinidade e preserva medidas brutas."""
import argparse
import csv
import json
import math
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess


def positivo(texto):
    valor = int(texto)
    if valor < 1:
        raise argparse.ArgumentTypeError('use um inteiro positivo')
    return valor


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--threads', nargs='+', type=positivo, default=[1, 4, 8, 16, 32])
    p.add_argument('--lados', nargs='+', type=positivo, default=[256, 512, 1024, 2048])
    p.add_argument('--base-fraca', type=positivo, default=65536,
                   help='pontos internos por thread na sequencia fraca')
    p.add_argument('--politicas', nargs='+', choices=['false', 'true', 'close', 'spread', 'master'],
                   default=['false', 'true', 'close', 'spread', 'master'])
    p.add_argument('--passos', type=positivo, default=300)
    p.add_argument('--repeticoes', type=positivo, default=5)
    p.add_argument('--saida', type=Path, required=True)
    a = p.parse_args()
    if 1 not in a.threads:
        p.error('--threads deve incluir 1 para calcular speedup')
    if any(n < 6 or n > 16382 for n in a.lados):
        p.error('--lados deve estar entre 6 e 16382 (pontos internos)')
    raiz = Path(__file__).resolve().parent
    fraca = {th: max(6, math.isqrt(a.base_fraca * th) +
                    (math.isqrt(a.base_fraca * th)**2 < a.base_fraca * th)) for th in a.threads}
    if max(fraca.values()) > 16382:
        p.error('malha fraca excede o limite do solver')
    # Mesma entrada com uma thread para a referencia de cada speedup.
    pares = {(th, n) for th in a.threads for n in a.lados}
    pares |= {(th, n) for th, n in fraca.items()} | {(1, n) for n in fraca.values()}
    a.saida.mkdir(parents=True, exist_ok=False)
    metadados = dict(vars(a), saida=str(a.saida), hostname=platform.node(),
                     cpus_permitidas=sorted(os.sched_getaffinity(0)),
                     ambiente={k: v for k, v in os.environ.items() if k.startswith(('OMP_', 'SLURM_'))},
                     fraca=fraca)
    (a.saida / 'configuracao.json').write_text(json.dumps(metadados, indent=2))
    envbase = os.environ.copy()
    # Evita clausulas herdadas que substituam o numero solicitado de threads.
    for chave in ('OMP_THREAD_LIMIT', 'GOMP_CPU_AFFINITY', 'KMP_AFFINITY',
                  'OMP_DISPLAY_ENV', 'OMP_DISPLAY_AFFINITY'):
        envbase.pop(chave, None)
    envbase.update(OMP_DYNAMIC='FALSE', OMP_PLACES='cores')
    for politica in a.politicas:
        for th in sorted(set(a.threads)):
            env = dict(envbase, OMP_PROC_BIND=politica, OMP_NUM_THREADS=str(th))
            # Validar cada politica; diagnostico de posicionamento fora das medidas.
            if th == 1:
                subprocess.run([str(raiz / 'navierStokes'), '--validar'], env=env, check=True)
            r = subprocess.run([str(raiz / 'afinidade')], env=env, check=True,
                               capture_output=True, text=True)
            (a.saida / f'afinidade-{politica}-{th}.txt').write_text(r.stdout + r.stderr)
    tarefas = [(pol, th, n, rep) for pol in a.politicas for th, n in sorted(pares)
               for rep in range(1, a.repeticoes + 1)]
    random.Random(13).shuffle(tarefas)
    campos = ['politica', 'threads', 'lado_interno', 'pontos', 'passos', 'repeticao',
              'evolucao_s', 'inicializacao_s', 'medido_s', 'soma', 'pico']
    grupos = {}
    referencias = {}
    with (a.saida / 'medidas.csv').open('w', newline='') as f:
        w = csv.DictWriter(f, fieldnames=campos)
        w.writeheader()
        for indice, (pol, th, n, rep) in enumerate(tarefas, 1):
            env = dict(envbase, OMP_PROC_BIND=pol, OMP_NUM_THREADS=str(th))
            r = subprocess.run([str(raiz / 'navierStokes'), str(n + 2), str(a.passos), str(th)],
                               env=env, check=True, capture_output=True, text=True)
            col = next(csv.reader([r.stdout.strip()]))
            if len(col) != 9 or int(col[3]) != th:
                raise RuntimeError(f'saida inesperada: {r.stdout} {r.stderr}')
            evolucao, soma, pico, inicializacao, medido = map(float, col[4:])
            if not all(math.isfinite(x) for x in (evolucao, soma, pico, inicializacao, medido)):
                raise RuntimeError('resultado nao finito')
            ref = referencias.setdefault(n, (soma, pico))
            if not (math.isclose(soma, ref[0], rel_tol=1e-12, abs_tol=1e-12)
                    and math.isclose(pico, ref[1], rel_tol=1e-12, abs_tol=1e-12)):
                raise RuntimeError('resultado difere entre politicas/threads')
            w.writerow(dict(zip(campos, [pol, th, n, n*n, a.passos, rep,
                                        evolucao, inicializacao, medido, soma, pico])))
            f.flush()
            grupos.setdefault((pol, th, n), []).append((evolucao, inicializacao, medido))
            if indice % 25 == 0 or indice == len(tarefas):
                print(f'{indice}/{len(tarefas)} execucoes', flush=True)
    medias = {k: [statistics.mean(col) for col in zip(*v)] for k, v in grupos.items()}
    with (a.saida / 'resumo.csv').open('w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['politica', 'threads', 'lado_interno', 'pontos', 'pontos_por_thread',
                    'sequencia_fraca', 'regiao', 'media_s', 'desvio_s', 'speedup', 'eficiencia',
                    'eficiencia_fraca'])
        for (pol, th, n), valores in sorted(grupos.items()):
            for regiao, nome in enumerate(['evolucao', 'inicializacao', 'medido']):
                media = medias[pol, th, n][regiao]
                speedup = medias[pol, 1, n][regiao] / media
                fr = n == fraca[th]
                ef = medias[pol, 1, fraca[1]][regiao] / media if fr else ''
                desvio = statistics.stdev(v[regiao] for v in valores) if len(valores) > 1 else 0
                w.writerow([pol, th, n, n*n, n*n/th, fr, nome, media, desvio,
                            speedup, speedup/th, ef])
    print(f'Resultados: {a.saida}')


if __name__ == '__main__':
    main()
