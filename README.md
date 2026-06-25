# Trabalho EDAII — Análise de Complexidade em Árvores de Busca

Trabalho da disciplina de Estruturas de Dados e Algoritmos II (Udesc).

O objetivo é medir empiricamente o esforço computacional das operações de **inserção** e **remoção** — incluindo rebalanceamento — nas estruturas de árvore AVL, Rubro-Negra e B, variando o tamanho do conjunto de dados de 1 a 10 000 chaves aleatórias.

---

## Estrutura do Projeto

```
Trabalho-EDAII/
├── include/
│   ├── arvores.h        # Cabeçalho principal (inclui os demais + declara contador_ops)
│   ├── avl.h            # Interface da árvore AVL
│   ├── rubro_negra.h    # Interface da árvore Rubro-Negra
│   └── arv_b.h          # Interface da árvore B
├── src/
│   ├── main.c           # Benchmark: gera chaves, mede operações, salva CSV
│   ├── avl.c            # Implementação da árvore AVL
│   ├── rubro_negra.c    # Implementação da árvore Rubro-Negra
│   └── arv_b.c          # Implementação da árvore B
├── graficos.py          # Lê os CSV e gera os gráficos de linha
├── resultado_insercao.csv
└── resultado_remocao.csv
```

---

## Como Compilar e Executar

### 1. Compilar o benchmark em C

```bash
gcc src/main.c src/avl.c src/rubro_negra.c src/arv_b.c -o benchmark -Wall -O2
```

### 2. Executar o benchmark

```bash
./benchmark
```

Isso gera os arquivos `resultado_insercao.csv` e `resultado_remocao.csv` na pasta atual.

### 3. Gerar os gráficos

```bash
pip install pandas matplotlib   # apenas na primeira vez
python graficos.py
```

Isso gera `grafico_adicao.png` e `grafico_remocao.png`.

---

## Funcionamento Geral

### Como a medição funciona

O projeto usa um **contador global de operações** declarado em `include/arvores.h`:

```c
extern long long contador_ops;
```

Todas as implementações de árvore incrementam esse contador sempre que realizam uma **comparação de chave** ou uma **rotação/divisão/fusão** de nó. Isso representa o "esforço computacional" da operação.

O fluxo do benchmark em `main.c` é:

```
Para cada tamanho n (100, 200, ..., 10000):
  Para cada amostra s (0 a 9):
    Gerar sequência aleatória de n chaves (usando rand(), semente 42)

    // Medição de inserção:
    Para cada chave na sequência:
      Zerar contador_ops
      Inserir chave na árvore
      Acumular contador_ops no total

    // Medição de remoção:
    Inserir todas as n chaves primeiro
    Para cada chave na sequência:
      Zerar contador_ops
      Remover chave da árvore
      Acumular contador_ops no total

  Calcular a média das 10 amostras para cada estrutura
  Gravar linha no CSV
```

O valor final no CSV é a **média de operações por inserção/remoção** para aquele tamanho n, calculada sobre 10 amostras independentes.

### Formato dos CSV

```
n,AVL,RB,B1,B5,B10
100,5.99,5.95,57.33,7.56,9.24
200,...
...
```

---

## Algoritmos

---

### Árvore AVL

#### O que é

Uma árvore de busca binária que mantém a propriedade: para qualquer nó, a diferença de altura entre a subárvore esquerda e a direita é no máximo 1. Isso garante que a árvore permaneça balanceada e que busca, inserção e remoção sejam O(log n).

#### Estrutura do nó

```c
typedef struct nodoAVL {
    int info;           // chave armazenada
    struct nodoAVL *esq;
    struct nodoAVL *dir;
    int alt;            // altura do nó (-1 para nó NULL)
} nodoAVL;
```

A altura é armazenada diretamente no nó para evitar recalcular em cada operação.

#### Inserção

1. Descer recursivamente pela árvore comparando a chave com cada nó, indo para a esquerda se menor, direita se maior.
2. Criar um nó folha quando encontrar posição NULL.
3. **Na volta da recursão** (unwinding), atualizar a altura de cada nó ancestral.
4. Verificar o **fator de balanceamento** (altura esquerda − altura direita). Se estiver fora de [−1, +1], aplicar rotação:

