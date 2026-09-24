# Tabelas dos resultados no NPAD

Parâmetros comuns: 500 passos e `alpha = 0,24`. Os tempos são o máximo entre os processos, em segundos. A repetição 1 foi descartada como aquecimento; cada mediana usa as repetições 2 e 3.

## Medianas — 2 nós / 2 processos

| Células | Bloqueante (s) | Não bloqueante + Wait (s) | Sobreposição + Test (s) | Melhor versão |
|---:|---:|---:|---:|:---|
| 100.000 | 0,033453 | 0,029195 | 0,028792 | Sobreposição + Test |
| 1.000.000 | 0,253331 | 0,244263 | 0,248946 | Não bloqueante + Wait |
| 2.000.000 | 0,460288 | 0,450252 | 0,442400 | Sobreposição + Test |
| 5.000.000 | 1,165172 | 1,139717 | 1,172414 | Não bloqueante + Wait |
| 7.000.000 | 2,446133 | 2,454813 | 2,473100 | Bloqueante |
| 10.000.000 | 3,627470 | 3,635232 | 3,698887 | Bloqueante |

## Medianas — 8 nós / 8 processos

| Células | Bloqueante (s) | Não bloqueante + Wait (s) | Sobreposição + Test (s) | Melhor versão |
|---:|---:|---:|---:|:---|
| 100.000 | 0,039462 | 0,018505 | 0,018587 | Não bloqueante + Wait |
| 1.000.000 | 0,091977 | 0,066390 | 0,065318 | Sobreposição + Test |
| 2.000.000 | 0,207194 | 0,150367 | 0,145002 | Sobreposição + Test |
| 5.000.000 | 0,356417 | 0,324146 | 0,742902 | Não bloqueante + Wait |
| 7.000.000 | 0,458537 | 1,088826 | 1,069567 | Bloqueante |
| 10.000.000 | 0,603468 | 0,572361 | 0,567997 | Sobreposição + Test |

## Amostras consideradas — 2 nós / 2 processos

| Células | Programa | Repetição 2 (s) | Repetição 3 (s) | Mediana (s) |
|---:|:---|---:|---:|---:|
| 100.000 | Bloqueante | 0,033380 | 0,033526 | 0,033453 |
| 100.000 | Não bloqueante + Wait | 0,029376 | 0,029014 | 0,029195 |
| 100.000 | Sobreposição + Test | 0,029136 | 0,028449 | 0,028792 |
| 1.000.000 | Bloqueante | 0,253093 | 0,253569 | 0,253331 |
| 1.000.000 | Não bloqueante + Wait | 0,241427 | 0,247100 | 0,244263 |
| 1.000.000 | Sobreposição + Test | 0,248394 | 0,249498 | 0,248946 |
| 2.000.000 | Bloqueante | 0,456582 | 0,463994 | 0,460288 |
| 2.000.000 | Não bloqueante + Wait | 0,446293 | 0,454212 | 0,450252 |
| 2.000.000 | Sobreposição + Test | 0,448028 | 0,436773 | 0,442400 |
| 5.000.000 | Bloqueante | 1,182120 | 1,148224 | 1,165172 |
| 5.000.000 | Não bloqueante + Wait | 1,138440 | 1,140994 | 1,139717 |
| 5.000.000 | Sobreposição + Test | 1,164679 | 1,180150 | 1,172414 |
| 7.000.000 | Bloqueante | 2,448139 | 2,444126 | 2,446133 |
| 7.000.000 | Não bloqueante + Wait | 2,457545 | 2,452080 | 2,454813 |
| 7.000.000 | Sobreposição + Test | 2,472173 | 2,474027 | 2,473100 |
| 10.000.000 | Bloqueante | 3,625497 | 3,629442 | 3,627470 |
| 10.000.000 | Não bloqueante + Wait | 3,632921 | 3,637543 | 3,635232 |
| 10.000.000 | Sobreposição + Test | 3,695956 | 3,701819 | 3,698887 |

## Amostras consideradas — 8 nós / 8 processos

| Células | Programa | Repetição 2 (s) | Repetição 3 (s) | Mediana (s) |
|---:|:---|---:|---:|---:|
| 100.000 | Bloqueante | 0,039788 | 0,039137 | 0,039462 |
| 100.000 | Não bloqueante + Wait | 0,018158 | 0,018851 | 0,018505 |
| 100.000 | Sobreposição + Test | 0,018562 | 0,018613 | 0,018587 |
| 1.000.000 | Bloqueante | 0,093752 | 0,090201 | 0,091977 |
| 1.000.000 | Não bloqueante + Wait | 0,066641 | 0,066139 | 0,066390 |
| 1.000.000 | Sobreposição + Test | 0,066318 | 0,064318 | 0,065318 |
| 2.000.000 | Bloqueante | 0,207922 | 0,206466 | 0,207194 |
| 2.000.000 | Não bloqueante + Wait | 0,154220 | 0,146515 | 0,150367 |
| 2.000.000 | Sobreposição + Test | 0,146253 | 0,143752 | 0,145002 |
| 5.000.000 | Bloqueante | 0,362973 | 0,349860 | 0,356417 |
| 5.000.000 | Não bloqueante + Wait | 0,327650 | 0,320642 | 0,324146 |
| 5.000.000 | Sobreposição + Test | 0,320252 | 1,165552 | 0,742902 |
| 7.000.000 | Bloqueante | 0,447781 | 0,469293 | 0,458537 |
| 7.000.000 | Não bloqueante + Wait | 1,751166 | 0,426485 | 1,088826 |
| 7.000.000 | Sobreposição + Test | 1,719274 | 0,419859 | 1,069567 |
| 10.000.000 | Bloqueante | 0,605101 | 0,601835 | 0,603468 |
| 10.000.000 | Não bloqueante + Wait | 0,569225 | 0,575496 | 0,572361 |
| 10.000.000 | Sobreposição + Test | 0,575253 | 0,560740 | 0,567997 |

> **Atenção:** em 8 processos há amostras muito acima das demais: Sobreposição com 5.000.000 células (repetição 3), e Não bloqueante/Sobreposição com 7.000.000 células (repetição 2). Como há apenas duas amostras úteis, esses valores elevam diretamente a mediana. Repita a bateria com mais amostras antes de tirar conclusões para esses dois tamanhos.
