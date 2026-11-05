# Resultados de chunks - Tarefa 11

- **Malha:** 512x512
- **Passos:** 20000
- **Threads fixas:** 8
- **Iteracoes por situacao:** 10
- **Chunk fixo:** 16

Mediana do tempo de execucao em segundos:

| Chunk | Versao 1 - static | Versao 2 - static + collapse(2) | Versao 3 - dynamic | Versao 4 - dynamic + collapse(2) | Versao 5 - guided | Versao 6 - guided + collapse(2) |
|---:|---:---:---:---:---:---:|
| 16 | 0,584623 | 3,230549 | 0,482959 | 6,025394 | 0,500743 | 1,813318 |
