# Resumos — tarefas 1 a 20

## Como organizar qualquer defesa

1. Problema e objetivo da comparação — 30 segundos.
2. Funcionamento, divisão de dados e dependências — 60–90 segundos.
3. Trecho central do código — 60 segundos.
4. Resultado com carga, recursos e unidade — 30–60 segundos.
5. Conclusão e limites — 30 segundos.

## Tarefa 1 — Aproximação de π

### Ideia central

“Aproximei π pela série de Leibniz: π/4 = 1 − 1/3 + 1/5 − 1/7 + … . O programa recebe a quantidade de termos, acumula a soma com sinais alternados e multiplica o resultado por quatro.

O objetivo é relacionar quantidade de trabalho e precisão. O custo cresce linearmente com o número de termos, mas a convergência é lenta. Em aritmética exata, o erro de truncamento para N termos é limitado por 4/(2N+1).

O programa imprime a aproximação e a diferença absoluta em relação à constante PI em double. Ele não mede o tempo internamente; uma comparação de desempenho exige uma medição externa e a mesma configuração de compilação.”

### Código mais relevante

Abra `tarefa1/main.c`: função `calcular_valor_pi`, denominador `2.0*i+1.0`, alternância por `i % 2`, multiplicação por quatro e cálculo com `fabs`. O argumento é convertido com `strtoull`, mas não há validação completa da entrada.

### Perguntas de aprofundamento

- **Qual a complexidade?** O(N) em tempo e O(1) em memória auxiliar.
- **Dobrar os termos dobra a precisão?** Reduz aproximadamente o erro de truncamento pela metade; não dobra a quantidade de dígitos corretos.
- **O erro impresso é exato?** É uma diferença calculada em ponto flutuante contra uma constante de precisão finita.
- **Daria para paralelizar?** Os termos podem ser calculados independentemente, mas a soma compartilhada exigiria uma redução. A ordem da soma pode alterar o arredondamento.

## Tarefa 2 — Localidade no produto matriz-vetor

### Ideia central

“Calculei y=A*x de duas formas, mantendo a matriz armazenada por linhas no vetor A. Na primeira, o laço interno percorre as colunas de uma mesma linha. Na segunda, percorre uma coluna e atualiza várias posições de y.

As duas formas fazem O(N²) operações, mas acessam a memória em ordens diferentes. Em `A[i*N+j]`, variar j alcança elementos consecutivos; variar i produz saltos de N elementos. Isso altera o aproveitamento das linhas de cache e a atuação do prefetch.

O programa mede cada multiplicação com CLOCK_MONOTONIC e deixa a inicialização fora desses intervalos. A versão por linhas executa primeiro, portanto a ordem e o estado dos caches também fazem parte da medição. O fonte imprime os tempos, mas não compara os vetores de saída.”

### Código mais relevante

Abra `tarefa2/main.c`: funções `multiplicacao_matriz_vetor_linha`, `multiplicacao_matriz_vetor_coluna` e `realizar_teste`. Mostre o índice linear e a posição dos relógios. O argumento opcional define N; sem ele, o programa percorre uma lista de tamanhos.

### Perguntas de aprofundamento

- **Trocar os laços muda o resultado matemático?** As duas versões calculam o mesmo produto matriz-vetor.
- **Por que operações iguais podem ter tempos diferentes?** O tempo depende também de movimentação de dados, cache e instruções geradas.
- **Quanto ocupa a matriz?** `N*N*sizeof(double)` bytes, além dos vetores; a memória cresce quadraticamente.
- **Como fortalecer a comparação?** Conferir todo o resultado, repetir as medições e alternar a ordem das versões.

## Tarefa 3 — Dependências e múltiplos acumuladores

### Ideia central

“Comparei duas somas de um vetor com cinco milhões de doubles. A primeira usa um acumulador: cada adição depende do valor produzido pela anterior. A segunda desenrola o laço por quatro e usa quatro acumuladores independentes, combinados ao final.

Essa mudança oferece ao processador mais operações independentes para sobrepor. É paralelismo em nível de instruções dentro de uma thread; o código não cria threads OpenMP.

As duas versões continuam O(N). O ganho depende do processador, do compilador e do custo de acessar os dados. O programa mede inicialização e cada laço separadamente; a soma final dos quatro acumuladores fica fora do intervalo do segundo laço.”

### Código mais relevante

Abra `tarefa3/main.c`: inicialização `vetor[i] = i*0.5`, soma dependente, laço `i += 4`, quatro acumuladores e combinação final. Mostre `CLOCK_MONOTONIC` e as somas impressas.

### Perguntas de aprofundamento

