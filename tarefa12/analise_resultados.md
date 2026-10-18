# Análise dos resultados da tarefa 12

A V3 apresenta a melhor combinação de tempo de execução e escalabilidade, sobretudo nas maiores entradas. A V2 melhora o tempo em relação à V1, mas seu speedup é menor; isso ocorre porque a referência com uma thread também ficou mais rápida. As três versões perdem eficiência ao aumentar as threads com uma entrada fixa, especialmente na menor malha.

## Enunciado e condições da avaliação

A tarefa 12, no slide numerado 57 do arquivo `Programacao_Paralela.pdf` (página 55 do PDF), solicita avaliar o código de Navier–Stokes em um nó do NPAD, identificar gargalos, apresentar a evolução das otimizações e comentar escalabilidade, escalabilidade fraca e forte.

Foram inspecionadas as 18 imagens de `pascalViewer`, seis por versão. Todas representam **Whole Program (Region 0)**. Os valores numéricos abaixo foram calculados como médias das cinco repetições dos JSON de `resultadosNpad/pascal-2213650`, configuração com 1, 4, 8, 16 e 32 threads compatível com os eixos e as faixas de speedup das imagens. A outra execução arquivada, `pascal-2212454`, contém apenas 1, 4 e 16 threads e não foi misturada à análise. As imagens, isoladamente, não registram o identificador do job.

O nó registrado é `r2n50`, com processadores AMD EPYC 7713, dois sockets e oito domínios NUMA. O job solicita 32 CPUs, sem multithreading, com `OMP_PLACES=cores`, `OMP_PROC_BIND=spread` e `OMP_DYNAMIC=FALSE`. A existência de 256 CPUs lógicas no inventário do nó não significa que o experimento utilizou todas elas.

| Entrada nos gráficos | Pontos internos | Malha interna | Passos |
|---|---:|---:|---:|
| i1 | 65.536 | 256 × 256 | 300 |
| i2 | 262.144 | 512 × 512 | 300 |
| i3 | 1.048.576 | 1.024 × 1.024 | 300 |
| i4 | 4.194.304 | 2.048 × 2.048 | 300 |

Cada entrada quadruplica os pontos internos. O programa acrescenta duas posições por dimensão para os contornos; por exemplo, i1 usa uma grade alocada de 258 × 258.

## Orientação para a leitura das imagens

Em todos os gráficos, as **linhas indicam o número de threads**, que aumenta de cima para baixo. As **colunas indicam as entradas**, que aumentam da esquerda para a direita. Assim, o canto inferior direito corresponde à maior entrada com 32 threads. Nos mapas de escalabilidade geral e fraca, as colunas começam em i2.

Cada gráfico tem sua própria legenda. Em `Performance`, amarelo-claro indica maior desempenho na escala apresentada. Em `Efficiency`, a interpretação se inverte: roxo-escuro e preto indicam maior eficiência, enquanto laranja e amarelo indicam menor eficiência. Em `Speedup`, azul representa ganhos menores e vermelho representa ganhos maiores. Nos três mapas de escalabilidade, vermelho indica índice negativo e azul indica índice positivo.

As análises abaixo seguem a mesma sequência: o que aparece nas imagens de cada versão, o que isso significa e o que a comparação permite concluir. Os números dos JSON complementam a leitura visual.

## Análise dos gráficos de desempenho — Performance

**Na V1**, a linha de uma thread é roxo-escura. Ao descer para 4, 8 e 16 threads, aparecem tons vermelhos, laranjas e amarelos. Nas entradas i2, i3 e i4, a linha de 32 threads é a mais clara. Essa mudança indica que acrescentar threads aumenta o desempenho nessas entradas. Em i1, porém, a célula de 16 threads é ligeiramente mais clara que a de 32: para o menor problema, o uso de mais threads deixa de ajudar.

**Na V2**, também há uma transição de roxo para amarelo ao aumentar as threads. A linha de 32 threads fica clara nas quatro entradas. Em i1, há uma pequena perda de desempenho de 8 para 16 threads, seguida de recuperação com 32. Nas maiores entradas, o desempenho continua aumentando, mas os tempos mostram ganhos cada vez menores nas últimas contagens de threads.

**Na V3**, a coluna i1 fica mais clara até 16 threads e volta a um tom um pouco mais escuro com 32. Nas maiores entradas, a linha de 32 threads é a mais clara. Em i4, as linhas intermediárias permanecem relativamente escuras em comparação com a última linha, evidenciando que o desempenho ainda cresce bastante ao chegar a 32 threads.

