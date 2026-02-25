#include <stdlib.h>
#include <stdio.h>

struct int_vector {
    int taille, capacite;
    int *data;
};

typedef struct int_vector int_vector;

void append_ivec(int_vector *v, int x){
    if (v->capacite > v->taille){
        v->data[v->taille] = x;
        v->taille++;
    } else {
        // printf("%d %d\n", v->taille, v->capacite);
        int *new_data = malloc(sizeof(int) * (v->capacite * 3 / 2 + 1));
        for (int i = 0; i < v->taille; i++){
            new_data[i] = v->data[i];
        }
        free(v->data);
        v->data = new_data;
        v->data[v->taille] = x;
        v->capacite = v->capacite * 3 / 2 + 1;
        v->taille++;
    }
}

int get_ivec(int_vector *v, int idx){
    if (idx >= 0 && idx < v->taille) return v->data[idx];
    printf("illegal get\n");
    return -1;
}

void insert_ivec(int_vector *v, int x, int idx){
    if (idx < 0 || idx > v->taille){
        printf("illegal insert\n");
        return;
    }
    if (idx == v->taille){
        append_ivec(v, x);
        return;
    }
    int buf = v->data[idx];
    v->data[idx] = x;
    for (int i = idx + 1; i < v->taille; i++){
        int temp = v->data[i];
        v->data[i] = buf;
        buf = temp;
    }
    append_ivec(v, buf);
}

int_vector *init_ivec(int capa){
    int_vector *v = malloc(sizeof(int_vector));
    v->taille = 0;
    if (capa <= 0){
        capa = 10;
    }
    v->capacite = capa;
    v->data = malloc(sizeof(int) * capa);
    return v;
}

void free_ivec(int_vector *v){
    free(v->data);
    free(v);
}

void print_ivec(int_vector *v){
    for (int i = 0; i < v->taille; i++){
        printf("%d ", v->data[i]);
    }
    printf("\n");
}

// void main(){
void test_ivec(){
    int_vector *v = init_ivec(0);
    append_ivec(v, 1);
    append_ivec(v, 1);
    append_ivec(v, 2);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    append_ivec(v, -1);
    printf("%d\n", v->taille);
    printf("%d %d %d\n", get_ivec(v, 0), get_ivec(v, 1), get_ivec(v, 2));
    insert_ivec(v, 3, 1);
    printf("%d %d %d\n", get_ivec(v, 0), get_ivec(v, 1), get_ivec(v, 2));
    printf("%d\n", v->taille);
    print_ivec(v);
    free_ivec(v);
}