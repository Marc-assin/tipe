#include "knot_gen.c"
#include <stdio.h>

//Structures utiles
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

double_liste* preinsertion(double_liste* lst, int x){ //insère en tête de liste
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

double_liste* postinsertion(double_liste* lst, int x){ //insere en queue de liste
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
    return lst;
};

double_liste* suppression(double_liste* lst){
    if(lst->prec == lst) {
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
    int** adj; //tableau de tableaux de taille au plus 4, les arêtes negatives signifient que le fil passe en-dessous
    int* signes; //tableau indiquant si les fils arrivant par les impairs sont en dessous
    //données pour les graphes simplifiés:
    int* degre; //degre de chaque sommet
    int** type; //tableau indiquant si l'arête est une arete directe ou retour (signe) et si elle est simple ou double (valeur absolue)
    int* DFI; //tableau indiquant l'ordre du DFS
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
    lien_adjacence* adj; //tableau de taille 2: relie aux demi arêtes de la face extérieure
    int parentDFS;
    int DFI;
    int petit_ancetre;
    int point_min;
    int visite; //Utilisé dans la Montée
    int flag_arete_retour; //Utilisé dans la Montée
    intlist* racines_pertinentes; 
    intlist* enfantsDFS; 
    intlist* p_parentDFS; //Pointe vers la cellule de enfantsDFS de son parent
};

typedef struct sommet sommet;

struct sommet_racine{
    lien_adjacence* lien; //tableau de taille 2
    int parent; //indice dans S
};

typedef struct sommet_racine sommet_racine;

struct demi_arete{
    lien_adjacence* adj; //tableau de taille 2
    int voisin; //vers quoi l'arête pointe
    int type; // 1 ou -1 selon du sens de rotation?
    int signe; //Double: 0; au-dessus: 1; en-dessous: -1
};

typedef struct demi_arete demi_arete;

struct graphe_comb{
    int n; //nombre de sommets
    int m; //nombre d'arêtes
    sommet* S;
    sommet_racine* R; //sommets racines correspondants aux enfants!
    demi_arete* A;
    pile* P;
};

typedef struct graphe_comb graphe_comb;

int signe(int k){
    if (k<0) return (-1); 
    return 1;
}

//Fonctions d'affichage
void print_graphe(graphe g){
    for(int i=0; i<g.n; i++){
        printf("%d: %d,%d,%d,%d, signe: %d\n", i, g.adj[i][0], g.adj[i][1], g.adj[i][2], g.adj[i][3], g.signes[i]);
    }
}

void print_tab(int* tab, int n){
    printf("n=%d: ", n);
    for(int i=0; i<n; i++){
        printf("%d;", tab[i]);
    }
    printf("\n");
}

//Precalcul

//Convertit une sequence DT en un graphe
graphe conversion_seqDT(seq_dt s){
    int n = s.taille;
    //printf("n=%d\n", n);
    int** adj = malloc(n*sizeof(int*)); //sommet i correspond au numéro de séquence 2*i+1
    //Un tableau avec la paire de tous les numéros de la séquence
    int* paires = malloc((2*n+1)*sizeof(int)); //les numéros allant de 1 à 2n+1, la case 0 est inallouée et inutile
    paires[0] = -1;
    for(int i=0; i<n; i++){
        paires[2*i+1] = abs(2*s.seq[i]);
        paires[abs(2*s.seq[i])] = 2*i+1;
    };
    print_tab(paires, 2*n+1);
    int* signes = malloc(n*sizeof(int*));
    for(int i=0; i<n;i++){
        adj[i] = malloc(4*sizeof(int));
        signes[i] = signe(2*s.seq[i]);
    };
    //La case 0 correspond au numéro impair
    //La case 1 au numéro pair
    //La case 2 à la continuation de l'impair
    //la case 3 à la continuation du pair 
    adj[0][0] = paires[2*n]/2; //Correspond au numero 2*n
    adj[0][1] = (paires[1]/2-1);
    adj[paires[2*n]/2][3] = 0; //Correspond à la continuation du pair 2*n final
    for(int i=1; i<n;i++){
        adj[i][0] = paires[2*i]/2; //Correspond à l'impair entrant
        adj[i][1] = (paires[2*i+1]/2-1); //Correspond au pair entrant
    };
    for(int i=0; i<n;i++){
        adj[i][3] = ((paires[2*i+1]+1)%12)/2; //Correspond à la continuation du pair
        adj[i][2] = paires[2*i+2]/2; //Correspond à la continuation de l'impair
    };
    graphe res = {
        .n = n,
        .adj = adj,
        .signes = signes,
        .DFI = NULL,
        .type = NULL 
    };
    free(paires);
    //printf("fini conversion\n");
    return res;
};

//Initialise un graphe issu d'une seq DT en un graphe de BM
//L'initialisation de m et des arêtes est laissé au DFS
graphe_comb init_graphe_comb(graphe g){
    sommet* sommets = malloc(g.n*sizeof(sommet));
    sommet_racine* R = malloc(g.n*sizeof(sommet_racine));
    demi_arete* A = malloc(8*g.n*sizeof(demi_arete));
    pile* P = init_pile();
    int m = 0;
    for(int i=0; i<g.n; i++){
        lien_adjacence* adj = malloc(2*sizeof(lien_adjacence));
        sommet s = {
            .adj = adj, 
            .parentDFS = -1,
            .DFI = -1,
            .petit_ancetre = -1,
            .point_min = -1,
            .visite = 0,
            .flag_arete_retour = -1,
            .racines_pertinentes = NULL,
            .enfantsDFS = NULL,
            .p_parentDFS = NULL
        };
        sommets[i] = s;
    }
    graphe_comb newg = {.n = g.n,
                        .m = 0,
                        .S = sommets,
                        .R = R,
                        .A = A,
                        P = P};
    return newg;
}

void DFS(graphe g, graphe* newg, bool* vus, int s, int* index){
    /*Le DFS:   donne un index DFI a chaque sommet
                trie les aretes en aretes retour/aretes de parcours
                simplifie les aretes doubles
    */
    vus[s] = true;
    newg->DFI[(*index)++] = s;
    //compte le nombre de voisins effectifs
    int nb_vois = 1;
    for(int i = 1; i<4; i++){
        if(g.adj[s][i] != g.adj[s][i-1]) nb_vois++;
    }
    newg->degre[s] = nb_vois;
    //cree les voisins effectifs
    int* vois = malloc(nb_vois*sizeof(int));
    int* types = malloc(nb_vois*sizeof(int));
    vois[0] = g.adj[s][0];
    types[0] = 1;
    int cpt = 1;
    for(int i = 1; i<4; i++){
        if(g.adj[s][i] != g.adj[s][i-1]){
            vois[cpt++] = g.adj[s][i];
            types[cpt] = 1;
        } else {
            types[cpt] = 2; //c'est une double arête
        }
    }
    newg->adj[s] = vois;
    newg->type[s] = types;
    for(int i = 1; i<nb_vois; i++){
        if(!vus[newg->adj[s][i]]){
            DFS(g,newg,vus,g.adj[s][i], index);
        } else {
            if(newg->DFI[newg->adj[s][i]] < newg->DFI[s]-1) newg->type[s][i] *= -1; //c'est une arete retour
        }
    }
}

graphe precalcul_graphe(graphe g){
    /* Prend un graphe g ayant été extrait d'une sequence DT, et utilise un DFS pour:
    - donner un ordre DFS aux sommets
    - simplifier les aretes doubles
    - trier les aretes de parcours ou de retour*/
    graphe *pnewg = malloc(sizeof(graphe));
    pnewg->n = g.n;
    pnewg->adj = malloc(g.n*sizeof(int*));
    pnewg->DFI = malloc(g.n*sizeof(int));
    pnewg->signes = malloc(g.n*sizeof(int));
    pnewg->degre = malloc(g.n*sizeof(int));
    pnewg->type = malloc(g.n*sizeof(int*));
    bool* vus = malloc(g.n*sizeof(bool));
    for(int i = 0; i<g.n; i++){
        vus[i] = false;
        pnewg->signes[i] = g.signes[i];
    }
    int index = 0;
    DFS(g, pnewg, vus, 0, &index);
    free(vus);
    return *pnewg;
}

void precalcul(graphe g, graphe_comb *gtilde){
    /*Modifie gtilde précédemment initialisé: (g est le graphe simplifié déjà parcouru)
        - calcule les petit_ancetre des sommets
        - calcule les points_min des sommets
        - truc truc externally active?*/
    
    //Calcul de l'ancêtre direct de plus petit indice
    for(int i = 0; i<g.n; i++){ //Indice DFI
        int s = g.DFI[i];
        gtilde->S[s].DFI = i;
        gtilde->S[s].petit_ancetre = i;
        for(int v = 0; v<g.degre[s]; v++){
            if( g.type[s][v] < 0 && g.DFI[g.adj[s][v]] < gtilde->S[s].petit_ancetre){
                gtilde->S[s].petit_ancetre = g.DFI[g.adj[s][v]];
            }
        }
    }
    //Calcul du point_min
    for(int i = g.n-1; i>=0; i--){
        int s = g.DFI[i];
        gtilde->S[s].point_min = gtilde->S[s].petit_ancetre;
        for(int v = 0; v<g.degre[s]; v++){
            if( g.type[s][v] > 0 && gtilde->S[g.adj[s][v]].point_min < gtilde->S[s].point_min){
                gtilde->S[s].point_min = gtilde->S[g.adj[s][v]].point_min;
            }
        }
    }
    //Creation des enfantsDFS
    for(int s = 0; s<g.n; s++){
        //on trie les enfants par ordre croissant
        int* voisins_tries = malloc(g.degre[s]*sizeof(int));
        for(int v = 0; v<g.degre[s]; v++){
            voisins_tries[v] = g.adj[s][v];
        }
        for(int v = 0; v<g.degre[s]; v++){
            for(int t = v; t<g.degre[s]; t++){
                if(gtilde->S[g.adj[s][t]].point_min > gtilde->S[g.adj[s][v]].point_min) {
                    int temp = voisins_tries[v];
                    voisins_tries[v] = voisins_tries[t];
                    voisins_tries[t] = temp;
                }
            }
        }
        //on insère dans la liste doublement chaînée par ordre décroissant, elle est donc croissante
        double_liste* lst = NULL;
        for(int v = 0; v<g.degre[s]; v++){
            lst = preinsertion(lst, voisins_tries[v]);
            gtilde->S[voisins_tries[v]].p_parentDFS = lst;
        }
        gtilde->S[s].enfantsDFS = lst;
    }
}

void successeur_face_ext(graphe_comb gtilde, demi_arete* w, int* entree_w){
   demi_arete e = gtilde.A[w->voisin];
   int x = e.adj[0].index;
   if(gtilde.S[x].adj[0].index != gtilde.S[x].adj[1].index){ //Si le sommet n'est pas d'arité 1
        if(gtilde.S[x].adj[0].index = e.adj[1].index) *entree_w = 1; //Si la demi arete 0 est celle d'où on vient
        else *entree_w = 0;
   }
   *w = e;
   return;
}

void montee(graphe_comb* gtilde, int w, int v){
    /* Identifie le sous-graphe pertinent pour l'arête retour (v,w)
        On parcourt le chemin dans l'arbre de v à w en identifiant les sommets coupants, ie les sommets racines
        Le parcours se fait de demi arête en demi arête*/

    gtilde->S[w].flag_arete_retour = v; //On marque w comme pertinent
    demi_arete x = gtilde->A[gtilde->S[w].adj[0].index] , y = gtilde->A[gtilde->S[w].adj[1].index];
    int entree_x = 0, entree_y = 1; //L'un part à gauche, l'autre à droite

    while(x.adj[0].index != v && y.adj[0].index != v && gtilde->S[x.adj[0].index].visite != v 
            && gtilde->S[y.adj[0].index].visite != v){ //Tant qu'on n'est pas arrivé à v, et qu'on n'est pas déjà passé par là
        if(x.adj[0].lieu == dansS) gtilde->S[x.adj[0].index].visite = v; //On a visité
        if(y.adj[0].lieu == dansS) gtilde->S[y.adj[0].index].visite = v; //On a visité
        demi_arete z = {.adj = NULL};
        if(x.adj[0].lieu == dansR){
            z = x;
        } else if (y.adj[0].lieu == dansR){ 
            z = y;
        }
        if(z.adj = NULL){ //On a trouvé le sommet racine de notre composante biconnexe actuelle, on cherche donc à continuer notre remontée
            //On retrouve le parent lié à notre sommet racine
            int c = x.adj[0].index;
            int p = gtilde->S[c].parentDFS;
            if(p != v){
                if(gtilde->S[c].point_min < gtilde->S[v].DFI) preinsertion(gtilde->S[p].racines_pertinentes, c);
                else postinsertion(gtilde->S[p].racines_pertinentes, c);
            }
            x = gtilde->A[gtilde->S[p].adj[0].index];
            y = gtilde->A[gtilde->S[p].adj[1].index];
            entree_x = 0;
            entree_y = 1;

        } else { //On continue
            successeur_face_ext(*gtilde, &x, &entree_x);
            successeur_face_ext(*gtilde, &y, &entree_y);
        }
    }
}

graphe_comb BoyerMyrvold(seq_dt seq){
    /*Prend une séquence DT et renvoie le graphe combinatoire après exécution de l'algo de Boyer Myrvold*/
        //Precalcul
    //Transformation en un graphe
    graphe g = conversion_seqDT(seq);
    //Simplification du graphe
    graphe g_simple = precalcul_graphe(g);
    //Initialisation du graphe combinatoire
    graphe_comb gtilde = init_graphe_comb(g_simple);
    precalcul(g_simple, &gtilde);
        //Boucle principale
    for(int i = g.n-1; i>=0; i--){ //On traite les sommets par ordre DFI descendant
        int v = g.DFI[i];
        
        //Pour chaque enfant de v
        double_liste* debut = gtilde.S[v].enfantsDFS;
        double_liste* suivant = debut;
        while(suivant != NULL && suivant->suiv != debut){
            int c = suivant->val;
            //On crée un sommet virtuel
            lien_adjacence lien_precedent = {.lieu = dansA, .index = gtilde.m}; //Indique la demi arete precendente
            lien_adjacence lien_suivant = {.lieu = dansA, .index = gtilde.m}; //Indique la demi arete suivante
            lien_adjacence lien[2] = {lien_precedent, lien_suivant};
            sommet_racine vc = {.lien = lien, .parent = v};
            gtilde.R[c] = vc;
            //On ajoute l'arête de parcours (vc, c) comme 2 demi-arêtes
            gtilde.A[gtilde.m].adj[1].lieu = dansR;
            gtilde.A[gtilde.m].adj[1].index = c; //relie a vc
            gtilde.A[gtilde.m].adj[1].lieu = dansA; //indique l'indice dans A
            gtilde.A[gtilde.m].adj[1].index = gtilde.m; 
            gtilde.A[gtilde.m].voisin = gtilde.m+1; 
            gtilde.A[gtilde.m].type = 1; //initialisé à 1
            //gtilde.A[gtilde.m].signe = A FAIRE: pour se rappeler du type des aretes
            gtilde.A[gtilde.m+1].adj[0].lieu = dansS;
            gtilde.A[gtilde.m+1].adj[0].index = c; //relie à c
            gtilde.A[gtilde.m+1].adj[1].lieu = dansA; //indique l'indice dans A
            gtilde.A[gtilde.m+1].adj[1].index = gtilde.m+1; 
            gtilde.A[gtilde.m+1].voisin = gtilde.m; 
            gtilde.A[gtilde.m+1].type = 1; //initialisé à 1
            //gtilde.A[gtilde.m+1].signe = A FAIRE: pour se rappeler du type des aretes

            //On marque la nouvelle demi-arete comme incidente à c
            gtilde.S[c].adj[0].index = gtilde.m+1;
            gtilde.S[c].adj[0].lieu = dansA;
            gtilde.S[c].adj[1].index = gtilde.m+1;
            gtilde.S[c].adj[1].lieu = dansA;

            //On vient d'ajouter une arête: on incrémente le compteur
            gtilde.m++; gtilde.m++;

        }

        //Pour chaque arete retour entre v et un de ses descendants w
        for(int j = 0; j<g_simple.degre[v]; j++){
            if(g_simple.type[v][j] < 0 && gtilde.S[g_simple.adj[v][j]].DFI > gtilde.S[v].DFI){ //Si arete retour et descendant
                int w = g_simple.adj[v][j];
                montee(&gtilde, w, v); 
            }
        }

    }


    return gtilde;
}

//Fonctions de test
void test_conversion(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    print_seq_dt(&noeud_wiki);
    print_graphe(gnoeud_wiki);
    return;
}

/*A tester: conversion en graphe combinatoire et ordre DFI
            procédure walkup*/

int main(){
    printf("\nok!\n");
    return 0;
}