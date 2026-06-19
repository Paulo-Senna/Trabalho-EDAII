#include <stdio.h>
#include <stdlib.h>

typedef struct nodoAVL{
    int info;
    struct nodoAVL *esq;
    struct nodoAVL *dir;
    int alt;
} nodoAVL;

int max(int a, int b){
    
	if(a >= b){
        return a;
    }else{
        return b;
    }
}

int altura(nodoAVL *sub){
    
	if(subarvore == NULL){
        return -1;
    }else{
        return sub->alt;
    }
}

nodoAVL *simp_roda_esq(nodoAVL *k2){
    
	nodoAVL *k1;

    k1 = k2->esq;
    k2->esq = k1->dir;
    k1->dir = k2;

    k2->alt = max(altura(k2->esq), altura(k2->dir)) + 1;
    k1->alt = max(altura(k1->esq), k2->alt) + 1;

    return k1;
}

nodoAVL *simp_roda_dir(nodoAVL *k2){

    nodoAVL *k1;

    k1 = k2->dir;
    k2->dir = k1->esq;
    k1->esq = k2;

    k2->alt = max(altura(k2->esq), altura(k2->dir)) + 1;
    k1->alt = max(altura(k1->dir), k2->alt) + 1;

    return k1;
}

nodoAVL *dup_roda_esq(nodoAVL *k3){

    k3->esq = simp_roda_dir(k3->esq);

    return simp_roda_esq(k3);
}

nodoAVL *dup_roda_dir(nodoAVL *k3){

    k3->dir = simp_roda_esq(k3->dir);

    return simp_roda_dir(k3);
}

nodoAVL *insereAVL(int info, nodoAVL *arv){

    if(arv == NULL){

        arv = (nodoAVL*)malloc(sizeof(nodoAVL));

        arv->info = info;
        arv->esq = NULL;
        arv->dir = NULL;
        arv->alt = 0;
    }

    else if(info < arv->info){

        arv->esq = insereAVL(info, arv->esq);

        if((altura(arv->esq) - altura(arv->dir)) > 1){

            if(info < arv->esq->info){
                arv = simp_roda_esq(arv);
            }else{
                arv = dup_roda_esq(arv);
            }
        }
    }

    else if(info > arv->info){

        arv->dir = insereAVL(info, arv->dir);

        if((altura(arv->dir) - altura(arv->esq)) > 1){

            if(info > arv->dir->info){
                arv = simp_roda_dir(arv);
            }else{
                arv = dup_roda_dir(arv);
            }
        }
    }

    arv->alt = max(altura(arv->esq), altura(arv->dir)) + 1;

    return arv;
}

nodoAVL *menorNodo(nodoAVL *raiz){

    while(raiz->esq != NULL){
        raiz = raiz->esq;
    }

    return raiz;
}

nodoAVL *removeAVL(nodoAVL *raiz, int x){

    if(raiz == NULL){
        return NULL;
    }

    if(x < raiz->info){

        raiz->esq = removeAVL(raiz->esq, x);

    }else if(x > raiz->info){

        raiz->dir = removeAVL(raiz->dir, x);

    }else{

        if(raiz->esq == NULL && raiz->dir == NULL){

            free(raiz);
            return NULL;
        }

        else if(raiz->esq == NULL){

            nodoAVL *temp = raiz->dir;

            free(raiz);
            return temp;
        }

        else if(raiz->dir == NULL){

            nodoAVL *temp = raiz->esq;

            free(raiz);
            return temp;
        }

        else{

            nodoAVL *temp = menorNodo(raiz->dir);

            raiz->info = temp->info;

            raiz->dir = removeAVL(raiz->dir, temp->info);
        }
    }

    raiz->alt = max(altura(raiz->esq), altura(raiz->dir)) + 1;

    int bal = altura(raiz->esq) - altura(raiz->dir);

    if(bal > 1){

        if(altura(raiz->esq->esq) >= altura(raiz->esq->dir)){
            return simp_roda_esq(raiz);
        }else{
            return dup_roda_esq(raiz);
        }
    }

    if(bal < -1){

        if(altura(raiz->dir->dir) >= altura(raiz->dir->esq)){
            return simp_roda_dir(raiz);
        }else{
            return dup_roda_dir(raiz);
        }
    }

    return raiz;
}

nodoAVL *buscaAVL(nodoAVL *raiz, int x){

    if(raiz == NULL){
        return NULL;
    }

    if(raiz->info == x){
        return raiz;
    }

    if(x < raiz->info){
        return buscaAVL(raiz->esq, x);
    }else{
        return buscaAVL(raiz->dir, x);
    }
}

void imprimir(nodoAVL *raiz, int nivel){

    int i;

    if(raiz != NULL){

        imprimir(raiz->dir, nivel + 1);

        printf("\n");

        for(i = 0; i < nivel; i++){
            printf("\t");
        }

        printf("%d", raiz->info);

        imprimir(raiz->esq, nivel + 1);
    }
}


int main(){

    nodoAVL *raiz = NULL;

    int opcao;
    int valor;

    do{

        printf("\n\n AVL ");
        printf("\n1 - Inserir");
        printf("\n2 - Remover");
        printf("\n3 - Buscar");
        printf("\n4 - Imprimir");
        printf("\n0 - Sair");
        printf("\nOpcao: ");
        scanf("%d", &opcao);

        switch(opcao){

            case 1:

                printf("Valor: ");
                scanf("%d", &valor);

                raiz = insereAVL(valor, raiz);

                break;

            case 2:

                printf("Valor: ");
                scanf("%d", &valor);

                raiz = removeAVL(raiz, valor);

                break;

            case 3:

                printf("Valor: ");
                scanf("%d", &valor);

                if(buscaAVL(raiz, valor) != NULL){
                    printf("Valor encontrado!");
                }else{
                    printf("Valor nao encontrado!");
                }

                break;

            case 4:

                printf("\n\nArvore:\n");
                imprimir(raiz, 0);

                printf("\n");

                break;

            case 0:

                printf("\nEncerrando...\n");

                break;

            default:

                printf("\nOpcao invalida!\n");
        }

    }while(opcao != 0);

    return 0;
}
