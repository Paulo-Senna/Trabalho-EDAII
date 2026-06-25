#include <stdio.h>
#include <stdlib.h>
#include "../include/arv_b.h"
#include "../include/arvores.h"

/* ---------- alocacao interna ---------- */

static BNodo *bn_novo(int t, int folha) {
    BNodo *x  = (BNodo *)malloc(sizeof(BNodo));
    x->chaves = (int *)malloc(2 * t * sizeof(int));
    x->filhos = (BNodo **)malloc((2 * t + 1) * sizeof(BNodo *));
    x->n      = 0;
    x->folha  = folha;
    return x;
}

static void bn_liberar(BNodo *x) {
    free(x->chaves);
    free(x->filhos);
    free(x);
}

/* ---------- criacao ---------- */

BArvore *b_criar(int t) {
    if (t < 1) t = 1;
    BArvore *bt = (BArvore *)malloc(sizeof(BArvore));
    bt->t       = t;
    bt->raiz    = bn_novo(t, 1);
    return bt;
}

/* ---------- insercao ---------- */

static void dividir_filho(BArvore *bt, BNodo *x, int i) {
    contador_ops++;                          /* split */
    int t    = bt->t;
    BNodo *y = x->filhos[i];
    BNodo *z = bn_novo(t, y->folha);
    int mid  = (y->n - 1) / 2;

    z->n = y->n - mid - 1;
    for (int j = 0; j < z->n; j++)      z->chaves[j] = y->chaves[mid + 1 + j];
    if (!y->folha)
        for (int j = 0; j <= z->n; j++) z->filhos[j] = y->filhos[mid + 1 + j];
    int mediana = y->chaves[mid];
    y->n = mid;

    for (int j = x->n; j > i; j--)       x->filhos[j + 1] = x->filhos[j];
    x->filhos[i + 1] = z;
    for (int j = x->n - 1; j >= i; j--) x->chaves[j + 1]  = x->chaves[j];
    x->chaves[i] = mediana;
    x->n++;
}

static void inserir_nao_cheio(BArvore *bt, BNodo *x, int chave) {
    int i = x->n - 1;
    if (x->folha) {
        while (i >= 0 && chave < x->chaves[i]) {
            contador_ops++;                  /* comparacao de chave */
            x->chaves[i + 1] = x->chaves[i];
            i--;
        }
        contador_ops++;                      /* comparacao que encerrou o while */
        x->chaves[i + 1] = chave;
        x->n++;
        return;
    }
    while (i >= 0 && chave < x->chaves[i]) {
        contador_ops++;                      /* comparacao de chave */
        i--;
    }
    contador_ops++;
    i++;
    if (x->filhos[i]->n >= 2 * bt->t - 1) {
        dividir_filho(bt, x, i);
        if (chave > x->chaves[i]) i++;
    }
    inserir_nao_cheio(bt, x->filhos[i], chave);
}

void b_inserir(BArvore *bt, int chave) {
    BNodo *r = bt->raiz;
    if (r->n >= 2 * bt->t - 1) {
        BNodo *s     = bn_novo(bt->t, 0);
        bt->raiz     = s;
        s->filhos[0] = r;
        dividir_filho(bt, s, 0);
        inserir_nao_cheio(bt, s, chave);
    } else {
        inserir_nao_cheio(bt, r, chave);
    }
}

/* ---------- remocao ---------- */

static int chave_max(BNodo *x) {
    while (!x->folha) x = x->filhos[x->n];
    return x->chaves[x->n - 1];
}

static int chave_min(BNodo *x) {
    while (!x->folha) x = x->filhos[0];
    return x->chaves[0];
}

static void fundir_filhos(BArvore *bt, BNodo *x, int i) {
    contador_ops++;                          /* merge */
    BNodo *L = x->filhos[i];
    BNodo *R = x->filhos[i + 1];

    L->chaves[L->n] = x->chaves[i];
    int base = L->n + 1;
    for (int j = 0; j < R->n; j++)      L->chaves[base + j] = R->chaves[j];
    if (!L->folha)
        for (int j = 0; j <= R->n; j++) L->filhos[base + j] = R->filhos[j];
    L->n = L->n + 1 + R->n;

    for (int j = i + 1; j < x->n; j++) {
        x->chaves[j - 1] = x->chaves[j];
        x->filhos[j]      = x->filhos[j + 1];
    }
    x->n--;
    bn_liberar(R);
}