- **Desenrolar o laço garante aceleração?** Não. Pode reduzir dependências e controle, mas o resultado depende das otimizações e do hardware.
- **Isso é SIMD?** Quatro acumuladores no fonte não provam vetorização; seria necessário examinar o código gerado ou o relatório do compilador.
- **Por que as somas podem diferir em outros dados?** Adição em ponto flutuante não é associativa, e a ordem de agrupamento mudou.
- **Por que testar os limites i+1, i+2 e i+3?** Para tratar um tamanho que não seja múltiplo de quatro.

## Tarefa 4 — Limites de processamento e memória

### Ideia central

“Comparei dois perfis de carga com OpenMP. O programa limitado por processamento aplica muitas operações matemáticas a cada elemento de um vetor pequeno. O limitado por memória soma dois vetores grandes em um terceiro, fazendo pouca aritmética por elemento transferido.

No primeiro, cada iteração externa mantém um valor local e executa 50.000 iterações com sin, cos, sqrt e tan. No segundo, três vetores de 50 milhões de doubles ocupam aproximadamente 1,2 GB em bytes decimais, e a soma é repetida dez vezes.

Aumentar threads pode ajudar até atingir o recurso dominante. Na soma vetorial, a largura de banda pode saturar antes de usar toda a capacidade aritmética. Essa interpretação deve ser confirmada pelas medições; os nomes dos arquivos não substituem uma análise do hardware.”

### Código mais relevante

Abra `tarefa4/limitado_cpu.c` e `limitado_memoria.c`. Compare os laços internos, os tamanhos e as regiões medidas com `omp_get_wtime`. Na versão de memória, a inicialização paralela ocorre antes da medição; as dez somas usam uma região paralela persistente.

### Perguntas de aprofundamento

- **O que é intensidade aritmética?** A quantidade de operações em relação ao volume de dados movimentado.
- **Por que mais threads podem deixar de ajudar?** Elas disputam recursos compartilhados, como largura de banda, e acrescentam custos de execução e sincronização.
- **First touch garante localidade NUMA?** Não por si só. A inicialização paralela pode favorecer a distribuição de páginas, dependendo do sistema e do posicionamento das threads.
- **O checksum valida tudo?** Não. O programa imprime apenas um elemento; isso oferece uma conferência parcial.

## Tarefa 5 — Primos e condição de corrida

### Ideia central

“Comparei a contagem de primos entre 2 e um limite nas versões sequencial e OpenMP. Cada candidato pode ser testado independentemente, mas a contagem total reúne os resultados de todas as iterações.

O fonte paralelo atual usa parallel for e incrementa count diretamente. Como count é compartilhado e não tem proteção, há condição de corrida: duas threads podem ler o mesmo valor e perder um incremento. Um resultado aparentemente correto em uma execução não elimina esse problema.

A correção natural seria uma redução de soma, mas ela ainda não está no arquivo. Antes de avaliar speedup, é necessário garantir a contagem correta. Também há desequilíbrio potencial: candidatos compostos podem encerrar o teste cedo, enquanto primos percorrem todos os divisores testados.”

### Código mais relevante

Abra `tarefa5/sequencial.c` e `paralelo.c`: função `eh_primo`, laço de candidatos e `count++`. No paralelo, destaque a ausência de `reduction(+:count)`. O teste de divisores do código vai de 3 até n−1, sem a otimização de parar na raiz quadrada.

### Perguntas de aprofundamento

- **Por que count++ não é seguro?** É uma operação de leitura, modificação e escrita que pode se intercalar com a de outra thread.
- **Private(count) resolveria?** Isolaria os contadores, mas ainda faltaria inicializá-los e combinar os resultados.
- **Atomic resolveria?** Protegeria os incrementos; uma redução evita atualizar um único contador compartilhado a cada primo.
- **Dynamic sempre seria melhor?** Não. Pode melhorar o balanceamento, mas acrescenta custo de distribuição; é preciso comparar após corrigir a corrida.

## Tarefa 6 — Monte Carlo e escopo dos dados

### Ideia central

“Estimei π sorteando pontos no quadrado unitário. A fração que satisfaz x²+y² <= 1 aproxima π/4. As versões mostram que distribuir iterações exige também definir corretamente o compartilhamento e a combinação dos dados.

A versão com apenas parallel for tem uma corrida no contador. A versão critical protege cada incremento. As seguintes usam contadores locais e combinam os totais uma vez por thread; firstprivate inicializa a cópia local com o valor original. Default none exige declarar explicitamente o escopo das variáveis sem atributo predeterminado.

