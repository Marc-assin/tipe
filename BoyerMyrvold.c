#include "knot_gen.c"

//Structutes utiles
struct intlist{
    int val;
    struct intlist* next;
};
typedef struct intlist intlist;

typedef struct intlist pile;

bool pile_vide(pile* P){
    return NULL == P;
};

int top(pile* P){
    return P->val;
};

pile* pop(pile* P){
    if(! pile_vide(P)){
        pile* res = P->next;
        free(P);
        return res;
    }
    return NULL;
};

pile* push(pile* P, int x){
    pile* res = malloc(sizeof(pile));
    res->val = x;
    res->next = P;
    return res;
};

pile* init_pile(){
    return NULL;
};

struct double_liste{
    int val;
    struct double_liste* suiv;
    struct double_liste* prec;
};

typedef struct double_liste double_liste;

double_liste* insertion(double_liste* lst, int x){ //insere avant la cellule selectionnee
    double_liste* cellule = malloc(sizeof(double_liste));
    cellule->val = x;
    if(lst != NULL){
        cellule->prec = lst;
        cellule->suiv = lst->suiv;
        lst->suiv = cellule;
        lst->suiv->prec = cellule;
        
    } else {
        cellule->suiv = cellule;
        cellule->prec = cellule;
    }
    return cellule;
};

double_liste* suppression(double_liste* lst){
    if(lst->prec = lst) {
        free(lst);
        return NULL;
    }
    lst->prec->suiv = lst->suiv;
    lst->suiv->prec = lst->prec;
    double_liste* res = lst->prec;
    free(lst);
    return res;
};

//Graphe d'entrée
struct graphe{
    int n;
    int** adj; //tableau de tableaux de taille 4
};

typedef struct graphe graphe;

//Structures de travail
enum lieu_adjacence{
    dansA, dansS, dansR
};

typedef enum lieu_adjacence lieu_adjacence;

struct lien_adjacence{
    lieu_adjacence lieu;
    int index;
};

typedef struct lien_adjacence lien_adjacence;

struct sommet{
    lien_adjacence* adj; //tableau de taille 2
    int parentDFS;
    int DFI;
    int ancetre_lointain;
    int point_min;
    int visite;
    int flag_arete_retour;
    intlist* racines_pertinentes;
    intlist* enfantsDFS_separes;
    intlist* p_parentDFS;
};

typedef struct sommet sommet;

struct sommet_racine{
    lien_adjacence* adj; //tableau de taille 2
    int parent; //indice dans S
};

typedef struct sommet_racine sommet_racine;

struct demi_arete{
    lien_adjacence* adj; //tableau de taille 2
    int voisin;
    int signe; // 1 ou -1 
};

typedef struct demi_arete demi_arete;

struct graphe_comb{
    int n; //nombre de sommets
    int m; //nombre d'arêtes
    sommet* S;
    sommet_racine* R;
    demi_arete* A;
    pile* P;
};

typedef struct graphe_comb graphe_comb;

graphe conversion_seqDT(seq_dt s){
    int n = s.taille;
    int** adj = malloc(n*sizeof(int*));
    //Un tableau avec la paire de tous les numéros de la séquence
    int* paires = malloc((2*n+1)*sizeof(int)); //les numéros allant de 1 à 2n+1, la case 0 est inallouée et inutile
    for(int i=0; i<n; i++){
        paires[2*i+1] = 2*s.seq[i];
        paires[2*s.seq[i]] = 2*i+1;
    };
    for(int i=0; i<n;i++){
        adj[i] = malloc(4*sizeof(int));
    };
    //Ajout des voisins issus du nombre impair
    adj[0][0] = paires[2*n]/2;
    adj[paires[2*n]/2][2] = 0;
    for(int i=1; i<n;i++){
        adj[i][0] = paires[2*i]/2;
        adj[paires[2*i]/2][2] = i;
    };
    for(int i=0; i<n;i++){
        adj[i][1] = paires[2*i+2]/2;
        adj[paires[2*i+2]/2][3] = i;
    };
    graphe res = {
        .n = n,
        .adj = adj
    };
    return res;
};

//Le DFS:   donne un index DFI a chaque sommet
//          trie les aretes en aretes retour/aretes de parcours
void DFS(graphe g, graphe_comb newg, bool* vus, int s, int* index){
    vus[s] = true;
    newg.S[s].DFI = (*index)++; 
    for(int i = 0; i<4; i++){
        if(!vus[g.adj[s][i]]){
            newg.S[g.adj[s][i]].parentDFS = s;
            DFS(g,newg,vus,g.adj[s][i], index);
        }
    }
}