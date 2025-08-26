#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

struct seq_dt {
    int *seq; // on stocke seulement les nombre pairs, divisés par 2
    int taille;
};

typedef struct seq_dt seq_dt;

void init_seq_dt(seq_dt *s, int taille) {
    s->seq = (int *)malloc(taille * sizeof(int));
    if (s->seq == NULL) {
        fprintf(stderr, "Pas pu allouer de la mémoire\n");
        exit(EXIT_FAILURE);
    }
    s->taille = taille;
}

void free_seq_dt(seq_dt *s) {
    if (s->seq != NULL) {
        free(s->seq);
        s->seq = NULL;
    }
    s->taille = 0;
}

void print_seq_dt(const seq_dt *s) {
    if (s->seq == NULL || s->taille <= 0) {
        printf("La séquence est vide.\n");
        return;
    }
    for (int i = 0; i < s->taille; i++) {
        printf("(%d; %d), ", 2*i+1, 2*s->seq[i]);
    }
    printf("\n");
}


int abs(int x) {
    return (x < 0) ? -x : x;
}

void calculer_a_i(int* p, seq_dt* s){   //On remplit les cases de 1 à 2n par leur binôme dans 1 à 2n
    for(int i=0; i<s->taille; i++){
        p[2*i+1] = 2*s->seq[i];
        p[2*s->seq[i]] = 2*i+1;
    }
}

void calculer_phi(int** phi, int* a, int n){
    for(int i=1; i<=2*n; i++){
        phi[i][i] = 1;
        for(int r=i+1; r<=2*n; r++){
            if(a[r]<= a[i] && i <= a[r]){
                phi[i][r] = -phi[i][r-1];
            } else {
                phi[i][r] = phi[i][r-1];
            }
        }
        for(int r=i-1; r>0; r--){
            if(a[r+1]<= a[i] && i <= a[r+1]){
                phi[i][r] = -phi[i][r+1];
            } else {
                phi[i][r] = phi[i][r+1];
            }
        }
    }
}


// ordre : ordre des permutations, puis ordre des sous parties pour les signes
void next_seq_dt(seq_dt *s) {
    if (s->seq == NULL || s->taille <= 0) {
        fprintf(stderr, "La séquence n'est pas initialisée ou a une taille invalide.\n");
        return;
    }
    bool est_entierement_negatif = true;
    for (int i = 0; i < s->taille; i++) {
        if (s->seq[i] >= 0) {
            est_entierement_negatif = false;
            break;
        }
    }

    if (!est_entierement_negatif) { // on prend les positions des moins suivantes en binaire
        int n = 0; // correspond aux positions des moins
        for (int i = 0; i < s->taille; i++) {
            n *= 2;
            if (s->seq[i] < 0) {
                n++;
            }
            // printf("n = %d\n", n);
        }

        n++;
        for (int i = s->taille - 1; i >= 0; i--) {
            s->seq[i] = (n % 2 == 0) ? abs(s->seq[i]) : -abs(s->seq[i]);
            n /= 2;
        }
    } else { // on prend la permutation suivante pour l'ordre lexicographique
        for (int i = 0; i < s->taille; i++) {
            s->seq[i] = -s->seq[i]; // on remet les signes à +
        }

        int pivot_index = s->taille - 2;
        while (pivot_index >= 0 && s->seq[pivot_index + 1] <= s->seq[pivot_index]) {
            pivot_index--;
        }
    
        if (pivot_index < 0) {
            printf("Fin de la séquence.\n");
            free_seq_dt(s);
            exit(EXIT_SUCCESS);
            return;
        }
    
        int swap_index = s->taille - 1;
        while (s->seq[swap_index] <= s->seq[pivot_index]) {
            swap_index--;
        }
    
        int temp = s->seq[pivot_index];
        s->seq[pivot_index] = s->seq[swap_index];
        s->seq[swap_index] = temp;
    
        for (int i = pivot_index + 1, j = s->taille - 1; i < j; i++, j--) {
            int temp = s->seq[i];
            s->seq[i] = s->seq[j];
            s->seq[j] = temp;
        }
    }
}