**Comparando as versões**, as cores ajudam a acompanhar a evolução dentro de cada imagem, mas não fornecem uma comparação direta de tempo entre imagens: a escala exibida vai de 0 a 100, sem unidade de segundos. Para decidir qual versão é mais rápida, usamos os tempos médios do programa completo em i4:

| Threads | V1 | V2 | V3 |
|---:|---:|---:|---:|
| 1 | 3985,34 | 2413,94 | 2474,23 |
| 4 | 1151,75 | 751,89 | 622,29 |
| 8 | 673,68 | 472,37 | 321,84 |
| 16 | 437,33 | 337,83 | 172,42 |
| 32 | 344,99 | 300,97 | 102,48 |


A V3 é a mais rápida nas configurações paralelas dessa tabela. Com 32 threads, leva 102,48 ms, contra 344,99 ms da V1 e 300,97 ms da V2: é, respectivamente, 3,37 e 2,94 vezes mais rápida. Com uma thread, a V2 tem o menor tempo. Portanto, a vantagem da V3 se destaca quando há recursos para executar o trabalho em paralelo.

## Análise dos gráficos de speedup — Speedup

Speedup mede quantas vezes uma execução fica mais rápida em relação à mesma versão com uma thread: **S(p) = T(1)/T(p)**. Por exemplo, speedup de 8× significa que o tempo foi dividido por oito.

**Na V1**, a linha de 4 threads é azul. Ao descer e avançar para as maiores entradas, as cores passam por tons claros até chegar ao vermelho. As células mais vermelhas aparecem em i3 e i4 com 32 threads, indicando os maiores ganhos da versão: 11,18× e 11,55×. A coluna i1 permanece em tons azulados ou acinzentados, mostrando que o problema pequeno aproveita pouco o aumento de threads.

**Na V2**, as maiores entradas também ficam mais vermelhas com mais threads. Entretanto, na linha de 32 threads, i2, i3 e i4 têm cores muito semelhantes. Os valores confirmam esse comportamento: 8,15×, 7,94× e 8,02×. Aumentar a entrada nessa faixa não amplia o speedup de forma consistente.

**Na V3**, o contraste entre o lado esquerdo e o canto inferior direito é mais evidente. A coluna i1 continua azulada, enquanto i3 e i4 com 32 threads ficam vermelhas. Em i4, o speedup cresce de 3,98× com 4 threads para 7,69× com 8, 14,35× com 16 e 24,14× com 32. A versão consegue transformar mais threads em uma redução expressiva do tempo quando o problema é grande.

**Comparando as versões**, a V3 alcança o maior speedup nas maiores entradas. É necessário observar a legenda: o máximo mostrado é aproximadamente 12 na V1, 9 na V2 e 25 na V3. Portanto, o vermelho da V3 representa um ganho muito maior que o vermelho da V2.

A V2 tem speedup menor que a V1, mas executa mais rápido. Isso acontece porque o speedup usa uma referência diferente para cada versão. Em i4, a V2 já é mais rápida com uma thread: 2.413,94 ms contra 3.985,34 ms da V1. Melhorar a referência reduz a razão T(1)/T(p), mesmo quando o tempo paralelo também melhora.

## Análise dos gráficos de eficiência — Efficiency

Eficiência indica o aproveitamento relativo das threads: **E(p) = S(p)/p**. Com 32 threads, um speedup de 32× corresponderia a 100% de eficiência. Neste gráfico, **quanto mais escura a célula, maior a eficiência**.

**Na V1**, a linha de 4 threads é predominantemente roxa, principalmente nas maiores entradas. Ao descer para 16 e 32 threads, aparecem tons laranjas e amarelos, sobretudo em i1. Isso mostra que o ganho de velocidade não acompanha o crescimento dos recursos. Na linha de 32 threads, i3 e i4 são mais escuras que i1: aumentar o problema ajuda, mas a eficiência em i4 ainda é de apenas 36,1%.

**Na V2**, as células de 4 threads também são roxas. Com 32 threads, porém, predominam tons amarelos e alaranjados em todas as entradas. Nas maiores malhas, essa linha é mais clara que na V1, indicando menor eficiência relativa. Em i4, a eficiência é de 25,1%. Assim, embora a V2 reduza o tempo absoluto, seu ganho em relação à própria execução com uma thread fica mais distante do ideal.

