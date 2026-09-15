# Tarefa 11 - Simulação de Navier-Stokes com Foco em Efeitos Viscosos

**Disciplina:** Programação Paralela  
**Instituição:** Universidade Federal do Rio Grande do Norte (UFRN)  

---

## 1. Descrição do Problema

O objetivo desta tarefa é implementar uma simulação computacional da evolução temporal de um campo de velocidade bidimensional de um fluido governado pela equação de Navier-Stokes, isolando estritamente os **efeitos da viscosidade**.

Conforme os requisitos da especificação:
1. **Desconsiderar** o gradiente de pressão ($-\nabla p = 0$).
2. **Desconsiderar** forças externas de campo ($\mathbf{f} = 0$, como gravidade).
3. Utilizar o método de **diferenças finitas** para a discretização espacial e temporal.
4. Inicializar o fluido em **repouso** ou com **velocidade constante** e comprovar numericamente a estabilidade do campo.
5. Injetar uma **pequena perturbação** localizada e verificar se ela se **difunde suavemente** pelo domínio ao longo do tempo.
6. Implementar a solução em C com aceleração paralela usando **OpenMP**.

---

## 2. Formulação Matemática

A equação de conservação de momento linear de Navier-Stokes para um fluido incompressível é:

$$\frac{\partial \mathbf{u}}{\partial t} + (\mathbf{u} \cdot \nabla)\mathbf{u} = -\frac{1}{\rho}\nabla p + \nu \nabla^2 \mathbf{u} + \mathbf{f}$$

Onde:
- $\mathbf{u} = (u, v)$ é o vetor velocidade do fluido;
- $p$ é a pressão estática;
- $\rho$ é a massa específica;
- $\nu = \frac{\mu}{\rho}$ é a viscosidade cinemática do fluido;
- $\mathbf{f}$ representa o termo de forças externas (por exemplo, gravitacional).

Ao isolar exclusivamente a ação viscosa e descartar pressão, forças de corpo e termos advectivos, a equação se reduz à clássica **equação de difusão viscosa**:

$$\frac{\partial \mathbf{u}}{\partial t} = \nu \nabla^2 \mathbf{u}$$

Em coordenadas cartesianas bidimensionais $(x, y)$, a evolução desacopla-se nas duas componentes escalares da velocidade:

$$\frac{\partial u}{\partial t} = \nu \left( \frac{\partial^2 u}{\partial x^2} + \frac{\partial^2 u}{\partial y^2} \right)$$

$$\frac{\partial v}{\partial t} = \nu \left( \frac{\partial^2 v}{\partial x^2} + \frac{\partial^2 v}{\partial y^2} \right)$$

### Propriedades Analíticas Fundamentais
1. **Estabilidade de Campos Homogêneos:**  
   Se o campo inicial for uniforme ($u(x, y, 0) = U_0$), temos $\frac{\partial^2 u}{\partial x^2} = 0$ e $\frac{\partial^2 u}{\partial y^2} = 0$. Logo, $\frac{\partial u}{\partial t} = 0$. O campo permanece rigorosamente inalterado ao longo do tempo.
2. **Difusão Suave de Perturbações:**  
   Para uma perturbação pontual ou gaussiana, o operador Laplaciano $\nabla^2 u$ é negativo no pico (máximo local) e positivo nas regiões vizinhas adjacentes. Isso produz um fluxo líquido de velocidade das áreas de alta velocidade para as de baixa velocidade, alargando o perfil espacial e amortecendo suavemente o pico ao longo do tempo (dissipação viscosa irreversível).

---

## 3. Discretização Numérica (Diferenças Finitas - FTCS)

O domínio espacial bidimensional $[0, L_x] \times [0, L_y]$ é discretizado em uma malha uniforme com $N_x \times N_y$ pontos, onde:
$$\Delta x = \frac{L_x}{N_x - 1}, \quad \Delta y = \frac{L_y}{N_y - 1}$$

Aproximamos as derivadas espaciais de 2ª ordem através de **diferenças centrais** de ordem $\mathcal{O}(\Delta x^2, \Delta y^2)$:
$$\left.\frac{\partial^2 u}{\partial x^2}\right|_{i,j}^n \approx \frac{u_{i+1, j}^n - 2u_{i, j}^n + u_{i-1, j}^n}{\Delta x^2}$$

$$\left.\frac{\partial^2 u}{\partial y^2}\right|_{i,j}^n \approx \frac{u_{i, j+1}^n - 2u_{i, j}^n + u_{i, j-1}^n}{\Delta y^2}$$