A versão lastprivate é um contraexemplo: devolve o valor associado à última iteração na ordem sequencial, e não a soma das contribuições. No relatório com dez milhões de pontos e quatro threads, ela estima 0,7855336, muito longe de π.

As versões continuam usando rand, cujo estado global pode causar contenção em implementações que sincronizam seu acesso. Corrigir o contador não garante escalabilidade.”

### Código mais relevante

Abra os sete fontes de `tarefa6/`, especialmente `2_pragma_parallel_for.c`, `4_shared_private.c`, `5_firstprivate.c`, `6_lastprivate.c` e `7_default_none.c`. Relacione as cláusulas com `tarefa6/resultados_testes.md`.

### Perguntas de aprofundamento

- **Private inicializa a variável com zero?** Não. Firstprivate copia o valor anterior; private exige inicialização antes da leitura.
- **Lastprivate escolhe a thread que termina por último?** Não. Usa a iteração logicamente final do laço sequencial.
- **Uma estimativa próxima de π prova ausência de corrida?** Não. Erro estatístico e falhas de concorrência são fenômenos distintos.
- **Mais pontos garantem erro menor em toda execução?** Não. Sob amostragem adequada, o erro típico diminui como 1/√N, mas cada realização pode variar.

## Tarefa 7 — Tarefas sobre uma lista encadeada

### Ideia central

“O programa monta uma lista de nomes de arquivos e cria uma tarefa para cada nó. O trabalho mostrado é imprimir o nome e a thread executora; não há processamento real do conteúdo dos arquivos.

Na versão inicial, todas as threads percorrem a lista e geram tarefas. Na versão corrigida, single faz uma única thread percorrer os nós e criar uma tarefa por elemento, enquanto a equipe pode executá-las.

O ponteiro no_atual é declarado dentro do percurso e sua captura pela tarefa preserva o endereço daquele nó. A lista continua compartilhada e só é liberada após a região paralela, quando as tarefas já terminaram. A ordem das mensagens pode variar.”

### Código mais relevante

Abra `tarefa7/codigo_inicial.c` e `tasks_corrigidas.c`. Compare a presença de `single`, a declaração de `no_atual`, a diretiva `task` e a posição de `liberar_lista`. O `taskwait` está comentado; o encerramento da região paralela já assegura a conclusão antes da liberação.

### Perguntas de aprofundamento

- **Single executa todas as tarefas sozinho?** Não. Seleciona quem cria as tarefas; a equipe pode executá-las.
- **Capturar o ponteiro copia o nó?** Não. Copia o endereço, e o objeto apontado precisa continuar válido.
- **As mensagens devem sair na ordem da lista?** Não. A execução das tarefas não preserva essa ordem automaticamente.
- **Vale a pena criar uma tarefa só para printf?** A sobrecarga pode dominar. Aqui o exemplo demonstra organização de tarefas.

## Tarefa 8 — Gerador aleatório e falso compartilhamento

### Ideia central

“Comparei quatro formatos de Monte Carlo combinando duas decisões: como gerar os pontos e como acumular os acertos. Os formatos 1 e 3 acumulam localmente e entram em critical uma vez por thread. Os formatos 2 e 4 incrementam uma posição de um vetor por thread e fazem a soma serial ao final.

Os formatos 1 e 2 usam rand; os formatos 3 e 4 usam rand_r com estado privado. Isso permite investigar o custo do estado global do gerador separadamente da organização dos contadores, embora mudar o gerador também mude as amostras.

No vetor de contadores, threads escrevem posições distintas, mas posições vizinhas podem ocupar a mesma linha de cache. A coerência pode provocar tráfego entre núcleos mesmo sem duas threads escreverem a mesma variável: esse é o falso compartilhamento. O efeito efetivo depende também do código gerado pelo compilador.”

### Código mais relevante

Abra `tarefa8/formato_1.c` a `formato_4.c`. Compare `rand` e `rand_r`, a semente local, o contador local seguido de critical e `acertos[tid]++`. Mostre a soma do vetor após a região paralela.

### Perguntas de aprofundamento

- **Falso compartilhamento é uma corrida de dados?** Não. Pode prejudicar desempenho mesmo com posições distintas e acesso correto.
- **Como reduzir esse efeito?** Acumular em variável local ou separar contadores em memória; qualquer ganho precisa ser medido.
- **Rand_r garante qualidade estatística?** Estado privado evita compartilhar o estado do gerador, mas não garante qualidade ou independência das sequências.
- **Por que critical pode ser competitivo aqui?** É usado apenas para combinar o total local uma vez por thread, em vez de proteger cada acerto.

## Tarefa 9 — Inserções concorrentes em listas

### Ideia central