**Na V3**, as células de i3 e i4 com 4 e 8 threads são roxo-escuras ou quase pretas. Mesmo com 16 threads, essas entradas continuam escuras. Na linha de 32 threads, a cor muda de amarelo em i1 para roxo em i3 e i4. Isso indica que a V3 precisa de um problema suficientemente grande para aproveitar muitas threads, mas consegue manter eficiência muito superior à das outras versões quando esse trabalho está disponível.

**Comparando as versões**, a V3 é a melhor nas maiores entradas porque suas células permanecem mais escuras à medida que aumentam as threads. Em i4, ela mantém aproximadamente 99,4%, 96,1%, 89,7% e 75,4% de eficiência com 4, 8, 16 e 32 threads. A vantagem não se estende à menor entrada com 32 threads: nesse ponto, as três versões apresentam eficiência próxima de 12%.

Os valores abaixo permitem relacionar os gráficos de speedup e eficiência. Cada célula apresenta **speedup / eficiência**, com 32 threads:

| Entrada | V1 | V2 | V3 |
|---|---:|---:|---:|
| i1 | 3,82× / 11,9% | 3,73× / 11,7% | 3,98× / 12,4% |
| i2 | 9,20× / 28,8% | 8,15× / 25,5% | 14,38× / 44,9% |
| i3 | 11,18× / 34,9% | 7,94× / 24,8% | 19,67× / 61,5% |
| i4 | 11,55× / 36,1% | 8,02× / 25,1% | 24,14× / 75,4% |


## Análise dos gráficos de escalabilidade geral — Scalability

Este gráfico ajuda a avaliar se aumentar o tamanho do problema compensa a perda de eficiência associada ao uso de mais recursos. A legenda vai de −100 a +100: azul indica índice positivo, e azul mais intenso indica um resultado mais favorável no critério do Viewer. As imagens não apresentam a fórmula desse índice; por isso, não atribuímos percentuais exatos às suas cores.

**Na V1**, todas as células são azuis, mas com intensidade relativamente baixa. Há um escurecimento discreto em direção às maiores entradas, especialmente com 16 threads. Isso sugere que aumentar o problema ajuda a recuperar eficiência, embora a recuperação seja limitada.

**Na V2**, o azul também predomina e varia pouco entre as células. A linha de 32 threads fica relativamente clara, mesmo nas maiores entradas. O aumento do problema oferece pouca recuperação adicional nessa faixa, coerente com a eficiência de aproximadamente 25% em i2, i3 e i4.

**Na V3**, o azul fica visivelmente mais intenso ao avançar para i3 e i4, principalmente nas linhas de 16 e 32 threads. Essa região mais escura indica que a versão se beneficia mais do aumento da carga de trabalho ao usar muitos recursos.

**Comparando as versões**, a V3 apresenta a recuperação de eficiência mais expressiva. Com 32 threads, passar de i1 para i4 eleva sua eficiência de 12,4% para 75,4%. Na V1, ela passa de 11,9% para 36,1%; na V2, de 11,7% para 25,1%. Portanto, as três se beneficiam de problemas maiores, mas a V3 compensa melhor a sobrecarga do paralelismo. Isso não significa que uma malha maior execute em menos tempo: significa que ela utiliza melhor os recursos disponíveis.

## Análise dos gráficos de escalabilidade forte — Strong Scalability

A escalabilidade forte observa o efeito de aumentar as threads **mantendo a entrada fixa**. O ideal é reduzir o tempo proporcionalmente aos recursos e, assim, manter a eficiência. Visualmente, acompanhamos cada coluna de cima para baixo.

**Na V1**, as células são rosadas com 4 threads e ficam mais vermelhas ao chegar a 32. O vermelho é particularmente intenso na coluna i1. Isso indica perda de eficiência ao repartir o mesmo trabalho entre mais threads, com maior dificuldade no menor problema.

**Na V2**, também há intensificação do vermelho de cima para baixo. Nas linhas de 16 e 32 threads, as maiores entradas ficam mais vermelhas que as correspondentes da V3. Isso mostra que a versão perde aproveitamento relativo ao aumentar os recursos, mesmo com mais trabalho disponível.

**Na V3**, a coluna i1 continua fortemente vermelha nas maiores contagens de threads. Entretanto, em i3 e i4 predominam tons rosados mais claros que nas outras versões. A perda existe, mas é menor: as maiores malhas permitem que o tempo continue caindo de forma mais próxima ao ideal.

