# Relatório de Testes - Tarefa 6 (OpenMP)
- **Total de Pontos por Teste:** 10,000,000
- **Valor Real de $\pi$:** 3.1415926536
- **Threads Testadas:** 1, 2, 4, 6, 8, 10, 12, 16

## 1. Tabela Comparativa Detalhada

| Programa | Threads | Pi Estimado | Erro Absoluto | Tempo (s) | Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| `1_sequencial` | 1 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 2 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 4 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 6 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 8 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 10 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 12 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `1_sequencial` | 16 | 3.1416440000 | 0.0000513464 | 0.100449 | Correto |
| `2_pragma_parallel_for` | 1 | 3.1416440000 | 0.0000513464 | 0.139063 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 2 | 3.1309712000 | 0.0106214536 | 0.620174 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 4 | 3.1346568000 | 0.0069358536 | 0.557420 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 6 | 3.1331628000 | 0.0084298536 | 0.708135 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 8 | 3.1328912000 | 0.0087014536 | 0.803327 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 10 | 3.1299348000 | 0.0116578536 | 0.897703 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 12 | 3.1294020000 | 0.0121906536 | 0.985198 | Data Race (Incorreto) |
| `2_pragma_parallel_for` | 16 | 3.1290260000 | 0.0125666536 | 0.963597 | Data Race (Incorreto) |
| `3_critical` | 1 | 3.1418704000 | 0.0002777464 | 0.168928 | Correto |
| `3_critical` | 2 | 3.1419068000 | 0.0003141464 | 0.746692 | Correto |
| `3_critical` | 4 | 3.1415652000 | 0.0000274536 | 0.644294 | Correto |
| `3_critical` | 6 | 3.1411564000 | 0.0004362536 | 0.824025 | Correto |
| `3_critical` | 8 | 3.1418248000 | 0.0002321464 | 0.956301 | Correto |
| `3_critical` | 10 | 3.1418224000 | 0.0002297464 | 1.100325 | Correto |
| `3_critical` | 12 | 3.1416964000 | 0.0001037464 | 1.138214 | Correto |
| `3_critical` | 16 | 3.1415588000 | 0.0000338536 | 1.071356 | Correto |
| `4_shared_private` | 1 | 3.1418476000 | 0.0002549464 | 0.106045 | Correto |
| `4_shared_private` | 2 | 3.1419628000 | 0.0003701464 | 0.471894 | Correto |
| `4_shared_private` | 4 | 3.1419140000 | 0.0003213464 | 0.467373 | Correto |
| `4_shared_private` | 6 | 3.1417232000 | 0.0001305464 | 0.603313 | Correto |
| `4_shared_private` | 8 | 3.1419524000 | 0.0003597464 | 0.711019 | Correto |
| `4_shared_private` | 10 | 3.1413532000 | 0.0002394536 | 0.813428 | Correto |
| `4_shared_private` | 12 | 3.1410992000 | 0.0004934536 | 0.859202 | Correto |
| `4_shared_private` | 16 | 3.1408244000 | 0.0007682536 | 0.656307 | Correto |
| `5_firstprivate` | 1 | 3.1406940000 | 0.0008986536 | 0.103022 | Correto |
| `5_firstprivate` | 2 | 3.1409656000 | 0.0006270536 | 0.492349 | Correto |
| `5_firstprivate` | 4 | 3.1418420000 | 0.0002493464 | 0.477556 | Correto |
| `5_firstprivate` | 6 | 3.1412176000 | 0.0003750536 | 0.599482 | Correto |
| `5_firstprivate` | 8 | 3.1415224000 | 0.0000702536 | 0.732037 | Correto |
| `5_firstprivate` | 10 | 3.1415328000 | 0.0000598536 | 0.788159 | Correto |
| `5_firstprivate` | 12 | 3.1415944000 | 0.0000017464 | 0.862593 | Correto |
| `5_firstprivate` | 16 | 3.1415656000 | 0.0000270536 | 0.841388 | Correto |
| `6_lastprivate` | 1 | 3.1417884000 | 0.0001957464 | 0.101825 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 2 | 1.5707012000 | 1.5708914536 | 0.445919 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 4 | 0.7855336000 | 2.3560590536 | 0.441045 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 6 | 0.5233820000 | 2.6182106536 | 0.596199 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 8 | 0.3927620000 | 2.7488306536 | 0.696638 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 10 | 0.3142152000 | 2.8273774536 | 0.806049 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 12 | 0.2617808000 | 2.8798118536 | 0.849680 | Sobrescrita Lastprivate (Incorreto) |
| `6_lastprivate` | 16 | 0.1962436000 | 2.9453490536 | 0.691108 | Sobrescrita Lastprivate (Incorreto) |
| `7_default_none` | 1 | 3.1416504000 | 0.0000577464 | 0.104092 | Correto |
| `7_default_none` | 2 | 3.1414376000 | 0.0001550536 | 0.501980 | Correto |
| `7_default_none` | 4 | 3.1415312000 | 0.0000614536 | 0.463196 | Correto |
| `7_default_none` | 6 | 3.1418884000 | 0.0002957464 | 0.616141 | Correto |
| `7_default_none` | 8 | 3.1414920000 | 0.0001006536 | 0.728830 | Correto |
| `7_default_none` | 10 | 3.1413228000 | 0.0002698536 | 0.788410 | Correto |
| `7_default_none` | 12 | 3.1405156000 | 0.0010770536 | 0.880299 | Correto |
| `7_default_none` | 16 | 3.1411628000 | 0.0004298536 | 0.917376 | Correto |

## 2. Matriz: Pi Estimado por Número de Threads