“Criei tarefas que alocam nós e escolhem uma lista de destino. Inserir no início exige ler o início atual, ligar o novo nó e atualizar o ponteiro da lista; essa sequência precisa ser protegida quando tarefas compartilham o destino.

A versão 1 usa duas listas e regiões critical com nomes distintos, permitindo proteger cada lista separadamente. A versão 2 generaliza a quantidade de listas, mas remove a proteção: duas tarefas podem perder inserções ao atualizar o mesmo início. A versão 3 usa um lock por lista e protege novamente as duas operações da inserção.

O lock é escolhido pelo índice da lista. Listas diferentes podem ser atualizadas simultaneamente, mas tarefas destinadas à mesma lista se serializam. A alocação do nó e o sorteio ficam fora da região protegida.”

### Código mais relevante

Abra `tarefa9/versao1.c`, `versao2.c` e `versao3.c`. Compare os critical nomeados com a atualização desprotegida e com `omp_set_lock`/`omp_unset_lock`. Na versão 3, mostre inicialização e destruição dos locks, além de `firstprivate(i, semente)` nas tarefas.

### Perguntas de aprofundamento

- **Por que proteger somente a atribuição final não basta?** A ligação ao início antigo e a publicação do novo início precisam formar uma operação consistente.
- **Mais listas eliminam a corrida da versão 2?** Não. Podem reduzir colisões, mas duas tarefas ainda podem escolher a mesma lista.
- **Por que um lock por lista?** Para permitir inserções simultâneas em destinos independentes.
- **Como conferir o resultado?** Verificar que o total de nós é N e que cada identificador esperado aparece exatamente uma vez; a ordem pode variar.

## Tarefa 10 — Estratégias de acumulação

### Ideia central

“Mantive o Monte Carlo com rand_r e estado privado por thread e comparei quatro formas de contar acertos. Critical protege um bloco de incremento; atomic protege a atualização do contador; atomic local acumula privadamente e combina uma vez por thread; reduction delega ao OpenMP a criação e a combinação das cópias privadas.

Critical e atomic por acerto disputam o mesmo contador muitas vezes. A acumulação local reduz o número de atualizações compartilhadas de aproximadamente uma por acerto para uma por thread. Reduction expressa diretamente a soma das contribuições.

A tendência é diminuir contenção ao acumular localmente, mas não há uma ordem universal de desempenho sem medições. Como as sementes dependem do relógio e da thread, execuções e quantidades de threads podem gerar amostras diferentes.”

### Código mais relevante

Abra `tarefa10/critical.c`, `atomic.c`, `atomic_local.c` e `reduction.c`. Localize o incremento protegido, o contador local e `reduction(+:pontos_no_circulo)`. Compare os intervalos medidos e use a mesma quantidade de pontos em todos os programas.

### Perguntas de aprofundamento

- **Atomic e critical são equivalentes?** Ambos podem tornar este incremento seguro, mas atomic se aplica a operações específicas e critical protege um bloco mais geral.
- **Por que atomic ainda pode limitar a escala?** Todas as threads atualizam a mesma posição, exigindo coordenação e coerência de cache.
- **Reduction garante o menor tempo?** Não. Expressa a operação corretamente; o desempenho depende da implementação e da carga.
- **Como medir speedup?** Dividir o tempo de uma referência correta pelo tempo paralelo sob a mesma carga, identificando a referência e repetindo as medições.

## Tarefa 11 — Escalonamento OpenMP

### Ideia central

“Nesta tarefa comparei formas de distribuir as iterações de uma atualização de difusão viscosa em uma malha. Em cada passo, cada ponto lê seus vizinhos do campo atual e escreve em outro buffer. Os pontos podem ser calculados em paralelo dentro do passo; os passos dependem uns dos outros.

Comparei static, dynamic e guided, com e sem collapse de dois laços. Static distribui os blocos antecipadamente; dynamic entrega novos blocos conforme as threads terminam; guided começa com blocos maiores e os reduz. O chunk controla a granularidade.

Um detalhe central é que collapse muda a unidade distribuída: sem ele, a iteração externa corresponde a uma linha; com ele, corresponde a um ponto do par de laços. Assim, chunk 16 não representa a mesma quantidade de cálculo nas duas situações.

No resumo de 512², 8 threads e chunk 16, dynamic levou 0,483 segundo e dynamic com collapse levou 6,025 segundos. Isso mostra que expor mais iterações não garante melhor desempenho. A conclusão vale para essa configuração; a tabela não compara diferentes chunks.”

### Código mais relevante