A derivada temporal é aproximada por **diferença avançada** (Euler Explícito, ordem $\mathcal{O}(\Delta t)$):
$$\left.\frac{\partial u}{\partial t}\right|_{i,j}^n \approx \frac{u_{i, j}^{n+1} - u_{i, j}^n}{\Delta t}$$

Substituindo na equação governante, obtemos a relação de recorrência explícita (FTCS - *Forward-Time Central-Space*):

$$u_{i, j}^{n+1} = u_{i, j}^n + \nu \Delta t \left[ \frac{u_{i+1, j}^n - 2u_{i, j}^n + u_{i-1, j}^n}{\Delta x^2} + \frac{u_{i, j+1}^n - 2u_{i, j}^n + u_{i, j-1}^n}{\Delta y^2} \right]$$

### Condição de Estabilidade Numérica (Critério de Von Neumann / CFL Difusivo)
Para que o método numérico explícito seja numericamente estável e não amplifique erros de truncamento, os números de Fourier da difusão $\sigma_x$ e $\sigma_y$ devem satisfazer:

$$\sigma_x + \sigma_y = \nu \Delta t \left( \frac{1}{\Delta x^2} + \frac{1}{\Delta y^2} \right) \le \frac{1}{2}$$

Se $\Delta x = \Delta y = h$:
$$\Delta t \le \frac{h^2}{4\nu}$$

O código calcula dinamicamente o passo máximo estável e aplica um fator de segurança:
$$\Delta t = 0.8 \times \frac{h^2}{4\nu}$$

---

## 4. Implementação e Paralelismo com OpenMP

O código-fonte em C (`simulacao_viscosidade.c`) adota os seguintes padrões de alto desempenho e clareza didática:

1. **Alocação de Memória Contígua (Row-Major):**  
   Os campos 2D de tamanho $N_x \times N_y$ são mapeados linearmente como vetores 1D indexados via macro `IDX(i, j, ny) = (i * ny + j)`. Essa organização favorece a localidade espacial nos caches L1d/L2 da CPU e permite vetorização automática (AVX2/AVX-512).
2. **Double-Buffering com Troca de Ponteiros:**  
   Em vez de copiar todo o conteúdo da malha de $n+1$ para $n$ ao final de cada passo (o que consumiria largura de banda de memória desnecessária), os ponteiros `u_atual` e `u_prox` são simplesmente alternados em tempo $\mathcal{O}(1)$.
3. **Paralelização dos Laços de Atualização Espacial:**  
   O avanço dos pontos internos da grade é distribuído entre as threads OpenMP com:
   ```c
   #pragma omp parallel for collapse(2) schedule(static)
   for (int i = 1; i < nx - 1; i++) {
       for (int j = 1; j < ny - 1; j++) {
           /* Calculo do Laplaciano e atualizacao FTCS */
       }
   }
   ```
4. **Reduções Paralelas Sem Locks:**  
   O cálculo das métricas de monitoramento (pico de velocidade, desvio máximo e energia cinética) utiliza cláusulas `reduction(max: ...)`, `reduction(min: ...)` e `reduction(+: ...)` do OpenMP, evitando contenção de sincronização.
5. **Renderizador Visual em Tempo Real no Terminal (ASCII Art Colorido):**  
   O programa gera mapas de calor 2D no console usando caracteres de densidade (`" .:-=+*#%@"`) e sequências de escape ANSI com gradientes de cores (azul, ciano, verde, amarelo e vermelho), permitindo acompanhar visualmente a difusão da velocidade.

---

## 5. Estrutura dos Arquivos

| Arquivo | Descrição |
| :--- | :--- |
| `simulacao_viscosidade.c` | Implementação principal em C17 com OpenMP, cenários de teste, renderizador ASCII e exportador de dados. |
| `Makefile` | Automação de compilação com `-O3 -fopenmp -Wall` e alvos rápidos (`cenario1`, `cenario2`, `cenario3`, `cenario4`, `clean`). |
| `visualizar.py` | Script auxiliar em Python com Matplotlib para geração de mapas de contorno, perfis de corte e curvas de decaimento temporal. |
| `README.md` | Documentação técnica e relatório de resultados da tarefa. |

---

## 6. Como Compilar e Executar

### Compilação
Basta utilizar o `make` ou o `gcc`:
```bash
make
# ou diretamente:
gcc -std=c17 -O3 -fopenmp -Wall -Wextra -pedantic simulacao_viscosidade.c -o simulacao_viscosidade -lm
```

### Execução dos Cenários Específicos

