
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
        raise argparse.ArgumentTypeError('informe inteiro positivo')
    return valor


def analisar(pasta):
    with (pasta / 'repeticoes.csv').open() as arquivo:
        linhas = list(csv.DictReader(arquivo))
    grupos = {}
    for linha in linhas:
        chave = (linha['modo'], int(linha['versao']), int(linha['n']),
                 int(linha['passos']), int(linha['threads']))
        grupos.setdefault(chave, []).append(float(linha['tempo_s']))
    medianas = {chave: statistics.median(valores) for chave, valores in grupos.items()}
    resumo = []
    for chave, tempos in sorted(grupos.items()):
        modo, ver, n, passos, p = chave
        mediana = medianas[chave]
        if modo == 'forte':
            referencia = medianas[(modo, ver, n, passos, 1)]
            speedup = referencia / mediana
            eficiencia = speedup / p
        else:
            base = next(k for k in medianas if k[0] == modo and k[1] == ver and k[3] == passos and k[4] == 1)
            referencia = medianas[base]
            razao_trabalho = ((n-2)/(base[2]-2))**2
            speedup = razao_trabalho * referencia / mediana
            eficiencia = speedup / p
        resumo.append([modo, ver, n, passos, p, len(tempos), mediana,
                       min(tempos), max(tempos), speedup, eficiencia,
                       (n-2)**2 * passos / mediana / 1e6])
    with (pasta / 'resumo.csv').open('w', newline='') as arquivo:
        escritor = csv.writer(arquivo)
        escritor.writerow(['modo', 'versao', 'n', 'passos', 'threads', 'repeticoes',
                           'mediana_s', 'min_s', 'max_s', 'speedup', 'eficiencia', 'milhoes_pontos_s'])
        escritor.writerows(resumo)
    with (pasta / 'tabelas.md').open('w') as arquivo:
        arquivo.write('# Medições de escalabilidade\n\nEficiência em fração; valores acima de 1 podem ocorrer por efeitos de cache ou ruído.\n\n')
        arquivo.write('| Modo | Versão | N | Threads | Mediana (s) | Speedup | Eficiência |\n|---|---:|---:|---:|---:|---:|---:|\n')
        for modo, ver, n, _, p, _, med, _, _, speedup, eficiencia, _ in resumo:
            arquivo.write(f'| {modo} | {ver} | {n} | {p} | {med:.6f} | {speedup:.3f} | {eficiencia:.3f} |\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tamanhos', nargs='+', type=positivo, default=[256, 512, 1024, 2048])
    parser.add_argument('--threads', nargs='+', type=positivo, default=[1, 2, 4, 8, 16, 32])
    parser.add_argument('--base-fraca', type=positivo, default=256, help='lado interno da malha para uma thread')
    parser.add_argument('--passos', type=positivo, default=300)
    parser.add_argument('--repeticoes', type=positivo, default=5)
    parser.add_argument('--saida', type=Path, required=True)
    parser.add_argument('--analisar', action='store_true', help='reanalisar CSV existente sem executar solver')
    args = parser.parse_args()
    if args.analisar:
        analisar(args.saida)
        return
    threads = sorted(set(args.threads))
    if threads[0] != 1:
        parser.error('--threads deve incluir 1 para referencia')
    if min(args.tamanhos) < 8:
        parser.error('N deve ser pelo menos 8')
    limite = int(os.environ.get('SLURM_CPUS_PER_TASK', len(os.sched_getaffinity(0))))
    if max(threads) > limite:
        parser.error(f'numero de threads excede CPUs disponiveis ({limite})')
    tarefas = [('forte', v, n, p) for v in range(1, 4) for n in sorted(set(args.tamanhos)) for p in threads]
    tarefas += [('fraca', v, round(args.base_fraca * math.sqrt(p))+2, p) for v in range(1, 4) for p in threads]
    if max(t[2] for t in tarefas) > 16384 or min(t[2] for t in tarefas) < 8:
        parser.error('malhas devem ficar entre 8 e 16384')
    if args.passos > 1000000 or max(threads) > 4096:
        parser.error('parametros excedem limites do solver')
    args.saida.mkdir(parents=True, exist_ok=False)
    ambiente = os.environ.copy()
    ambiente.setdefault('OMP_PLACES', 'cores')
    ambiente.setdefault('OMP_PROC_BIND', 'spread')
    ambiente['OMP_DYNAMIC'] = 'FALSE'
    pasta_binarios = Path(__file__).resolve().parent
    for versao in range(1, 4):
        subprocess.run([str(pasta_binarios / f'navierStokesV{versao}'), '--validar'], env=ambiente, check=True)
    metadados = {'host': platform.node(), 'plataforma': platform.platform(),
                 'parametros': {k: str(v) if isinstance(v, Path) else v for k, v in vars(args).items()},
                 'afinidade': sorted(os.sched_getaffinity(0)),
                 'ambiente': {k: v for k, v in ambiente.items() if k.startswith(('OMP_', 'SLURM_'))},
                 'compilador': subprocess.check_output(['gcc', '--version'], text=True),
                 'cpu': subprocess.check_output(['lscpu'], text=True)}
    (args.saida / 'ambiente.json').write_text(json.dumps(metadados, indent=2))
    # Copias permitem identificar exatamente o codigo e as opcoes medidos.
    for nome in ['navierStokes.c', 'navierStokes.h', 'navierStokesV1.c', 'navierStokesV2.c',
                 'navierStokesV3.c', 'Makefile', 'executarExperimentos.py', 'executarPascal.slurm']:
        (args.saida / nome).write_bytes((pasta_binarios / nome).read_bytes())
    rng = random.Random(12)
    with (args.saida / 'repeticoes.csv').open('w', newline='') as arquivo:
        escritor = csv.writer(arquivo)
        escritor.writerow(['modo', 'repeticao', 'versao', 'n', 'passos', 'threads', 'tempo_s', 'checksum', 'pico'])
        checksums = {}
        for repeticao in range(1, args.repeticoes+1):
            rng.shuffle(tarefas)
            for modo, v, n, p in tarefas:
                saida = subprocess.check_output([str(pasta_binarios / f'navierStokesV{v}'), str(n), str(args.passos), str(p)], env=ambiente, text=True)
                valores = next(csv.reader([saida.strip()]))
                if len(valores) != 7 or [int(x) for x in valores[:4]] != [v, n, args.passos, p]:
                    raise RuntimeError('saida inesperada do solver')
                tempo, checksum, pico = map(float, valores[4:])
                if not all(map(math.isfinite, [tempo, checksum, pico])) or tempo <= 0:
                    raise RuntimeError('medicao invalida')
                chave = (n, args.passos)
                if chave in checksums and not math.isclose(checksum, checksums[chave], rel_tol=1e-12, abs_tol=1e-12):
                    raise RuntimeError('versoes produziram checksums diferentes')
                checksums[chave] = checksum
                escritor.writerow([modo, repeticao, *valores])
                arquivo.flush()
                print(f'{modo} v{v} N={n} p={p} repeticao={repeticao}: {tempo:.6f}s', flush=True)
    analisar(args.saida)


if __name__ == '__main__':
    main()