void test_en_taille_4() {
    int taille = 4;

    seq_dt s;
    init_seq_dt(&s, taille);

    for (int i = 1; i <= taille; i++) {
        s.seq[i-1] = i;
    }

    while (true) {
        print_seq_dt(&s);
        next_seq_dt(&s);
    }
}

typedef seq_dt r_seq_dt;

r_seq_dt* realiser_seq_dt(seq_dt *s){
    r_seq_dt *res = malloc(sizeof(r_seq_dt));
    init_seq_dt(res, 2*s->taille);
    if (s->seq == NULL || s->taille <= 0) {
        return res;
    }
    for (int i = 0; i < s->taille; i++) {
        res->seq[2*i] = 2*i+1;
        res->seq[2*i+1] = 2*s->seq[i];
    }
    return res;
}

int * init_UF(int n){
    int *arbre = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++){
        arbre[i] = i;
    }
    return arbre;
}

int Find(int arbre[], int A){
    if (arbre[A] == A){
        return A;
    }
    arbre[A] = Find(arbre, arbre[A]);
    return arbre[A];
}

void Union(int arbre[], int A, int B){
    arbre[Find(arbre, A)] = Find(arbre, B);
}

int min(int a, int b){
    if (a < b){
        return a;
    }
    return b;
}

int max(int a, int b){
    if (a > b){
        return a;
    }
    return b;
}

bool regle1(seq_dt *seq){
    if (seq->taille < 3){
        return false;
    }
    int *arbre = init_UF(seq->taille);
    for (int i = 0; i < seq->taille; i++){
        for (int j = 0; j < i; j++){
            if (Find(arbre, i) == Find(arbre, j)){
                continue;
            }
            int d1 = min(abs(seq->seq[i]) * 2, i * 2 + 1), 
                d2 = min(abs(seq->seq[j]) * 2, j * 2 + 1), 
                f1 = max(abs(seq->seq[i]) * 2, i * 2 + 1), 
                f2 = max(abs(seq->seq[j]) * 2, j * 2 + 1);

            if ((d1 < d2 && d2 < f1 && f1 < f2) || (d2 < d1 && d1 < f2 && f2 < f1)){
                Union(arbre, i, j);
            }
        }
    }
    
    for (int i = 1; i < seq->taille; i++){
        if (Find(arbre, 0) != Find(arbre, i)){
            free(arbre);
            return false;
        }
    }
    free(arbre);
    return true;
}

void test_regle1(){
    int taille = 7;

    seq_dt s;
    init_seq_dt(&s, taille);

    s.seq[0] = 6;
    s.seq[1] = 4;
    s.seq[2] = 5;
    s.seq[3] = 2;
    s.seq[4] = 3;
    s.seq[5] = 7;
    s.seq[6] = 1;

    printf("%s\n", regle1(&s)==false?"correct":"faux");
    free_seq_dt(&s);
    
    taille = 3;
    init_seq_dt(&s, taille);
    
    s.seq[0] = 2;
    s.seq[1] = 3;
    s.seq[2] = 1;
    
    printf("%s\n", regle1(&s)==true?"correct":"faux");
    free_seq_dt(&s);
    
    taille = 5;
    init_seq_dt(&s, taille);
    
    s.seq[0] = 3;
    s.seq[1] = 4;
    s.seq[2] = 5;
    s.seq[3] = 1;
    s.seq[4] = 2;
    
    printf("%s\n", regle1(&s)==true?"correct":"faux");
    free_seq_dt(&s);
    
    taille = 6;
    init_seq_dt(&s, taille);
    
    s.seq[0] = 5;
    s.seq[1] = 3;
    s.seq[2] = 4;
    s.seq[3] = 2;
    s.seq[4] = 6;
    s.seq[5] = 1;
    
    printf("%s\n", regle1(&s)==false?"correct":"faux");
    free_seq_dt(&s);
}