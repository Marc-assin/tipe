#include "knot_gen.c"

struct graph
{
    int nb_sommets;
    int** voisins;
    int* nb_voisins;
};

typedef struct graph graph;

void init_graph(graph* g, int n){
    g->nb_sommets = n;
    g->voisins = malloc(n*sizeof(int*));
    g->nb_voisins = malloc(n*sizeof(int));

    for(int i=0; i<n; i++){
        g->voisins[i] = malloc(n*sizeof(int));
        g->nb_voisins[i] = 0;
    }
}

void free_graph(graph* g){
    for(int i=0; i<g->nb_sommets; i++){
        free(g->voisins[i]);
    }
    free(g->nb_voisins);
    free(g->voisins);
}

void ajout_arete(graph* g, int a, int b){
    g->voisins[a][g->nb_voisins[a]] = b;
    g->voisins[b][g->nb_voisins[b]] = a;
    g->nb_voisins[a]++;
    g->nb_voisins[b]++;
}

graph* dt_vers_graph(seq_dt* s){
    graph* g = malloc(sizeof(graph));
    init_graph(g,s->taille);
    int prev_idx = 0;
    for(int i=0; i<s->taille; i++){
        ajout_arete(g, i, s->seq[i]%s->taille);
        ajout_arete(g, i, s->seq[i]-1);
    }
}

int main(){
    return 0;
}