```
Caso desbalanceado à esquerda-esquerda → rotação simples direita
Caso desbalanceado à esquerda-direita  → rotação dupla (esq→dir)
Caso desbalanceado à direita-direita   → rotação simples esquerda
Caso desbalanceado à direita-esquerda  → rotação dupla (dir→esq)
```

#### Remoção

1. Localizar o nó a remover.
2. Três casos:
   - **Folha**: remover diretamente.
   - **Um filho**: substituir pelo filho.
   - **Dois filhos**: encontrar o menor da subárvore direita (sucessor in-order), copiar sua chave para o nó atual e remover o sucessor recursivamente.
3. Na volta da recursão, rebalancear com as mesmas rotações da inserção.

#### Rotações

As rotações são rearranjamentos locais de ponteiros que preservam a propriedade de busca binária enquanto reduzem o desequilíbrio. Cada rotação simples conta como 1 operação no contador; uma rotação dupla é composta por duas rotações simples (conta 2).

```
Rotação simples direita (k2 desbalanceado à esquerda):

    k2             k1
   /    →        /    \
  k1           k0     k2
 /
k0
```

#### O que é contado

- `+1` a cada comparação de chave durante a descida
- `+1` a cada rotação simples executada

---

### Árvore Rubro-Negra

#### O que é

Uma árvore de busca binária onde cada nó tem uma cor (vermelho ou preto). As propriedades de coloração garantem que o caminho mais longo da raiz até uma folha seja no máximo o dobro do menor caminho, mantendo a altura em O(log n).

#### Propriedades que devem ser sempre satisfeitas

1. Todo nó é vermelho ou preto.
2. A raiz é preta.
3. Toda folha (sentinela `nil`) é preta.
4. Se um nó é vermelho, seus dois filhos são pretos (não há dois vermelhos consecutivos no mesmo caminho).
5. Para todo nó, todos os caminhos até as folhas passam pelo mesmo número de nós pretos (altura-preta).

#### Sentinela `nil`

Em vez de usar ponteiros NULL, a implementação usa um nó especial `nil` (sempre preto) como folha universal. Isso simplifica o código dos fixups pois nunca precisamos verificar se o pai/filho é NULL — usamos `== t->nil`.

```c
typedef struct RBTree {
    RBNode *raiz;
    RBNode *nil;   // nó sentinela compartilhado por todas as "folhas"
} RBTree;
```

#### Estrutura do nó

```c
typedef struct RBNode {
    int chave;
    int cor;              // VERMELHO (1) ou PRETO (0)
    struct RBNode *esq, *dir, *pai;
} RBNode;
```

O ponteiro para o pai é necessário para que os fixups possam subir na árvore sem recursão.

#### Inserção

1. Inserir o nó como em uma BST comum (descida iterativa), colorindo-o de **vermelho**.
2. Chamar `inserir_fixup` para restaurar as propriedades RB:
   - **Caso 1 — tio vermelho**: recolorir pai, tio e avô; subir para o avô e continuar.
   - **Caso 2 — tio preto, nó é filho interno**: rotacionar o pai para transformar no Caso 3.
   - **Caso 3 — tio preto, nó é filho externo**: rotacionar o avô e trocar cores com o pai.

#### Remoção

A remoção em árvores RB é a operação mais complexa. O algoritmo:

1. Localizar o nó `z` a remover (descida iterativa contando comparações).
2. Determinar o nó `y` que será fisicamente removido:
   - Se `z` tem 0 ou 1 filho: `y = z`
   - Se `z` tem 2 filhos: `y` é o sucessor in-order (mínimo da subárvore direita)
3. Substituir `y` por seu filho `x` via `transplant`.
4. Se `y` era preto, chamar `remover_fixup(x)` para restaurar a propriedade de altura-preta, usando rotações e recolorações.

#### O que é contado

- `+1` a cada comparação de chave na descida iterativa de inserção
- `+1` a cada comparação de chave na descida iterativa de remoção
- `+1` a cada rotação (esquerda ou direita)
- Recolorações não são contadas separadamente (são O(1) e não envolvem comparação de chave)