Abra `tarefa11/1_static.c`, `2_dynamic.c`, `3_guided.c` e suas versões com collapse. Localize a diretiva, os dois laços, a aplicação dos contornos e a troca de ponteiros. A numeração de alguns binários e da tabela difere dos nomes atuais dos fontes: identifique as versões pela diretiva.

### Perguntas de aprofundamento

- **Dynamic é sempre melhor?** Não. Balanceamento pode compensar seu custo quando o trabalho varia; trabalho uniforme costuma favorecer distribuição de baixo custo.
- **Há corrida na escrita do campo novo?** Cada iteração escreve uma posição distinta. As leituras usam outro buffer e o laço paralelo termina antes de trocar os ponteiros.
- **Por que não atualizar no próprio campo?** Isso mudaria as dependências: algumas leituras poderiam usar valores já atualizados, alterando o método e criando conflitos.
- **Qual chunk é melhor?** O resumo disponível contém apenas 16; seria necessário comparar outros valores sob as mesmas condições.

## Tarefa 12 — Evolução das otimizações

### Ideia central

“O objetivo foi melhorar o programa e avaliar sua escalabilidade. A V1 usa collapse e copia contornos a cada passo. A V2 distribui linhas, indica SIMD no laço interno e mantém os contornos nos dois buffers. A V3 mantém uma região paralela durante os passos e também usa inicialização paralela.

A região persistente evita repetir a entrada e a saída da região paralela. Os ponteiros privados permitem que cada thread alterne os buffers localmente. Todas precisam manter a mesma alternância e concluir a atualização antes do próximo passo; por isso a barreira ao fim do omp for continua necessária.

Para 2048² pontos internos, 300 passos e 32 threads, o programa completo passou de 344,99 ms na V1 para 102,48 ms na V3. A V3 obteve speedup de 24,14 e eficiência de 75,4% sobre sua própria execução com uma thread.

O ganho global combina mudanças na evolução e na inicialização. Para problemas pequenos, acrescentar threads pode aumentar a sobrecarga sem reduzir o tempo.”

### Código mais relevante

Abra `tarefa12/navierStokesV1.c`, `navierStokesV2.c` e `navierStokesV3.c`. Na V3, explique `parallel firstprivate(u,v,un,vn)`, `for schedule(static)`, `simd` e troca de buffers. No arquivo comum, localize a inicialização e as regiões medidas.

### Perguntas de aprofundamento

- **Firstprivate copia as matrizes?** Não. Copia os valores dos ponteiros para variáveis privadas; as alocações apontadas continuam compartilhadas.
- **Por que não usar nowait entre passos?** Uma thread poderia ler vizinhos ainda não produzidos por outra no passo anterior.
- **Por que a V2 é mais rápida e tem menor speedup?** Sua execução com uma thread também melhorou; speedups de versões diferentes têm denominadores diferentes.
- **Forte versus fraca?** Forte mantém o problema fixo; fraca mantém aproximadamente o trabalho por recurso. Na fraca convencional, o ideal é tempo constante.
- **SIMD explica todo o ganho?** Não. Há várias mudanças simultâneas; faltam medições isoladas e relatório de vetorização para atribuir uma parcela exata.

## Tarefa 13 — Afinidade

### Fala-base

“Mantive o solver V3 e alterei a afinidade, para avaliar o efeito do posicionamento das threads. Usei OMP_PLACES=cores e as políticas false, true, close, spread e master.

False não fixa pelo OpenMP; true habilita fixação com distribuição dependente da implementação; close aproxima as threads; spread as distribui; master concentra no lugar da thread principal. Criar 32 threads não significa usar 32 núcleos distintos.

No job 2214968, para evolução de 2048² com 32 threads, true teve média de 86,11 ms, spread 89,32 ms, close 92,22 ms, false 344,62 ms e master cerca de 18,18 segundos. A concentração em master oferece um contraste forte com as demais.

As médias próximas de true, spread e close precisam ser avaliadas com a dispersão. Os dados não provam uma causa específica de cache ou NUMA, nem uma política universalmente melhor.”

### Código mais relevante

Abra `tarefa13/executarExperimentos.py` e `afinidade.c`: encontre as variáveis de ambiente e o diagnóstico de CPU/máscara. Mostre que `navierStokesV3.c` é mantido entre políticas. Os diagnósticos são de um programa separado, não um rastreamento do solver.

### Perguntas de aprofundamento

- **Afinidade e número de threads são a mesma coisa?** Não. Um define onde executar; o outro, quantas threads criar.
- **False significa acesso irrestrito a todas as CPUs?** Não. As restrições do sistema e do Slurm continuam valendo.
- **True equivale sempre a spread?** Não. Depende da implementação e configuração.
- **Posso comparar esse tempo com 102,48 ms da tarefa 12?** Não diretamente: aqui o exemplo mede evolução; o outro mede o programa completo, em outro experimento.
- **O primeiro acesso garante memória local?** Não. O padrão de inicialização, as páginas e o posicionamento efetivo precisam ser considerados.