**Comparando as versões**, nenhuma mantém escalabilidade forte ideal em toda a faixa, pois os três mapas apresentam índices negativos. A V3 é a mais favorável nas maiores entradas. Em i4, dobrar de 16 para 32 threads produz ganho de 1,68× na V3, contra 1,27× na V1 e 1,12× na V2; o ideal seria 2×.

O limite é mais evidente em i1: de 16 para 32 threads, o tempo da V1 sobe de 15,73 para 17,46 ms, e o da V3 sobe de 8,69 para 10,73 ms. Nesses casos, acrescentar threads piora o resultado. Como as execuções são curtas, pequenas oscilações também podem influenciar as médias; os gráficos não isolam a causa de cada variação.

## Análise dos gráficos de escalabilidade fraca — Weak Scalability

Este mapa avalia o comportamento ao variar conjuntamente trabalho e recursos segundo o critério do Viewer. Na definição apresentada pela disciplina, a escalabilidade fraca envolve manter eficiência mesmo quando o problema cresce menos que os recursos. A legenda usa vermelho para índices negativos e azul para positivos.

**Na V1**, quase todo o mapa é vermelho ou rosado. A exceção é a célula i2 com 32 threads, que aparece azul. Isso indica um resultado favorável localizado, mas as demais combinações não sustentam o mesmo comportamento. O vermelho mais intenso em i4 com 16 threads evidencia uma região menos favorável.

**Na V2**, o padrão é parecido: predomina o vermelho, e a célula i2 com 32 threads é azul. Nas maiores entradas, especialmente i4 com 16 threads, o vermelho é mais intenso. Assim, a melhoria de tempo da V2 em relação à V1 não se transforma em uma melhoria generalizada deste critério de escalabilidade.

**Na V3**, a área azul é maior: aparece em i2 com 8 threads, em i2 e i3 com 16 threads e nas três colunas com 32 threads. Comparada às outras versões, a V3 apresenta mais combinações favoráveis ao crescimento conjunto do problema e dos recursos. Ainda há células rosadas, principalmente na linha de 4 threads e em i4 com 8 e 16 threads, de modo que o comportamento não é uniforme.

**Comparando as versões**, a V3 é a mais favorável neste mapa por ampliar a região azul. A célula azul isolada de V1 e V2 não permite classificá-las como fracamente escaláveis em todas as configurações.

Uma verificação complementar é manter **a quantidade de pontos internos por thread constante**, conforme a definição convencional de escalabilidade fraca. A sequência (1 thread, i1), (4 threads, i2), (16 threads, i3) mantém 65.536 pontos internos por thread. Nesse teste, o ideal é que o tempo permaneça constante:

| Versão | T(1, i1), ms | T(4, i2), ms | T(16, i3), ms | T(1, i1)/T(16, i3) |
|---|---:|---:|---:|---:|
| V1 | 66,71 | 93,94 | 123,79 | 53,9% |
| V2 | 42,25 | 66,92 | 95,47 | 44,3% |
| V3 | 42,73 | 58,75 | 47,19 | 90,5% |


A V1 e a V2 ficam progressivamente mais lentas nessa sequência. Na V3, o tempo final fica apenas 10,5% acima do inicial, embora o caso intermediário seja mais lento que ambos. Isso reforça a vantagem da V3, sem demonstrar manutenção perfeita do tempo.

O resultado também depende do ponto de partida. De (8 threads, i1) para (32 threads, i2), outra comparação com carga por thread constante, a razão entre os tempos inicial e final é de 53,1% na V1, 49,4% na V2 e 69,9% na V3. Portanto, mesmo a V3 apresenta perdas em parte das configurações. As entradas disponíveis não permitem montar uma sequência de carga constante para todas as contagens de threads.

## Evolução do código e gargalos

**V1 → V2.** A V1 usa `collapse(2)` e três regiões `parallel for` por passo: uma para atualizar o interior e duas para copiar os contornos. A V2 distribui linhas, acrescenta `omp simd` ao laço interno e preserva os contornos nos dois buffers, eliminando as cópias por passo. Isso reduz trabalho e entradas em regiões paralelas. O tempo melhora, mas a inicialização continua serial e limita o ganho global. A presença de `omp simd` expressa a intenção de vetorização; sem relatório do compilador, não é possível quantificar sua contribuição isolada.