static void emprestar_esq(BArvore *bt, BNodo *x, int i) {
    BNodo *filho = x->filhos[i];
    BNodo *irmao = x->filhos[i - 1];

    if (!filho->folha) filho->filhos[filho->n + 1] = filho->filhos[filho->n];
    for (int j = filho->n - 1; j >= 0; j--) {
        filho->chaves[j + 1] = filho->chaves[j];
        if (!filho->folha) filho->filhos[j + 1] = filho->filhos[j];
    }
    filho->chaves[0]  = x->chaves[i - 1];
    if (!filho->folha) filho->filhos[0] = irmao->filhos[irmao->n];
    x->chaves[i - 1] = irmao->chaves[irmao->n - 1];
    filho->n++;
    irmao->n--;
}

static void emprestar_dir(BArvore *bt, BNodo *x, int i) {
    BNodo *filho = x->filhos[i];
    BNodo *irmao = x->filhos[i + 1];

    filho->chaves[filho->n] = x->chaves[i];
    if (!filho->folha) filho->filhos[filho->n + 1] = irmao->filhos[0];
    x->chaves[i] = irmao->chaves[0];
    for (int j = 1; j < irmao->n; j++) {
        irmao->chaves[j - 1] = irmao->chaves[j];
        if (!irmao->folha) irmao->filhos[j - 1] = irmao->filhos[j];
    }
    if (!irmao->folha) irmao->filhos[irmao->n - 1] = irmao->filhos[irmao->n];
    filho->n++;
    irmao->n--;
}

static void garantir_minimo(BArvore *bt, BNodo *x, int i) {
    int t = bt->t;
    if (x->filhos[i]->n >= t) return;
    if (i > 0 && x->filhos[i - 1]->n >= t)         emprestar_esq(bt, x, i);
    else if (i < x->n && x->filhos[i + 1]->n >= t) emprestar_dir(bt, x, i);
    else {
        if (i < x->n) fundir_filhos(bt, x, i);
        else          fundir_filhos(bt, x, i - 1);
    }
}

static void remover_rec(BArvore *bt, BNodo *x, int chave) {
    int t = bt->t;
    int i = 0;
    while (i < x->n && chave > x->chaves[i]) {
        contador_ops++;                      /* comparacao de chave */
        i++;
    }
    contador_ops++;                          /* comparacao que encerrou o while */

    if (i < x->n && x->chaves[i] == chave) {
        if (x->folha) {
            for (int j = i + 1; j < x->n; j++) x->chaves[j - 1] = x->chaves[j];
            x->n--;
        } else if (x->filhos[i]->n >= t) {
            int pred       = chave_max(x->filhos[i]);
            x->chaves[i]  = pred;
            remover_rec(bt, x->filhos[i], pred);
        } else if (x->filhos[i + 1]->n >= t) {
            int suc        = chave_min(x->filhos[i + 1]);
            x->chaves[i]  = suc;
            remover_rec(bt, x->filhos[i + 1], suc);
        } else {
            fundir_filhos(bt, x, i);
            remover_rec(bt, x->filhos[i], chave);
        }
    } else {
        if (x->folha) return;
        int ultimo = (i == x->n);
        garantir_minimo(bt, x, i);
        if (ultimo && i > x->n) i--;
        remover_rec(bt, x->filhos[i], chave);
    }
}

void b_remover(BArvore *bt, int chave) {
    if (!bt->raiz) return;
    if (bt->raiz->n == 0 && bt->raiz->folha) return;   /* arvore vazia */
    remover_rec(bt, bt->raiz, chave);
    /* colapsa raizes vazias ate ter uma com chaves ou ser folha */
    while (bt->raiz->n == 0 && !bt->raiz->folha) {
        BNodo *antigo = bt->raiz;
        bt->raiz      = antigo->filhos[0];
        bn_liberar(antigo);
    }
}

/* ---------- busca ---------- */

BNodo *b_buscar(BArvore *bt, BNodo *x, int chave) {
    int i = 0;
    while (i < x->n && chave > x->chaves[i]) i++;
    if (i < x->n && chave == x->chaves[i]) return x;
    if (x->folha) return NULL;
    return b_buscar(bt, x->filhos[i], chave);
}

/* ---------- impressao ---------- */

void b_imprimir(BNodo *x, int nivel) {
    if (x == NULL) return;
    printf("\n");
    for (int i = 0; i < nivel; i++) printf("  ");
    printf("[");
    for (int i = 0; i < x->n; i++) { if (i) printf(" "); printf("%d", x->chaves[i]); }
    printf("]");
    if (!x->folha)
        for (int i = 0; i <= x->n; i++) b_imprimir(x->filhos[i], nivel + 1);
}

/* ---------- liberacao ---------- */

static void liberar_nos(BNodo *x) {
    if (!x->folha)
        for (int i = 0; i <= x->n; i++) liberar_nos(x->filhos[i]);
    bn_liberar(x);
}

void b_destruir(BArvore *bt) {
    if (bt->raiz) liberar_nos(bt->raiz);
    free(bt);
}
