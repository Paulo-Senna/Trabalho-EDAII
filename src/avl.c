#include <stdio.h>
#include <stdlib.h>
#include "../include/avl.h"
#include "../include/arvores.h"

/* ----- auxiliares ----- */

int max(int a, int b) { return (a >= b) ? a : b; }

int altura(nodoAVL *sub) {
    return (sub == NULL) ? -1 : sub->alt;
}

/* ----- rotacoes (cada chamada = 1 op) ----- */

nodoAVL *simp_roda_esq(nodoAVL *k2) {
    contador_ops++;                          /* rotacao simples */
    nodoAVL *k1 = k2->esq;
    k2->esq = k1->dir;
    k1->dir = k2;
    k2->alt = max(altura(k2->esq), altura(k2->dir)) + 1;
    k1->alt = max(altura(k1->esq), k2->alt) + 1;
    return k1;
}

nodoAVL *simp_roda_dir(nodoAVL *k2) {
    contador_ops++;                          /* rotacao simples */
    nodoAVL *k1 = k2->dir;
    k2->dir = k1->esq;
    k1->esq = k2;
    k2->alt = max(altura(k2->esq), altura(k2->dir)) + 1;
    k1->alt = max(altura(k1->dir), k2->alt) + 1;
    return k1;
}

nodoAVL *dup_roda_esq(nodoAVL *k3) {
    /* internamente chama simp_roda_dir + simp_roda_esq, cada uma ja conta */
    k3->esq = simp_roda_dir(k3->esq);
    return simp_roda_esq(k3);
}

nodoAVL *dup_roda_dir(nodoAVL *k3) {
    k3->dir = simp_roda_esq(k3->dir);
    return simp_roda_dir(k3);
}

/* ----- insercao ----- */

nodoAVL *insereAVL(int info, nodoAVL *arv) {
    if (arv == NULL) {
        arv        = (nodoAVL *)malloc(sizeof(nodoAVL));
        arv->info  = info;
        arv->esq   = NULL;
        arv->dir   = NULL;
        arv->alt   = 0;
    } else if (info < arv->info) {           /* comparacao de chave */
        contador_ops++;
        arv->esq = insereAVL(info, arv->esq);
        if ((altura(arv->esq) - altura(arv->dir)) > 1) {
            if (info < arv->esq->info)       /* comparacao para decidir rotacao */
                arv = simp_roda_esq(arv);
            else
                arv = dup_roda_esq(arv);
        }
    } else if (info > arv->info) {           /* comparacao de chave */
        contador_ops++;
        arv->dir = insereAVL(info, arv->dir);
        if ((altura(arv->dir) - altura(arv->esq)) > 1) {
            if (info > arv->dir->info)
                arv = simp_roda_dir(arv);
            else
                arv = dup_roda_dir(arv);
        }
    }
    arv->alt = max(altura(arv->esq), altura(arv->dir)) + 1;
    return arv;
}

/* ----- remocao ----- */

nodoAVL *menorNodo(nodoAVL *raiz) {
    while (raiz->esq != NULL) { contador_ops++; raiz = raiz->esq; }
    return raiz;
}

nodoAVL *removeAVL(nodoAVL *raiz, int x) {
    if (raiz == NULL) return NULL;

    contador_ops++;                          /* comparacao de chave */
    if (x < raiz->info) {
        raiz->esq = removeAVL(raiz->esq, x);
    } else if (x > raiz->info) {
        raiz->dir = removeAVL(raiz->dir, x);
    } else {
        if (raiz->esq == NULL && raiz->dir == NULL) {
            free(raiz); return NULL;
        } else if (raiz->esq == NULL) {
            nodoAVL *temp = raiz->dir; free(raiz); return temp;
        } else if (raiz->dir == NULL) {
            nodoAVL *temp = raiz->esq; free(raiz); return temp;
        } else {
            nodoAVL *temp = menorNodo(raiz->dir);
            raiz->info    = temp->info;
            raiz->dir     = removeAVL(raiz->dir, temp->info);
        }
    }

    raiz->alt = max(altura(raiz->esq), altura(raiz->dir)) + 1;
    int bal   = altura(raiz->esq) - altura(raiz->dir);

    if (bal > 1) {
        if (altura(raiz->esq->esq) >= altura(raiz->esq->dir))
            return simp_roda_esq(raiz);
        else
            return dup_roda_esq(raiz);
    }
    if (bal < -1) {
        if (altura(raiz->dir->dir) >= altura(raiz->dir->esq))
            return simp_roda_dir(raiz);
        else
            return dup_roda_dir(raiz);
    }
    return raiz;
}

/* ----- busca ----- */

nodoAVL *buscaAVL(nodoAVL *raiz, int x) {
    if (raiz == NULL)    return NULL;
    if (raiz->info == x) return raiz;
    if (x < raiz->info)  return buscaAVL(raiz->esq, x);
    return buscaAVL(raiz->dir, x);
}

/* ----- impressao ----- */

void imprimirAVL(nodoAVL *raiz, int nivel) {
    if (raiz == NULL) return;
    imprimirAVL(raiz->dir, nivel + 1);
    printf("\n");
    for (int i = 0; i < nivel; i++) printf("\t");
    printf("%d", raiz->info);
    imprimirAVL(raiz->esq, nivel + 1);
}