**V2 → V3.** A V3 mantém uma região paralela durante os passos, usa ponteiros privados para a troca de buffers e conserva a barreira necessária ao fim de cada atualização. Além disso, o código compartilhado ativa inicialização paralela para a V3. Logo, a melhora global combina otimização do cálculo e da inicialização; não pode ser atribuída apenas à região persistente.

Os JSON permitem separar essas contribuições em i4 com 32 threads:

| Versão | Programa completo, ms | Inicialização (região 1), ms | Evolução (região 2), ms |
|---|---:|---:|---:|
| V1 | 344,99 | 89,09 | 156,36 |
| V2 | 300,97 | 89,25 | 113,80 |
| V3 | 102,48 | 2,12 | 80,35 |

A inicialização consome cerca de 89 ms na V1 e na V2, contra 2,12 ms na V3. Na V2 ela representa aproximadamente 29,7% do tempo total. A evolução também melhora, de 113,80 ms na V2 para 80,35 ms na V3. A soma das duas regiões é menor que o tempo completo porque há alocação, aquecimento, verificação, saída e outras etapas fora delas.

Os gargalos diretamente sustentados pelo código e pelos resultados são a inicialização serial em V1/V2, a repetição de regiões paralelas e o custo das sincronizações diante de pouco trabalho por thread. Os acessos às quatro matrizes tornam largura de banda, caches e posicionamento NUMA candidatos adicionais. A inicialização paralela da V3 pode favorecer a distribuição das páginas por primeiro acesso, mas os gráficos não medem tráfego de memória nem acesso remoto. Confirmar saturação de banda ou um efeito NUMA específico exigiria medições adicionais.

Para a tarefa 12, o progresso principal é a passagem de uma implementação que reduz o tempo, mas satura cedo, para uma V3 que aproveita melhor o crescimento do problema. A maior malha com 32 threads é o melhor caso de escalabilidade relativa da V3, com 24,14× de speedup e 75,4% de eficiência. A menor malha continua limitada pela sobrecarga e não se beneficia de aumentar indiscriminadamente as threads.

## Imagens analisadas

Os links abaixo apontam para os arquivos originais, sem alterar as imagens:

| Métrica | V1 | V2 | V3 |
|---|---|---|---|
| Performance | [Abrir](<pascalViewer/versao1/Performance_Whole_Program_(Region_0).png>) | [Abrir](<pascalViewer/versao2/Performance_Whole_Program_(Region_0) (1).png>) | [Abrir](<pascalViewer/versao3/Performance_Whole_Program_(Region_0) (2).png>) |
| Speedup | [Abrir](<pascalViewer/versao1/Speedup_Whole_Program_(Region_0).png>) | [Abrir](<pascalViewer/versao2/Speedup_Whole_Program_(Region_0) (1).png>) | [Abrir](<pascalViewer/versao3/Speedup_Whole_Program_(Region_0) (2).png>) |
| Efficiency | [Abrir](<pascalViewer/versao1/Efficiency_Whole_Program_(Region_0).png>) | [Abrir](<pascalViewer/versao2/Efficiency_Whole_Program_(Region_0) (1).png>) | [Abrir](<pascalViewer/versao3/Efficiency_Whole_Program_(Region_0) (2).png>) |
| Scalability | [Abrir](<pascalViewer/versao1/Scalability_Whole_Program_(Region_0).png>) | [Abrir](<pascalViewer/versao2/Scalability_Whole_Program_(Region_0) (1).png>) | [Abrir](<pascalViewer/versao3/Scalability_Whole_Program_(Region_0) (2).png>) |
| Strong Scalability | [Abrir](<pascalViewer/versao1/Strong_Scalability_Whole_Program_(Region_0).png>) | [Abrir](<pascalViewer/versao2/Strong_Scalability_Whole_Program_(Region_0) (1).png>) | [Abrir](<pascalViewer/versao3/Strong_Scalability_Whole_Program_(Region_0) (2).png>) |
| Weak Scalability | [Abrir](<pascalViewer/versao1/Weak_Scalability_Whole_Program_(Region_0).png>) | [Abrir](<pascalViewer/versao2/Weak_Scalability_Whole_Program_(Region_0) (1).png>) | [Abrir](<pascalViewer/versao3/Weak_Scalability_Whole_Program_(Region_0) (2).png>) |