---

### Árvore B

#### O que é

Uma árvore de busca balanceada onde cada nó pode armazenar **múltiplas chaves** e ter **múltiplos filhos**. É projetada para minimizar acessos a disco em sistemas com memória hierárquica. O parâmetro **grau mínimo `t`** controla o tamanho dos nós:

- Cada nó não-raiz tem entre `t-1` e `2t-1` chaves.
- A raiz tem entre 1 e `2t-1` chaves (exceto quando a árvore está vazia).
- Um nó com `k` chaves tem exatamente `k+1` filhos (se não for folha).

#### Estrutura do nó

```c
typedef struct BNodo {
    int     *chaves;       // array de até 2t-1 chaves
    struct BNodo **filhos; // array de até 2t ponteiros para filhos
    int      n;            // número atual de chaves
    int      folha;        // 1 se é folha, 0 se é nó interno
} BNodo;
```

Os arrays são alocados dinamicamente com tamanho fixo `2t` (chaves) e `2t+1` (filhos) na criação do nó.

#### As três ordens usadas neste trabalho

| Parâmetro | Grau mínimo t | Chaves por nó | Comportamento |
|-----------|--------------|---------------|---------------|
| B1        | 1            | 0 a 1         | Degenerado — ver nota abaixo |
| B5        | 5            | 4 a 9         | Eficiente |
| B10       | 10           | 9 a 19        | Alta capacidade por nó |

> **Nota sobre B1 (t=1):** Com grau mínimo 1, os nós têm no máximo 1 chave. A raiz fica cheia após a primeira inserção e é dividida em cada inserção subsequente, fazendo a altura crescer linearmente com n. O custo médio de inserção se torna O(n) e não O(log n). O grau mínimo útil para Árvores B é t ≥ 2.

#### Inserção — divisão proativa (top-down)

A inserção desce sempre pelo caminho correto **dividindo proativamente** qualquer nó cheio encontrado, antes de recursão:

1. Se a raiz está cheia: criar nova raiz vazia, mover raiz antiga para filho[0], dividir.
2. Chamar `inserir_nao_cheio` recursivamente:
   - Em folhas: encontrar posição por varredura linear e inserir.
   - Em nós internos: identificar o filho a descer; se esse filho está cheio, dividi-lo primeiro, depois descer.

**Divisão de nó (`dividir_filho`):**

Um nó cheio com `2t-1` chaves é dividido ao meio:
- Filho esquerdo fica com as primeiras `t-1` chaves.
- Chave mediana sobe para o pai.
- Novo filho direito recebe as últimas `t-1` chaves.

Cada divisão conta como `+1` no contador de operações.

```
Antes (t=2, nó cheio com 3 chaves):        Depois:

pai: [ ... ]                                pai: [ ... M ... ]
      |                                          /         \
   [A B C]                                   [A]           [C]
   (2t-1=3)                               (t-1=1)        (t-1=1)
                                           (mediana M sobe)
```

#### Remoção — garantia de mínimo (top-down)

Durante a descida para localizar a chave a remover, cada filho visitado precisa ter **pelo menos `t` chaves** (para que depois de remover ainda tenha o mínimo `t-1`). Se tiver menos, `garantir_minimo` é chamado antes de descer:

- **Empréstimo à esquerda**: irmão esquerdo tem ≥ t chaves → rotacionar via pai.
- **Empréstimo à direita**: irmão direito tem ≥ t chaves → rotacionar via pai.
- **Fusão**: ambos os irmãos têm `t-1` chaves → fundir filho + chave do pai + irmão em um só nó.

Cada fusão conta como `+1` no contador. Empréstimos não são contados (apenas deslocamento de ponteiros).

Ao encontrar a chave:
- **Folha**: remover diretamente.
- **Nó interno com filho esquerdo ≥ t chaves**: substituir pela chave predecessora e remover a predecessora recursivamente.
- **Nó interno com filho direito ≥ t chaves**: substituir pelo sucessor e remover o sucessor.
- **Ambos os filhos com t-1 chaves**: fundir e remover da fusão resultante.

#### O que é contado