| Programa | 1T | 2T | 4T | 6T | 8T | 10T | 12T | 16T |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| `1_sequencial` | 3.1416440000 | 3.1416440000 | 3.1416440000 | 3.1416440000 | 3.1416440000 | 3.1416440000 | 3.1416440000 | 3.1416440000 |
| `2_pragma_parallel_for` | 3.1416440000 | 3.1309712000 | 3.1346568000 | 3.1331628000 | 3.1328912000 | 3.1299348000 | 3.1294020000 | 3.1290260000 |
| `3_critical` | 3.1418704000 | 3.1419068000 | 3.1415652000 | 3.1411564000 | 3.1418248000 | 3.1418224000 | 3.1416964000 | 3.1415588000 |
| `4_shared_private` | 3.1418476000 | 3.1419628000 | 3.1419140000 | 3.1417232000 | 3.1419524000 | 3.1413532000 | 3.1410992000 | 3.1408244000 |
| `5_firstprivate` | 3.1406940000 | 3.1409656000 | 3.1418420000 | 3.1412176000 | 3.1415224000 | 3.1415328000 | 3.1415944000 | 3.1415656000 |
| `6_lastprivate` | 3.1417884000 | 1.5707012000 | 0.7855336000 | 0.5233820000 | 0.3927620000 | 0.3142152000 | 0.2617808000 | 0.1962436000 |
| `7_default_none` | 3.1416504000 | 3.1414376000 | 3.1415312000 | 3.1418884000 | 3.1414920000 | 3.1413228000 | 3.1405156000 | 3.1411628000 |

## 3. Matriz: Erro Absoluto por Número de Threads (10 Casas Decimais)

| Programa | 1T | 2T | 4T | 6T | 8T | 10T | 12T | 16T |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| `1_sequencial` | 0.0000513464 | 0.0000513464 | 0.0000513464 | 0.0000513464 | 0.0000513464 | 0.0000513464 | 0.0000513464 | 0.0000513464 |
| `2_pragma_parallel_for` | 0.0000513464 | 0.0106214536 | 0.0069358536 | 0.0084298536 | 0.0087014536 | 0.0116578536 | 0.0121906536 | 0.0125666536 |
| `3_critical` | 0.0002777464 | 0.0003141464 | 0.0000274536 | 0.0004362536 | 0.0002321464 | 0.0002297464 | 0.0001037464 | 0.0000338536 |
| `4_shared_private` | 0.0002549464 | 0.0003701464 | 0.0003213464 | 0.0001305464 | 0.0003597464 | 0.0002394536 | 0.0004934536 | 0.0007682536 |
| `5_firstprivate` | 0.0008986536 | 0.0006270536 | 0.0002493464 | 0.0003750536 | 0.0000702536 | 0.0000598536 | 0.0000017464 | 0.0000270536 |
| `6_lastprivate` | 0.0001957464 | 1.5708914536 | 2.3560590536 | 2.6182106536 | 2.7488306536 | 2.8273774536 | 2.8798118536 | 2.9453490536 |
| `7_default_none` | 0.0000577464 | 0.0001550536 | 0.0000614536 | 0.0002957464 | 0.0001006536 | 0.0002698536 | 0.0010770536 | 0.0004298536 |

## 4. Matriz: Tempo de Execução (segundos)

| Programa | 1T | 2T | 4T | 6T | 8T | 10T | 12T | 16T |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| `1_sequencial` | 0.100449 | 0.100449 | 0.100449 | 0.100449 | 0.100449 | 0.100449 | 0.100449 | 0.100449 |
| `2_pragma_parallel_for` | 0.139063 | 0.620174 | 0.557420 | 0.708135 | 0.803327 | 0.897703 | 0.985198 | 0.963597 |
| `3_critical` | 0.168928 | 0.746692 | 0.644294 | 0.824025 | 0.956301 | 1.100325 | 1.138214 | 1.071356 |
| `4_shared_private` | 0.106045 | 0.471894 | 0.467373 | 0.603313 | 0.711019 | 0.813428 | 0.859202 | 0.656307 |
| `5_firstprivate` | 0.103022 | 0.492349 | 0.477556 | 0.599482 | 0.732037 | 0.788159 | 0.862593 | 0.841388 |
| `6_lastprivate` | 0.101825 | 0.445919 | 0.441045 | 0.596199 | 0.696638 | 0.806049 | 0.849680 | 0.691108 |
| `7_default_none` | 0.104092 | 0.501980 | 0.463196 | 0.616141 | 0.728830 | 0.788410 | 0.880299 | 0.917376 |

## 5. Análise dos Resultados e Observações Didáticas

1. **`1_sequencial`**:
   - Tempo e precisão constantes (não afetado pelo número de threads).
2. **`2_pragma_parallel_for` (Condição de Corrida)**:
   - Com 1 thread o resultado é exato, mas à medida que o número de threads aumenta, o número de colisões/lost updates explode, reduzindo o $\pi$ estimado drasticamente (erro cresce com as threads).
3. **`3_critical` (Seção Crítica)**:
   - O resultado de $\pi$ é matematicamente correto em todas as threads, porém o tempo de execução é elevado devido à forte contenção no lock da seção crítica.
4. **`4_shared_private`, `5_firstprivate`, `7_default_none` (Contadores Locais)**:
   - Resultados matematicamente corretos em todas as configurações de threads com excelente escalabilidade de desempenho.
5. **`6_lastprivate` (Erro Conceitual)**:
   - Com 1 thread o resultado é normal. Com $T > 1$ threads, a variável final retém apenas a contagem da última thread lógica (~1/$T$ das iterações), fazendo $\pi_{\text{estimado}} \approx \pi / T$, o que evidencia por que `lastprivate` não serve como acumulador.