## Tarefa 14 — Ping-pong MPI

### Ideia central

“O programa mede comunicação entre exatamente dois processos. O processo zero envia um buffer e o processo um o devolve. O zero mede 10.000 idas e voltas por tamanho, de 8 bytes a 1 MiB.

O tempo tem um componente aproximadamente fixo, associado à latência, e outro que cresce com o volume transferido. Mensagens pequenas ajudam a observar o primeiro; grandes também evidenciam o custo por byte.

Para 8 bytes, o CSV registra 0,19368 segundo no total. Dividindo por 10.000, obtenho 19,37 microssegundos por ida e volta. Dividir por dois produz uma estimativa por sentido, sob hipótese de simetria, não uma medição unidirecional direta.

Para interpretar o resultado, preciso identificar se os processos estão no mesmo nó ou em nós distintos. A topologia faz parte do experimento.”

### Código mais relevante

Abra `tarefa14/ping_pong.c`: verificação de dois processos, laço de tamanhos, laço de repetições, `MPI_Send`, `MPI_Recv` e `MPI_Wtime`. Em `executar.slurm`, mostre a reserva de um processo em cada um de dois nós.

### Perguntas de aprofundamento

- **Por que repetir?** Para acumular um intervalo mensurável e reduzir o peso relativo da resolução do relógio e de oscilações pontuais.
- **Qual volume circula por ida e volta?** Duas mensagens de tamanho B, totalizando 2B bytes de payload.
- **Como estimar a taxa agregada de payload?** `2*B*repetições/tempo_total`; declarar que é uma métrica de ida e volta.
- **Send bloqueante significa que o receptor já terminou?** Não. Seu retorno permite reutilizar o buffer de envio; a conclusão do recebimento remoto não é garantida por isso.

## Tarefa 15 — Halos e sobreposição

### Ideia central

“Dividi um domínio de calor 1D entre processos. Cada um armazena seu trecho e duas células fantasma, que recebem valores dos vizinhos. A atualização usa o ponto central e os dois vizinhos do passo anterior.

Comparei três organizações: comunicação bloqueante com ordenação par/ímpar; comunicação não bloqueante seguida de espera; e comunicação não bloqueante com cálculo do interior enquanto os halos chegam. Na terceira, MPI_Test é chamado a cada 100 células e as bordas são calculadas após os recebimentos terminarem.

Isend e Irecv permitem sobreposição, mas esperar antes de todo o cálculo não a aproveita. Também é preciso concluir os envios antes de reutilizar seus buffers.

Com 8 processos, um milhão de células e 500 passos, as medianas foram 0,091013 s, 0,075661 s e 0,066392 s. A sobreposição venceu nesse caso, mas o custo de testar e o progresso da comunicação podem reduzir o benefício em outros.”

### Código mais relevante

Abra `tarefa15/calorComSobreposicao.c` e `calorComum.h`. Localize publicação dos recebimentos/envios, cálculo dos índices internos, `MPI_Test`, esperas, cálculo das bordas e troca de buffers. Explique por que as extremidades globais são diferentes das fronteiras entre processos.

### Perguntas de aprofundamento

- **O que causa deadlock na versão ingênua?** Uma dependência circular de operações que não conseguem prosseguir; não se deve depender do buffering dos envios.
- **Qual cálculo pode sobrepor?** Pontos internos que não usam os halos recebidos.
- **Por que medir o maior tempo?** O processo mais lento limita a conclusão do trabalho paralelo.
- **Qual a estabilidade do esquema usado?** Para esta atualização explícita 1D, o parâmetro adimensional deve satisfazer `0 < alpha <= 0,5`.
- **Mais processos sempre ajudam?** Não. Diminuem o trabalho local e podem tornar a comunicação dominante.

## Tarefa 16 — Matriz-vetor por linhas
### Ideia central

“Calculei y=A*x distribuindo linhas completas da matriz. O processo zero inicializa os dados, Scatter distribui as linhas e Bcast entrega o vetor x inteiro a todos.

Cada processo calcula elementos completos de y, pois possui todas as colunas das suas linhas e todo x. Ao final, Gather reúne esses segmentos no processo zero; não preciso somar contribuições entre processos.

Com dois processos e quatro linhas, cada processo calcula duas posições de y. O armazenamento por linhas em C facilita distribuir esses blocos contíguos.