#### 1. Fluido em Repouso (Verificação de Estabilidade)
```bash
./simulacao_viscosidade 1 64 300 4
# ou via Makefile:
make cenario1
```

#### 2. Fluido com Velocidade Uniforme (Verificação de Estabilidade)
```bash
./simulacao_viscosidade 2 64 300 4
# ou via Makefile:
make cenario2
```

#### 3. Difusão de Perturbação Localizada
```bash
./simulacao_viscosidade 3 64 300 4
# ou via Makefile:
make cenario3
```

#### 4. Suíte Completa com Benchmark OpenMP
```bash
./simulacao_viscosidade 4 64 300 4
# ou via Makefile:
make run
```

### Argumentos da Linha de Comando
```bash
./simulacao_viscosidade [cenario] [tamanho_malha_N] [passos_tempo] [num_threads]
```
- `cenario`: `1` (repouso), `2` (velocidade constante), `3` (perturbação gaussiana), `4` (suíte completa).
- `tamanho_malha_N`: Dimensão da malha $N \times N$ (ex: 64, 128, 256).
- `passos_tempo`: Quantidade de passos temporais da simulação (ex: 300, 500).
- `num_threads`: Quantidade de threads do OpenMP (ex: 1, 2, 4, 8).

---

## 7. Análise dos Resultados Obtidos

### Cenário 1: Fluido em Repouso
- **Condição Inicial:** $u(x, y, 0) = 0.0$ m/s, $v(x, y, 0) = 0.0$ m/s em todo o domínio.
- **Resultado:**
  - Desvio máximo após 300 passos: `0.0000000000e+00 m/s`
  - Energia cinética: `0.0000000000e+00 J/kg`
- **Conclusão:** O campo em repouso é perfeitamente estável. Não há geração de velocidade espúria ou oscilações numéricas.

### Cenário 2: Fluido com Velocidade Constante
- **Condição Inicial:** $u(x, y, 0) = 1.5$ m/s, $v(x, y, 0) = 0.5$ m/s em todo o domínio.
- **Resultado:**
  - Velocidade máxima após 300 passos: `1.5000000000 m/s`
  - Desvio máximo relativo ao estado inicial: `0.0000000000e+00 m/s`
  - Energia cinética: `1.250000 J/kg`
- **Conclusão:** Como o gradiente e a curvatura espacial são nulos ($\nabla^2 \mathbf{u} = 0$), o termo de difusão é rigorosamente nulo. O fluido translada uniformemente sem sofrer qualquer amortecimento artificial ou deriva numérica.

### Cenário 3: Perturbação Localizada e Difusão Suave
- **Condição Inicial:** Pulso de velocidade gaussiano centrado em $(x_c, y_c) = (0.5, 0.5)$ com amplitude máxima de $2.0$ m/s e largura $\sigma = 0.08$.
- **Evolução Temporal Observada:**
  - **Passo 0 (t = 0.0000 s):** Pico máximo = **1.9804 m/s** (altamente concentrado no centro).
  - **Passo 75 (t = 0.0756 s):** Pico máximo = **0.9115 m/s** (redução de 53.97%).
  - **Passo 150 (t = 0.1512 s):** Pico máximo = **0.5924 m/s** (redução de 70.09%).
  - **Passo 225 (t = 0.2268 s):** Pico máximo = **0.4388 m/s** (redução de 77.84%).
  - **Passo 300 (t = 0.3023 s):** Pico máximo = **0.3485 m/s** (redução total de 82.40%).
- **Comportamento Espacial:**
  - A velocidade do centro é transferida suave e continuamente para as camadas vizinhas.
  - As curvas de perfil mantêm simetria radial e continuidade de derivadas (ausência de bordas serrilhadas ou instabilidades espúrias de Gibbs).
  - O decaimento do pico é monotonicamente decrescente em todos os passos, comprovando experimentalmente a estabilidade incondicional sob a restrição CFL de Von Neumann adotada.

---

## 8. Geração de Gráficos (Opcional)

Após executar o cenário 3 ou 4, o arquivo `difusao_simulacao.csv` é gerado automaticamente. Para gerar as figuras em alta resolução:
```bash
python3 visualizar.py difusao_simulacao.csv evolucao_difusao.png
```
O script gera uma imagem consolidada contendo:
1. Perfis 1D de velocidade no corte horizontal central ($y \approx 0.5$) para diferentes instantes;
2. Curvas de decaimento do pico máximo e dissipação da energia cinética média;
3. Mapas 2D de contorno de velocidade demonstrando a evolução espacial da difusão.