- `+1` a cada comparação de chave na varredura linear dentro de um nó (inserção e remoção)
- `+1` a cada divisão de nó (`dividir_filho`)
- `+1` a cada fusão de nós (`fundir_filhos`)

---

## Arquivos em Detalhe

### `include/arvores.h`

Cabeçalho principal que inclui os três headers específicos e declara o contador global:

```c
extern long long contador_ops;
```

Todos os `.c` incluem este header. O `main.c` define a variável; os demais apenas usam `extern`.

### `include/avl.h`

Define a struct `nodoAVL` e declara as funções públicas da AVL. A raiz da árvore é um simples ponteiro `nodoAVL *` gerenciado pelo chamador (não há struct de árvore separada).

### `include/rubro_negra.h`

Define `RBNode` e `RBTree`. A struct `RBTree` encapsula a raiz e o nó sentinela `nil`, que é compartilhado como folha de todos os nós internos.

### `include/arv_b.h`

Define `BNodo` e `BArvore`. O campo `t` em `BArvore` determina a capacidade de todos os nós criados por aquela árvore.

---

### `src/avl.c`

| Função | Papel |
|--------|-------|
| `altura(sub)` | Retorna `sub->alt` ou `-1` se NULL |
| `simp_roda_esq(k2)` | Rotação simples que eleva o filho esquerdo; conta +1 |
| `simp_roda_dir(k2)` | Rotação simples que eleva o filho direito; conta +1 |
| `dup_roda_esq(k3)` | Rotação dupla: chama `simp_roda_dir` + `simp_roda_esq` (conta +2) |
| `dup_roda_dir(k3)` | Rotação dupla: chama `simp_roda_esq` + `simp_roda_dir` (conta +2) |
| `insereAVL(info, arv)` | Inserção recursiva com rebalanceamento na volta |
| `menorNodo(raiz)` | Retorna o nó mais à esquerda (menor da subárvore) |
| `removeAVL(raiz, x)` | Remoção recursiva com rebalanceamento na volta |
| `buscaAVL(raiz, x)` | Busca recursiva simples (sem contagem) |
| `imprimirAVL(raiz, nivel)` | Impressão em ordem (não usada no benchmark) |

A árvore é representada apenas pelo ponteiro para a raiz. `insereAVL` e `removeAVL` retornam o novo ponteiro para a raiz (que pode mudar após rotação no topo).

---

### `src/rubro_negra.c`

| Função | Papel |
|--------|-------|
| `rb_criar()` | Aloca `RBTree` e o nó `nil` (preto, com todos os ponteiros para si mesmo) |
| `rotacionar_esq(t, x)` | Rotação esquerda iterativa; conta +1 |
| `rotacionar_dir(t, x)` | Rotação direita iterativa; conta +1 |
| `inserir_fixup(t, z)` | Loop que sobe na árvore corrigindo violações após inserção |
| `rb_inserir(t, chave)` | Busca posição (contando comparações), insere vermelho, chama fixup |
| `transplant(t, u, v)` | Substitui subárvore enraizada em `u` por `v` (auxiliar de remoção) |
| `minimo(t, x)` | Retorna o menor nó da subárvore (sucessor in-order) |
| `remover_fixup(t, x)` | Loop que corrige violações de altura-preta após remoção |
| `rb_remover(t, chave)` | Busca nó (contando), remove via transplant, chama fixup se necessário |
| `rb_buscar(t, chave)` | Busca iterativa (sem contagem) |
| `liberar_nos(t, x)` | Liberação recursiva de memória |
| `rb_destruir(t)` | Libera todos os nós e a struct `RBTree` |

---

### `src/arv_b.c`