O intervalo medido inclui Scatter, Bcast, multiplicação e Gather, após uma barreira. Isso permite avaliar a operação distribuída completa, mas não isola o kernel de multiplicação. Nesta implementação, a quantidade de linhas precisa ser divisível pelo número de processos.”

### Código mais relevante

Abra `tarefa16/matrizVetor.c`. Explique `linhasLocais`, contagem de elementos do Scatter, Bcast, os laços locais e a contagem do Gather. O comentário menciona divisibilidade, mas o código não a valida: é uma precondição a respeitar.

### Perguntas de aprofundamento

- **Por que não dividir x também?** Cada linha completa precisa de todos os elementos de x.
- **Por que Gather e não Reduce?** Os resultados locais correspondem a posições distintas de y e já estão completos.
- **Custo de cálculo local?** Aproximadamente `M*N/p` multiplicações e somas, além da comunicação.
- **E se M não for divisível por p?** Seria necessário tratar sobras, por exemplo com Scatterv/Gatherv; este código não faz isso.
- **Y0 prova que está correto?** Não. É uma verificação parcial; uma validação completa compara todo y com uma referência.

## Tarefa 17 — Matriz-vetor por colunas

### Ideia central

“Nesta versão, distribuí blocos de colunas. Cada processo recebe algumas colunas de todas as linhas e somente o segmento correspondente de x.

Assim, cada processo contribui para todas as posições de y. Como nenhuma contribuição local contém a soma de todas as colunas, uso Reduce com MPI_SUM para obter o vetor completo no processo zero.

A matriz original é armazenada por linhas. Um bloco de colunas tem trechos separados por saltos na memória. MPI_Type_vector descreve esses trechos, e Type_create_resized ajusta o avanço usado pelo Scatter entre blocos de processos. No receptor, os elementos são recebidos de forma contígua.

A vantagem estrutural é distribuir também x; o custo inclui comunicar e somar contribuições para todo y. Não posso afirmar que essa decomposição sempre vence a divisão por linhas.”

### Código mais relevante

Abra `tarefa17/matrizVetor.c`: `MPI_Type_vector(linhas,colunasLocais,colunas,...)`, extensão `colunasLocais*sizeof(double)`, commit, Scatter da matriz, Scatter de x e Reduce. Diferencie o salto entre linhas da extensão usada para avançar entre blocos no Scatter.

### Perguntas de aprofundamento

- **Por que resized?** Para que instâncias sucessivas do tipo comecem no próximo bloco de colunas, usando a extensão ajustada.
- **Resized remove os saltos do tipo?** Não. Ajusta limites/extensão; a descrição dos elementos permanece.
- **Por que o resultado parcial tem M posições?** Cada processo tem algumas colunas de todas as M linhas.
- **Gather resolveria?** Apenas concatenaria contribuições; ainda faltaria somá-las por posição.
- **Tipo derivado evita toda cópia?** Não há essa garantia; a implementação pode empacotar internamente.

## Tarefa 18 — Soma vetorial CPU e GPU

### Ideia central

“Comparei a soma de vetores na CPU e na GPU com OpenMP. Cada índice é independente, portanto há paralelismo de dados. Na GPU, target seleciona a execução no dispositivo e loop expressa a distribuição do laço.

O programa separa inicialização, computação e teste. Nos logs arquivados, a CPU levou 0,113 s na computação e 0,673 s no total; a GPU levou 0,576 s e 1,182 s. Portanto, a GPU foi mais lenta nessa execução.

O intervalo chamado computação envolve a região target; não deve ser interpretado automaticamente como tempo exclusivo de operações aritméticas na GPU. Offload também envolve gerenciamento e potencial movimentação de dados.

O script usa NVHPC e offload obrigatório, evitando aceitar silenciosamente execução na CPU como se fosse na GPU. Como há uma execução por caso, os números não sustentam uma conclusão universal.”

### Código mais relevante

Abra `tarefa18/vadd_target.c`, `vadd_par.c` e `executar_gpu.slurm`. Localize os intervalos de tempo, target/loop, reduction de erros, flags de compilação e `OMP_TARGET_OFFLOAD=MANDATORY`. O fonte usa mapeamento implícito: não invente cláusulas map explícitas ao explicá-lo.

### Perguntas de aprofundamento

- **Por que GPU pode perder?** Custo de inicialização, movimentação e lançamento pode superar o ganho no cálculo; o experimento não separa todas essas parcelas.
- **A soma vetorial faz muito cálculo por byte?** Não. Lê duas entradas e escreve uma saída para uma soma; tende a exigir muita movimentação para pouca aritmética.
- **Reduction serve para quê aqui?** Para somar contagens de erros das threads sem disputa por um acumulador compartilhado.
- **O resultado foi validado?** O fonte tem teste, mas os logs arquivados consultados só apresentam tempos. Confirme a contagem de erros antes de afirmar que a execução GPU foi validada integralmente.

## Tarefa 19 — Stencil na GPU e profiling

### Ideia central

“Parti de uma referência serial de difusão de calor 2D. O stencil usa o ponto central e quatro vizinhos. Cada passo escreve em outro campo, e o passo seguinte depende desse resultado.

Usei target teams loop collapse(2), map de entrada e map de saída. Como os mapeamentos estão dentro da função de um passo, sem uma região externa de persistência, o programa envia e devolve campos repetidamente.

O Nsight Systems permite observar a frequência e a duração das cópias e dos kernels. Em 4096² e 100 passos, solve teve mediana de 3,774509 s. Em outra execução, com profiler, houve aproximadamente 110 ms de kernels e 3513 ms de cópias.

O perfil indica que as cópias dominam essas categorias medidas. A melhoria seguinte é manter os campos no dispositivo, em vez de otimizar somente a aritmética.”

### Código mais relevante

Abra `tarefa19/heatTarget.c`: diretiva com `map(to:u[...])` e `map(from:u_tmp[...])`, stencil, laço temporal e troca de ponteiros. No script, mostre a separação entre execuções sem profiler e com Nsight.

### Perguntas de aprofundamento

- **Collapse paraleliza os passos temporais?** Não. Combina os laços espaciais dentro de cada passo.
- **Por que map(from) na saída?** O kernel produz o campo; seu conteúdo anterior não precisa ser enviado.
- **Posso subtrair kernels e cópias da mediana?** Não como decomposição exata: foram medidos em outra execução e há outros custos.
- **Como avaliar correção?** O programa calcula erro L2 contra a solução analítica; compare também com a referência sob os mesmos parâmetros.

## Tarefa 20 — Dados persistentes e ordem dos acessos

### Ideia central

“Comparei três versões com o mesmo problema: cópias por passo, dados persistentes com a ordem original dos laços e dados persistentes com acesso contíguo.

A região target data cobre todo o laço temporal. Map to envia o campo inicial e map alloc reserva o auxiliar sem enviar seu conteúdo. Os kernels reutilizam as duas alocações já mapeadas. A CPU troca os ponteiros entre passos, mas não precisa receber os campos para isso.

No final, target update from usa o ponteiro que aponta para o campo mais recente. Isso funciona com quantidades pares e ímpares de passos. Na última versão, i passa a ser o laço interno: o índice i+j*n avança de um em um.

Para 4096² e 100 passos, a mediana caiu de 3,899758 s para 0,424280 s com persistência e 0,356260 s com a versão contígua. O ganho final foi de aproximadamente 10,95 vezes e os erros L2 impressos coincidiram. Em cargas menores, a última mudança nem sempre reduz o tempo total.”

### Código mais relevante

Abra os três fontes `heatOriginal.c`, `heatPersistente.c` e `heatContiguo.c`. Na persistente, destaque a região target data, solve, troca de ponteiros e target update. Compare a ordem i/j nos dois kernels persistentes.

### Perguntas de aprofundamento

- **Por que map alloc é suficiente para o auxiliar?** Cada passo escreve todas as suas células antes de o campo ser lido como entrada.
- **Os map dos kernels provocam as mesmas cópias da versão original?** Dentro da região de persistência, eles reutilizam os mapeamentos presentes; não forçam automaticamente envio e retorno a cada passo.
- **Como funciona a paridade?** Com buffers A/B, após um passo o resultado está em B; após dois, em A. O ponteiro u acompanha o resultado e determina o campo devolvido.
- **Trocar ponteiros na CPU transfere a matriz?** Não. Troca endereços; as alocações mapeadas continuam no dispositivo.
- **Por que ainda há 100 kernels?** Há dependência entre os passos. Esta otimização remove transferências repetidas, não a dependência temporal.
- **O que significa acesso contíguo?** Índices consecutivos alcançam elementos vizinhos na memória. Isso favorece os acessos, mas o ganho depende do mapeamento gerado pelo compilador e do dispositivo.
- **Como o volume cai?** Com P passos e doubles, de `2*P*8*n²` para `2*8*n²` bytes dos campos. Para P=100 e n=4096: 25 GiB para 256 MiB.
- **Qual limite de estabilidade 2D?** Para este stencil explícito com coeficientes iguais nas duas direções, `r=alpha*dt/dx² <= 0,25`; não confundir r com o alpha adimensional da tarefa 15.