| Função | Papel |
|--------|-------|
| `bn_novo(t, folha)` | Aloca nó com arrays de chaves (`2t`) e filhos (`2t+1`) |
| `bn_liberar(x)` | Libera os arrays e o nó |
| `b_criar(t)` | Cria árvore B com raiz folha vazia e grau mínimo `t` |
| `dividir_filho(bt, x, i)` | Divide filho cheio `x->filhos[i]`; mediana sobe para `x`; conta +1 |
| `inserir_nao_cheio(bt, x, chave)` | Insere em nó não-cheio, descendo e dividindo filhos cheios proativamente |
| `b_inserir(bt, chave)` | Se raiz cheia: divide; depois chama `inserir_nao_cheio` |
| `chave_max(x)` | Maior chave da subárvore (vai sempre para o filho mais à direita) |
| `chave_min(x)` | Menor chave da subárvore (vai sempre para o filho mais à esquerda) |
| `fundir_filhos(bt, x, i)` | Funde filhos `i` e `i+1` com a chave `x->chaves[i]` do pai; conta +1 |
| `emprestar_esq(bt, x, i)` | Rotaciona chave do irmão esquerdo via pai para o filho `i` |
| `emprestar_dir(bt, x, i)` | Rotaciona chave do irmão direito via pai para o filho `i` |
| `garantir_minimo(bt, x, i)` | Garante que `x->filhos[i]` tenha ≥ t chaves antes de descer |
| `remover_rec(bt, x, chave)` | Remoção recursiva com garantia de mínimo na descida |
| `b_remover(bt, chave)` | Chama `remover_rec`; colapsa raiz vazia se necessário |
| `b_buscar(bt, x, chave)` | Busca recursiva (sem contagem) |
| `liberar_nos(x)` | Liberação recursiva pós-ordem |
| `b_destruir(bt)` | Libera toda a árvore e a struct `BArvore` |

---

### `src/main.c`

Responsável pela geração de dados e medição. Parâmetros definidos via `#define`:

```c
#define MAX_N    10000   // tamanho máximo do conjunto
#define PASSO    100     // incremento: testa n = 100, 200, ..., 10000
#define AMOSTRAS 10      // amostras independentes por tamanho n
```

**Geração de chaves:**

```c
static int chaves[AMOSTRAS][MAX_N];

static void gerar_chaves(void) {
    srand(42);  // semente fixa para reprodutibilidade
    for (int s = 0; s < AMOSTRAS; s++)
        for (int i = 0; i < MAX_N; i++)
            chaves[s][i] = rand();  // chaves aleatórias no intervalo [0, RAND_MAX]
}
```

As 10 × 10 000 chaves são geradas uma única vez antes de qualquer medição.

**Funções de medição:**

Cada `medir_*_insercao(amostra, n)` e `medir_*_remocao(amostra, n)` retorna a **média de operações por elemento** para aquela amostra e tamanho n:

```
retorno = (soma de contador_ops de cada operação individual) / n
```

**Loop principal:**

```
Para n = 100 até 10000 (passo 100):
  Para s = 0 até 9:
    soma_ins[AVL] += medir_avl_insercao(s, n)
    soma_ins[RB]  += medir_rb_insercao(s, n)
    soma_ins[B1]  += medir_b_insercao(s, n, t=1)
    soma_ins[B5]  += medir_b_insercao(s, n, t=5)
    soma_ins[B10] += medir_b_insercao(s, n, t=10)
    (idem para remoção)

  Gravar médias das 10 amostras no CSV
```

---

### `graficos.py`

Script Python independente que lê os CSV gerados pelo benchmark e produz dois gráficos:

- **`grafico_adicao.png`**: esforço computacional de inserção × tamanho n, com 5 linhas (AVL, RB, B1, B5, B10).
- **`grafico_remocao.png`**: mesmo formato para remoção.

Usa `pandas` para leitura do CSV e `matplotlib` para plotagem. Os gráficos são salvos em 300 DPI e também exibidos em janela interativa.

---

## Complexidade Esperada

| Estrutura | Inserção | Remoção | Observação |
|-----------|----------|---------|------------|
| AVL | O(log n) | O(log n) | ≤ 2 rotações por inserção, ≤ O(log n) por remoção |
| Rubro-Negra | O(log n) | O(log n) | ≤ 2 rotações por inserção, ≤ 3 por remoção |
| Árvore B (t≥2) | O(t · log\_t n) | O(t · log\_t n) | = O(log n); t maior → menos níveis, mais trabalho por nível |
| Árvore B (t=1) | O(n) na prática | — | Caso degenerado: raiz sempre cheia, altura cresce linearmente |
