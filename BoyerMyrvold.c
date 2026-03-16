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
        cellule->suiv = lst;
        cellule->prec = lst->prec;
        lst->prec->suiv = cellule;
        lst->prec = cellule;
        
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
        cellule->suiv = lst;
        cellule->prec = lst->prec;
        lst->prec->suiv = cellule;
        lst->prec = cellule;
        
    } else {
        cellule->suiv = cellule;
        cellule->prec = cellule;
        lst = cellule;
    }
    return lst;
};

double_liste* suppression(double_liste* lst){ //Supprime le 1er élément
    if(lst->prec == lst) {
        free(lst);
        return NULL;
    }
    lst->prec->suiv = lst->suiv;
    lst->suiv->prec = lst->prec;
    double_liste* res = lst->suiv;
    free(lst);
    return res;
};

//Graphe d'entrée
struct graphe{
    int n;
    int** adj; //tableau de tableaux de taille au plus 4 (4 si multigraphe, moins si graphe simplifié)
    int** signes; //tableau indiquant si l'arête passe au-dessus (>0), en-dessous (<0), ou est double (graphes simplifiés)
    //données pour les graphes simplifiés:
    int* degre; //degre de chaque sommet: s'il vaut 2, alors il y a une boucle
    int** type; //tableau indiquant si l'arête est une arete directe (>0) ou retour (<0)
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
    lien_adjacence adj[2]; //tableau de taille 2: relie aux demi arêtes de la face extérieure
    int parentDFS;
    int DFI;
    int petit_ancetre;
    int point_min;
    int visite; //Utilisé dans la Montée
    int flag_arete_retour; //Utilisé dans la Montée
    double_liste* racines_pertinentes; 
    double_liste* enfantsDFS; 
    double_liste* p_parentDFS; //Pointe vers la cellule de enfantsDFS de son parent
};

typedef struct sommet sommet;

struct sommet_racine{
    lien_adjacence lien[2]; //tableau de taille 2: demi arêtes de part et d'autre
    int parent; //indice dans S
    double_liste* pertinence; //lien vers la cellule de racines_pertinentes éventuelle de son parent
};

typedef struct sommet_racine sommet_racine;

struct demi_arete{
    lien_adjacence adj[2]; //tableau de taille 2: 0 -> la précédente dans le sens trigo, 1 -> la suivante dans le sens trigo
    int jumelle; //demi-arete jumelle
    lien_adjacence voisin; //sommet (racine ou non) correspondant
    int type; // 1 ou -1 selon le sens de rotation
    int signe; //Double: 0; au-dessus: 1; en-dessous: -1
    bool reelle; //arête réelle ou raccourci
};

typedef struct demi_arete demi_arete;

struct graphe_comb{
    int n; //nombre de sommets
    int m; //nombre d'arêtes
    sommet* S;
    sommet_racine* R; //sommets racines correspondants aux enfants!
    demi_arete* A;
    pile* P; //Pile de fusion de la descente, contient alternativement sommet et direction
};

typedef struct graphe_comb graphe_comb;

//Fonctions utiles

int signe(int k){
    if (k<0) return (-1); 
    return 1;
}

void successeur_face_ext(graphe_comb gtilde, demi_arete* w, int* entree_w){
    /*On est entré par la demi arete w par entree_w:
    On prend l'autre arete
    On cherche à savoir si ce successeur est entré par 0 ou 1*/
    int x = w->voisin.index;

    demi_arete e;
    if(w->voisin.lieu == dansS) e = gtilde.A[gtilde.S[x].adj[1-*entree_w].index];//la demi-arete suivante (toujours reliée à x)
    else e = gtilde.A[gtilde.R[x].lien[1-*entree_w].index];

    // char l;
    // if(w->voisin.lieu == dansS) l = 'S'; else l='R';
    // printf("Entree: %d%c par %d\n",x,l, *entree_w);
    // if(e.voisin.lieu == dansS) l = 'S'; else l='R';
    // printf("Sortie: %d%c par %d\n", e.voisin.index,l, 1-*entree_w);

    *w = gtilde.A[e.jumelle];//on traverse
    x = w->voisin.index;
    if(w->voisin.lieu == dansS && gtilde.S[x].adj[0].index != gtilde.S[x].adj[1].index){ //Si le sommet n'est pas d'arité 1
            if(gtilde.A[gtilde.S[x].adj[0].index].jumelle == w->jumelle) *entree_w = 0; //Si la demi arete 0 est celle d'où on vient
            else *entree_w = 1;
    } else if(w->voisin.lieu == dansR && gtilde.R[x].lien[0].index != gtilde.R[x].lien[1].index){
            if(gtilde.A[gtilde.R[x].lien[0].index].jumelle == w->jumelle) *entree_w = 0; //Si la demi arete 0 est celle d'où on vient
            else *entree_w = 1;
    }//Si le sommet est d'arité 1, on continue du mm côté
   
//    if(w->voisin.lieu == dansS) l = 'S'; else l='R';
//     printf("Entree: %d%c par %d\n",w->voisin.index,l, *entree_w);
   return;
}

//Fonctions d'affichage
void print_graphe_simple(graphe g){
    for(int i=0; i<g.n; i++){
        printf("%d:  DFI: %d ", i, g.DFI[i]);
        for(int j = 0; j<g.degre[i]; j++){
            printf(" (%d: type %d, signe: %d) ,", g.adj[i][j], g.type[i][j], g.signes[i][j]);
        }
        printf("\n");
    }
}

void print_multigraphe(graphe g){
    for(int i=0; i<g.n; i++){
        printf("%d: %d,%d,%d,%d, signe: %d\n", i, g.adj[i][0], g.adj[i][1], g.adj[i][2], g.adj[i][3], g.signes[i][0]);
    }
}

void print_tab(int* tab, int n){
    printf("n=%d: ", n);
    for(int i=0; i<n; i++){
        printf("%d;", tab[i]);
    }
    printf("\n");
}

void print_list(double_liste* lst){
    if(lst ==  NULL) {
        printf("NULL\n");
        return;
    };
    printf("%d, ", lst->val);
    double_liste* suiv = lst->suiv;
    while(suiv != lst){
        printf("%d, ", suiv->val);
        suiv = suiv->suiv;
    }
    printf("\n");
}

void print_graphe_comb_initial(graphe_comb g){
    printf("nombre de sommets: %d\n", g.n);
    for(int i = 0; i<g.n; i++){
        printf("\nsommet %d:\n", i);
        printf("DFI: %d, ", g.S[i].DFI);
        printf("parentDFS: %d, ", g.S[i].parentDFS);
        printf("plus petit ancetre: %d, ", g.S[i].petit_ancetre);
        printf("point min: %d, ", g.S[i].point_min);
        printf("enfantsDFS: ");
        print_list(g.S[i].enfantsDFS);
    }
    printf("\n");
}

void print_face_ext(graphe_comb g, lieu_adjacence l, int s){
    if(l == dansS && g.S[s].adj[1].index != -1){
        printf("%d -> ", s);
        demi_arete w = g.A[g.S[s].adj[1].index];
        int entree_w = 0;
        w = g.A[g.R[g.A[w.jumelle].voisin.index].lien[1].index];
        while(w.voisin.index != s || w.voisin.lieu != dansS){ //Tant que pas revenu au départ
            if(w.voisin.lieu == dansS) printf("%d -> ", w.voisin.index);
            else printf("v%d -> ", w.voisin.index);
            successeur_face_ext(g, &w, &entree_w);
        }
        
    }
}

void print_aretes_ext(graphe_comb g, lieu_adjacence l, int s){
    char lieu0, lieu1;
    if(l == dansS){
        if(g.A[g.A[g.S[s].adj[0].index].jumelle].voisin.lieu == dansS) lieu0 = 'S'; else lieu0 = 'R';
        if(g.A[g.A[g.S[s].adj[1].index].jumelle].voisin.lieu == dansS) lieu1 = 'S'; else lieu1 = 'R';
        printf("(%d, %d %c) <- %d -> (%d, %d %c) ", g.S[s].adj[0].index, g.A[g.A[g.S[s].adj[0].index].jumelle].voisin.index, lieu0, 
            s, g.S[s].adj[1].index, g.A[g.A[g.S[s].adj[1].index].jumelle].voisin.index, lieu1);
    } else if(l == dansR){
        if(g.A[g.A[g.R[s].lien[0].index].jumelle].voisin.lieu == dansS) lieu0 = 'S'; else lieu0 = 'R';
        if(g.A[g.A[g.R[s].lien[1].index].jumelle].voisin.lieu == dansS) lieu1 = 'S'; else lieu1 = 'R';
        printf("(%d, %d %c) <- %d -> (%d, %d %c) ", g.R[s].lien[0].index, g.A[g.A[g.R[s].lien[0].index].jumelle].voisin.index, lieu0, 
            s, g.R[s].lien[1].index, g.A[g.A[g.R[s].lien[1].index].jumelle].voisin.index, lieu1);
    } else printf("Probleme de lieu!!!");
}

void print_graphe_comb(graphe_comb g){
    printf("nombre de sommets: %d\n", g.n);
    for(int i = 0; i<g.n; i++){
        printf("\nsommet %d:\n", i);
        printf("DFI: %d, ", g.S[i].DFI);
        printf("parentDFS: %d, ", g.S[i].parentDFS);
        printf("Aretes face exterieure: ");
        print_aretes_ext(g, dansS, i);
        //printf("Face ext:\n");
        //print_face_ext(g, dansS, i);
        printf("\nRacines pertinentes: ");
        print_list(g.S[i].racines_pertinentes);
    }
    printf("\nSommets pertinents:\n");
    for(int i = 0; i<g.n; i++){
        printf("%d: %d\n", i, g.S[i].flag_arete_retour);
    }
    print_aretes_ext(g, dansR, 2);
    printf("\n");
}

void print_pile (pile* P){
    if(P != NULL){
        printf("%d, ", P->val);
        print_pile(P->next);
    }
}

void print_position(char nom, int x, lieu_adjacence l){
    char c;
    if(l==dansS) c = 'S'; else if(l==dansR) c = 'R'; else c = 'A';
    printf("%c: %d %c\n", nom, x, c);
}

void print_aretes(graphe_comb g, lieu_adjacence l, int s){
    char lieu;
    if(l==dansS){
        int w = g.S[s].adj[0].index;
        if (w == -1) return;
        if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
        printf("(%d, %d %c) ->", w, g.A[g.A[w].jumelle].voisin.index, lieu);
        w = g.A[w].adj[1].index;
        while(w != g.S[s].adj[0].index){
            if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
            printf("(%d, %d %c) ->", w, g.A[g.A[w].jumelle].voisin.index, lieu);
            w = g.A[w].adj[1].index;
        }
    } else {
        int w = g.R[s].lien[0].index;
        if (w == -1) return;
        if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
        printf("(%d, %d %c) ->", w, g.A[g.A[w].jumelle].voisin.index, lieu);
        w = g.A[w].adj[1].index;
        while(w != g.R[s].lien[0].index){
            if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
            printf("(%d, %d %c) ->", w, g.A[g.A[w].jumelle].voisin.index, lieu);
            w = g.A[w].adj[1].index;
        }
    }
    printf("\n");
}

void print_graphe_comb_final(graphe_comb g){
    printf("nombre de sommets: %d\n", g.n);
    printf("nombre d'aretes: %d\n", g.m);
    for(int i = 0; i<g.n; i++){
        printf("sommet %d:\n", i);
        printf("Aretes:");
        print_aretes(g, dansS, i);
        printf("Racines pertinentes: ");
        print_list(g.S[i].racines_pertinentes);
    }
    //print_aretes(g, dansR, 1);
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
    //print_tab(paires, 2*n+1);
    int** signes = malloc(n*sizeof(int*));
    for(int i=0; i<n;i++){
        adj[i] = malloc(4*sizeof(int));
        signes[i] = malloc(4*sizeof(int));
        signes[i][0] = signe(s.seq[i]);
        signes[i][1] = (-1)*signe(s.seq[i]);
        signes[i][2] = signe(s.seq[i]);
        signes[i][3] = (-1)*signe(s.seq[i]);
        
    };
    //La case 0 correspond au numéro impair
    //La case 1 au numéro pair
    //La case 2 à la continuation de l'impair
    //la case 3 à la continuation du pair 
    adj[0][0] = paires[2*n]/2; //Correspond au numero 2*n
    adj[0][1] = (paires[1]/2-1);
 //   adj[paires[2*n]/2][3] = 0; //Correspond à la continuation du pair 2*n final
    for(int i=1; i<n;i++){
        adj[i][0] = paires[2*i]/2; //Correspond à l'impair entrant
        adj[i][1] = (paires[2*i+1]/2-1); //Correspond au pair entrant
    };
    for(int i=0; i<n;i++){
        adj[i][3] = ((paires[2*i+1]+1)%(2*n))/2; //Correspond à la continuation du pair
        adj[i][2] = paires[2*i+2]/2; //Correspond à la continuation de l'impair
    };
    graphe res = {
        .n = n,
        .adj = adj,
        .signes = signes,
        .type = NULL,
        .DFI = NULL,
    };
    free(paires);
    //printf("fini conversion\n");
    return res;
};

//Initialise un graphe issu d'une seq DT en un graphe de BM
graphe_comb init_graphe_comb(graphe g){
    sommet* sommets = malloc(g.n*sizeof(sommet));
    sommet_racine* R = malloc(g.n*sizeof(sommet_racine));
    demi_arete* A = malloc(8*g.n*sizeof(demi_arete));
    pile* P = init_pile();

    if (!sommets || !R || !A ) {
        printf("Allocation failed\n");
        exit(1);
    }
    for(int i=0; i<g.n; i++){
        sommets[i].adj[0].index = -1;
        sommets[i].adj[1].index = -1;
        sommets[i].adj[0].lieu = dansA;
        sommets[i].adj[1].lieu = dansA;
        sommets[i].parentDFS = -1;
        sommets[i].DFI = -1;
        sommets[i].petit_ancetre = -1;
        sommets[i].point_min = -1;
        sommets[i].visite = -1;
        sommets[i].flag_arete_retour = -1;
        sommets[i].racines_pertinentes = NULL;
        sommets[i].enfantsDFS = NULL;
        sommets[i].p_parentDFS = NULL;
    }
    for(int i=0; i<g.n; i++){
        R[i].parent = -1;
    }

    graphe_comb newg = {.n = g.n,
                        .m = 0,
                        .S = sommets,
                        .R = R,
                        .A = A,
                        .P = P};
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
    int nb_vois = 0;
    for(int i = 0; i<4; i++){
        if(g.adj[s][i] != g.adj[s][(i+1) % 4] && g.adj[s][i] != s) nb_vois++;
    }
    newg->degre[s] = nb_vois;
    //cree les voisins effectifs
    int* vois = malloc(nb_vois*sizeof(int));
    int* types = malloc(nb_vois*sizeof(int));
    int* signes = malloc(nb_vois*sizeof(int));
    for(int i = 0; i<nb_vois; i++){
        types[i] = -1;
        signes[i] = 1;
    }
    int cpt = 0;
    for(int i = 0; i<4; i++){
        if(g.adj[s][i] != g.adj[s][(i+1)%4]){
            if(g.adj[s][i] != s){
                vois[cpt] = g.adj[s][i];
                signes[cpt] *= g.signes[s][i];
                cpt++;
            }
        } else {
            signes[cpt] = 0; //c'est une double arête
        }
    }
    newg->adj[s] = vois;
    newg->type[s] = types;
    newg->signes[s] = signes;

    for(int i = 0; i<nb_vois; i++){
        if(!vus[newg->adj[s][i]]){
            newg->type[s][i] = 1;
            DFS(g,newg,vus,newg->adj[s][i], index);
        } else {
            //on identifie l'arête correspondante et on match son type
            for(int j = 0; j<newg->degre[newg->adj[s][i]]; j++){
                if(newg->adj[newg->adj[s][i]][j] == s) newg->type[s][i] = newg->type[newg->adj[s][i]][j] ;
            } 

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
    pnewg->signes = malloc(g.n*sizeof(int*));
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
        - calcule le parent DFS de chaque sommet
        - calcule les petit_ancetre des sommets
        - calcule les points_min des sommets
        - truc truc externally active?*/

    //Calcul de l'ancêtre direct de plus petit indice et DFI
    for(int i = 0; i<g.n; i++){ //Indice DFI
        int s = g.DFI[i];
        gtilde->S[s].DFI = i;
        gtilde->S[s].petit_ancetre = i;
    }

    for(int i = 0; i<g.n; i++){
        int s = g.DFI[i];
        for(int v = 0; v<g.degre[s]; v++){
            if( g.type[s][v] < 0 && gtilde->S[g.adj[s][v]].DFI < gtilde->S[s].petit_ancetre){ //Arete retour et DFI plus petit
                gtilde->S[s].petit_ancetre = gtilde->S[g.adj[s][v]].DFI;
            }
        }
    }

    //Calcul du parent DFS
    for(int s=0; s<g.n; s++){
        for(int v = 0; v<g.degre[s]; v++){
            if( g.type[s][v] > 0 && gtilde->S[s].DFI > gtilde->S[g.adj[s][v]].DFI){
                gtilde->S[s].parentDFS = g.adj[s][v];
            }
        }
    }

    //Calcul du point_min
    for(int i = g.n-1; i>=0; i--){
        int s = g.DFI[i];
        gtilde->S[s].point_min = gtilde->S[s].petit_ancetre;
        for(int v = 0; v<g.degre[s]; v++){
            if( g.type[s][v] > 0 && gtilde->S[g.adj[s][v]].DFI > gtilde->S[s].DFI //arête de parcours descendante
                && gtilde->S[g.adj[s][v]].point_min < gtilde->S[s].point_min){
                gtilde->S[s].point_min = gtilde->S[g.adj[s][v]].point_min;
            }
        }
    }
    //Creation des enfantsDFS
    //on trie les sommets par ordre croissant de point min
    int* sommets_tries = malloc(g.n*sizeof(int));
    for(int v = 0; v<g.n; v++){
        sommets_tries[v] = v;
    }
    for(int v = 0; v<g.n; v++){
        for(int t = v; t<g.n; t++){
            if(gtilde->S[v].point_min > gtilde->S[t].point_min) {
                int temp = sommets_tries[v];
                sommets_tries[v] = sommets_tries[t];
                sommets_tries[t] = temp;
            }
        }
    }
    for(int s = 0; s<g.n; s++){
        //on insère dans la liste doublement chaînée par ordre croissant à la fin de la liste
        int parent = gtilde->S[sommets_tries[s]].parentDFS;
        if(parent >= 0) {
            gtilde->S[parent].enfantsDFS = postinsertion(gtilde->S[parent].enfantsDFS, sommets_tries[s]);
            gtilde->S[sommets_tries[s]].p_parentDFS = gtilde->S[parent].enfantsDFS->prec;
        }
    }
    free(sommets_tries);
}

void montee(graphe_comb* gtilde, int w, int v){
    /* Identifie le sous-graphe pertinent pour l'arête retour (v,w)
        On parcourt le chemin dans l'arbre de v à w en identifiant les sommets coupants, ie les sommets racines
        Le parcours se fait de demi arête en demi arête*/

    printf("-----Montee depuis %d:\n\n", w);
    gtilde->S[w].flag_arete_retour = v; //On marque w comme pertinent
    demi_arete x = gtilde->A[gtilde->S[w].adj[0].index] , y = gtilde->A[gtilde->S[w].adj[1].index];
    int entree_x = 0, entree_y = 1; //L'un part sens horaire, l'autre sens trigo

    while(x.voisin.index != v && y.voisin.index != v && (gtilde->S[x.voisin.index].visite != v || x.voisin.lieu != dansS)
            && (gtilde->S[y.voisin.index].visite != v || y.voisin.lieu != dansS)){ //Tant qu'on n'est pas arrivé à v, et qu'on n'est pas déjà passé par là
        
        // print_position('x', x.voisin.index, x.voisin.lieu);
        // print_position('y', y.voisin.index, y.voisin.lieu);

        if(x.voisin.lieu == dansS) gtilde->S[x.voisin.index].visite = v; //On a visité
        if(y.voisin.lieu == dansS) gtilde->S[y.voisin.index].visite = v; //On a visité

        demi_arete z = {.jumelle = -1};
        if(x.voisin.lieu == dansR){
            z = x;
        } else if (y.voisin.lieu == dansR){ 
            z = y;
        }
        if(z.jumelle != -1){ //On a trouvé le sommet racine de notre composante biconnexe actuelle, on cherche donc à continuer notre remontée
            //On retrouve le parent lié à notre sommet racine
            // printf("trouve racine\n");
            int c = z.voisin.index;
            int p = gtilde->S[c].parentDFS;
            // printf("c: %d, p: %d\n\n", c, p);
            if(p != v){
                if(gtilde->S[c].point_min < gtilde->S[v].DFI) gtilde->S[p].racines_pertinentes = preinsertion(gtilde->S[p].racines_pertinentes, c);
                else gtilde->S[p].racines_pertinentes = postinsertion(gtilde->S[p].racines_pertinentes, c);
                x = gtilde->A[gtilde->S[p].adj[0].index];
                y = gtilde->A[gtilde->S[p].adj[1].index];
                entree_x = 0;
                entree_y = 1;
            } else { //Si on est arrivé à v, il n'a pas forcément de Demi arête à lui
                x.voisin.index = v; y.voisin.index = v;
            }
        } else { //On continue
            successeur_face_ext(*gtilde, &x, &entree_x);
            successeur_face_ext(*gtilde, &y, &entree_y);
        }
    }

    // printf("A la fin:\n");
    // print_position('x', x.voisin.index, x.voisin.lieu);
    // print_position('y', y.voisin.index, y.voisin.lieu);
}

void fusion_compo_biconnexes(graphe_comb* gtilde){
    /*La pile contient (du haut vers le bas):
        sortie_wc, wc, entree_w, w*/
    //print_pile(gtilde->P);
    int sortie_rc = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int rc = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int entree_r = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int r = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    if(entree_r == sortie_rc){ //on a la garantie que les sommets traversés ne sont pas ext actifs
        printf("\n---On a inverse le sens:\n");
        //On inverse le sens de la liste des demi-arêtes de rc
        int temp = gtilde->R[rc].lien[0].index;
        gtilde->R[rc].lien[0].index = gtilde->R[rc].lien[1].index;
        gtilde->R[rc].lien[1].index = temp;
        int pos = temp;
        while(pos != gtilde->R[rc].lien[0].index){
            temp = gtilde->A[pos].adj[0].index;
            gtilde->A[pos].adj[0].index = gtilde->A[pos].adj[1].index;
            gtilde->A[pos].adj[1].index = temp;
        } //pos est arrivée à la fin
        temp = gtilde->A[pos].adj[0].index;
        gtilde->A[pos].adj[0].index = gtilde->A[pos].adj[1].index;
        gtilde->A[pos].adj[1].index = temp;
        //On change le signe de l'arête entre (rc et c)
        while( gtilde->A[gtilde->A[pos].jumelle].voisin.index != rc){ //Ici, c = rc à cause de la manière dont on stocke les sommets racines
            pos = gtilde->A[pos].adj[1].index;
        }
        gtilde->A[pos].signe = -1;
        sortie_rc = 1 - sortie_rc;
        print_graphe_comb(*gtilde);
    }
    
    //On fusionne les composantes biconnexes, et on supprime le sommet racine
    //On redirige les arêtes
    int pos = gtilde->R[rc].lien[0].index;
    while(gtilde->A[pos].voisin.index != r){ //On dit que les arêtes pointent maintenant vers r
        //printf("Redirige l'arete %d vers %d\n", pos, r);
        gtilde->A[pos].voisin.index = r;
        gtilde->A[pos].voisin.lieu = dansS;

        pos = gtilde->A[pos].adj[1].index;
    }
    //On ne retire pas rc des racines pertinentes de r car deja fait dans la descente

    //On retire c de la liste des enfants DFS de r
    gtilde->S[r].enfantsDFS = suppression(gtilde->S[rc].p_parentDFS);

    //Enfin, on fusionne les listes d'adjacence dans le bon ordre 
    lien_adjacence rin = gtilde->S[r].adj[entree_r];
    lien_adjacence rcout = gtilde->R[rc].lien[sortie_rc];
    //printf("Les aretes sont: rin %d, suiv rin %d, rcout %d, prec rcout %d\n", rin.index, suiv_rin.index, rcout.index, prec_rcout.index);
    printf("Aretes de v%d: ", rc);
    print_aretes(*gtilde, dansR, rc);
    printf("Aretes de %d: ", r);
    print_aretes(*gtilde, dansS, r);

    //On distingue selon r_in
    if(entree_r == 1){
        gtilde->A[gtilde->S[r].adj[0].index].adj[0] = gtilde->R[rc].lien[1]; 
        gtilde->A[gtilde->R[rc].lien[1].index].adj[1] = gtilde->S[r].adj[0]; 
        gtilde->A[rin.index].adj[1] = rcout; //le suivant du r_in est le rc_out
        gtilde->A[rcout.index].adj[0] = rin; //le precedent du rc_out est le r_in
        gtilde->S[r].adj[1] = gtilde->R[rc].lien[1]; // on sort par là où on est sorti
    } else {
        gtilde->A[gtilde->S[r].adj[1].index].adj[1] = gtilde->R[rc].lien[0]; 
        gtilde->A[gtilde->R[rc].lien[0].index].adj[0] = gtilde->S[r].adj[1];  
        gtilde->A[rin.index].adj[0] = rcout; //le prec du r_in est le rc_out
        gtilde->A[rcout.index].adj[1] = rin; //le suiv du rc_out est le r_in
        gtilde->S[r].adj[0] = gtilde->R[rc].lien[0]; // on sort par là où on est sorti
    }

    print_graphe_comb_final(*gtilde);

}

bool pertinent(graphe_comb gtilde, int v, int s){
    return gtilde.S[s].flag_arete_retour == v || gtilde.S[s].racines_pertinentes != NULL;
}

bool actif_externe(graphe_comb gtilde, int v, int s){
    return gtilde.S[s].petit_ancetre < gtilde.S[v].DFI || (gtilde.S[s].enfantsDFS != NULL && gtilde.S[gtilde.S[s].enfantsDFS->val].point_min < gtilde.S[v].DFI);
}

bool inactif(graphe_comb gtilde, int v,  int s){
    return (! pertinent(gtilde, v, s)) && (! actif_externe(gtilde, v, s));
}

bool actif_interne(graphe_comb gtilde, int v, int s){
    return pertinent(gtilde, v, s) && ! actif_externe(gtilde,v,s);
}

void ajout_arete_retour(graphe_comb* gtilde, int c, int dir, int w, int entree_w){
    printf("On a %d aretes\n", gtilde->m);
    //On ajoute la demi arete incidente à vc
    gtilde->A[gtilde->m].adj[dir].lieu = dansA;
    gtilde->A[gtilde->m].adj[1-dir].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m + 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = false;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansR;
    gtilde->A[gtilde->m].voisin.index = c;
    //On la place dans la liste circulaire
    if(dir == 0){
        gtilde->A[gtilde->m].adj[0].index = gtilde->R[c].lien[1-dir].index; //ma précédente est l'autre côté
        gtilde->A[gtilde->m].adj[1].index = gtilde->R[c].lien[dir].index; //ma suivante est l'ancienne
        gtilde->A[gtilde->R[c].lien[dir].index].adj[0].index = gtilde->m; //la précédente de l'ancienne est moi
        gtilde->A[gtilde->R[c].lien[1-dir].index].adj[1].index = gtilde->m; //la suivante de la dernière est moi
    } else {
        gtilde->A[gtilde->m].adj[0].index = gtilde->R[c].lien[dir].index; //ma précédente est l'ancienne
        gtilde->A[gtilde->m].adj[1].index = gtilde->R[c].lien[1-dir].index; //ma suivante est la première
        gtilde->A[gtilde->R[c].lien[1-dir].index].adj[0].index = gtilde->m; //la précédente de la première est moi
        gtilde->A[gtilde->R[c].lien[dir].index].adj[1].index = gtilde->m; //la suivante de l'ancienne est moi
    }
    
    gtilde->R[c].lien[dir].index = gtilde->m; //je suis la nouvelle arete à suivre
    (gtilde->m)++;

    //On ajoute la demi arete incidente à w, idem
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m - 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = false;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansS;
    gtilde->A[gtilde->m].voisin.index = w;
    //On la place dans la liste circulaire
    if(entree_w == 0){
        gtilde->A[gtilde->m].adj[0].index = gtilde->S[w].adj[1-entree_w].index; //ma précédente est l'autre côté
        gtilde->A[gtilde->m].adj[1].index = gtilde->S[w].adj[entree_w].index; //ma suivante est l'ancienne
        gtilde->A[gtilde->S[w].adj[entree_w].index].adj[0].index = gtilde->m; //la précédente de l'ancienne est moi
        gtilde->A[gtilde->S[w].adj[1-entree_w].index].adj[1].index = gtilde->m; //la suivante de la dernière est moi
    } else {
        gtilde->A[gtilde->m].adj[0].index = gtilde->S[w].adj[entree_w].index; //ma précédente est l'ancienne
        gtilde->A[gtilde->m].adj[1].index = gtilde->S[w].adj[1-entree_w].index; //ma précédente est l'ancienne
        gtilde->A[gtilde->S[w].adj[1-entree_w].index].adj[0].index = gtilde->m; //la précédente de la première est moi
        gtilde->A[gtilde->S[w].adj[entree_w].index].adj[1].index = gtilde->m; //la suivante de l'ancienne est moi
    }
    
    gtilde->S[w].adj[entree_w].index = gtilde->m; //je suis la nouvelle arete à suivre
    (gtilde->m)++;
}

void ajout_arete_raccourci(graphe_comb* gtilde, int c, int dir, int w, int entree_w){
    //On ajoute la demi arete incidente à vc
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m + 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = false;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansR;
    gtilde->A[gtilde->m].voisin.index = c;
    //On la place dans la liste circulaire
    if(dir == 0){
        gtilde->A[gtilde->m].adj[0].index = gtilde->R[c].lien[1-dir].index; //ma précédente est l'autre côté
        gtilde->A[gtilde->m].adj[1].index = gtilde->R[c].lien[dir].index; //ma suivante est l'ancienne
        gtilde->A[gtilde->R[c].lien[dir].index].adj[0].index = gtilde->m; //la précédente de l'ancienne est moi
        gtilde->A[gtilde->R[c].lien[1-dir].index].adj[1].index = gtilde->m; //la suivante de la dernière est moi
    } else {
        gtilde->A[gtilde->m].adj[0].index = gtilde->R[c].lien[dir].index; //ma précédente est l'ancienne
        gtilde->A[gtilde->m].adj[1].index = gtilde->R[c].lien[1-dir].index; //ma suivante est la première
        gtilde->A[gtilde->R[c].lien[1-dir].index].adj[0].index = gtilde->m; //la précédente de la première est moi
        gtilde->A[gtilde->R[c].lien[dir].index].adj[1].index = gtilde->m; //la suivante de l'ancienne est moi
    }
    
    gtilde->R[c].lien[dir].index = gtilde->m; //je suis la nouvelle arete à suivre
    (gtilde->m)++;

    //On ajoute la demi arete incidente à w, idem
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m - 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = false;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansS;
    gtilde->A[gtilde->m].voisin.index = w;
    //On la place dans la liste circulaire
    if(entree_w == 0){
        gtilde->A[gtilde->m].adj[0].index = gtilde->S[w].adj[1-entree_w].index; //ma précédente est l'autre côté
        gtilde->A[gtilde->m].adj[1].index = gtilde->S[w].adj[entree_w].index; //ma suivante est l'ancienne
        gtilde->A[gtilde->S[w].adj[entree_w].index].adj[0].index = gtilde->m; //la précédente de l'ancienne est moi
        gtilde->A[gtilde->S[w].adj[1-entree_w].index].adj[1].index = gtilde->m; //la suivante de la dernière est moi
    } else {
        gtilde->A[gtilde->m].adj[0].index = gtilde->S[w].adj[entree_w].index; //ma précédente est l'ancienne
        gtilde->A[gtilde->m].adj[1].index = gtilde->S[w].adj[1-entree_w].index; //ma précédente est l'ancienne
        gtilde->A[gtilde->S[w].adj[1-entree_w].index].adj[0].index = gtilde->m; //la précédente de la première est moi
        gtilde->A[gtilde->S[w].adj[entree_w].index].adj[1].index = gtilde->m; //la suivante de l'ancienne est moi
    }
    
    gtilde->S[w].adj[entree_w].index = gtilde->m; //je suis la nouvelle arete à suivre
    (gtilde->m)++;
}

void descente(graphe_comb* gtilde, int c){
    printf("-----Descente depuis %d:\n\n", c);
    sommet_racine vc = gtilde->R[c];
    int v = vc.parent;
    //On vide la pile
    while (gtilde->P != NULL){
        gtilde->P = pop(gtilde->P);
    }

    for(int dir = 0; dir <2; dir++){ //On part dans les deux directions
        //printf("dir: %d\n", dir);
        demi_arete w = gtilde->A[gtilde->R[c].lien[1-dir].index]; //modif dir
        int entree_w = 1-dir;
        // char l;
        // if(w.voisin.lieu == dansS) l = 'S'; else l='R';
        // printf("Predecesseur: %d%c par %d\n", w.voisin.index,l, entree_w);
        successeur_face_ext(*gtilde, &w, &entree_w);
        // if(w.voisin.lieu == dansS) l = 'S'; else l='R';
        // printf("Successeur: %d%c par %d\n", w.voisin.index,l, entree_w);
        while(w.voisin.lieu != dansR || w.voisin.index != c){ //Tant qu'on n'est pas revenu au point de départ
            // printf("Entre dans le while: %d par %d,", w.voisin.index, entree_w);
            // if(w.voisin.lieu == dansR){printf("R\n");} else printf("S\n");
            //Invariant de boucle: w est une demi-arete liée à un sommet réel (pas racine)
            //w est la demi arête par laquelle on est entré dans le sommet
            // if(w.voisin.lieu != dansS) {printf("sommet virtuel dans le while\n"); return;}
            // printf("Inactif: %d\n", inactif(*gtilde, v, w.voisin.index));
            // printf("Pertinent: %d\n", pertinent(*gtilde, v, w.voisin.index));
            // printf("Actif Externe: %d\n", actif_externe(*gtilde, v, w.voisin.index));
            // print_graphe_comb(*gtilde);
            sommet sw = gtilde->S[w.voisin.index];

            if(sw.flag_arete_retour == v){ //On a trouvé une arete retour à ajouter
                while(gtilde->P != NULL){
                    printf("Fusion\n\n");
                    fusion_compo_biconnexes(gtilde);
                    //print_graphe_comb_final(*gtilde);
                }
                printf("Ajout arete retour v%d, %d\n\n", c, w.voisin.index);
                ajout_arete_retour(gtilde, c, dir, w.voisin.index, entree_w);
                gtilde->S[w.voisin.index].flag_arete_retour = gtilde->n;
                print_graphe_comb_final(*gtilde);
            }

            if(sw.racines_pertinentes != NULL){
                //On doit push w et entree w
                printf("Racines pertinentes trouvees\n");
                gtilde->P = push(gtilde->P, w.voisin.index);
                gtilde->P = push(gtilde->P, entree_w);

                int wc = gtilde->S[w.voisin.index].racines_pertinentes->val; //Premier élément des racines pertinentes
                gtilde->S[w.voisin.index].racines_pertinentes = suppression(gtilde->S[w.voisin.index].racines_pertinentes); //Pas dans le papier originel, source éventuelle de bug
                demi_arete x = gtilde->A[gtilde->R[wc].lien[1].index]; int entree_x = 1;
                demi_arete y = gtilde->A[gtilde->R[wc].lien[0].index]; int entree_y = 0;
                successeur_face_ext(*gtilde, &x, &entree_x);
                successeur_face_ext(*gtilde, &y, &entree_w);

                //JSP pk on fait ça
                if(actif_interne(*gtilde, v, x.voisin.index)){w = x; entree_w = entree_x;}
                else if(actif_interne(*gtilde, v, y.voisin.index)){w = y; entree_w = entree_y;}
                else if(pertinent(*gtilde, v, x.voisin.index)){w = x; entree_w = entree_x;}
                else {w = y; entree_w = entree_y;}

                int sortie_w; 
                if(w.adj[1].index == x.adj[1].index) sortie_w = 0;
                else sortie_w = 1;
                //On push wc et sortie_w
                gtilde->P = push(gtilde->P, wc);
                gtilde->P = push(gtilde->P, sortie_w);

            } else if(inactif(*gtilde, v, w.voisin.index)){
                printf("inactif\n");
                successeur_face_ext(*gtilde, &w, &entree_w);
            } else { //w est un sommet stoppant
                if(gtilde->P == NULL && gtilde->S[c].point_min < v){
                    printf("Ajout arete raccourci %d %d, %d %d\n\n", c, dir, w.voisin.index, entree_w);
                    ajout_arete_raccourci(gtilde, c, dir, w.voisin.index, entree_w);
                    print_graphe_comb_final(*gtilde);
                }
                break;
            }
        }
        if(gtilde->P != NULL) break;
    }
}

graphe_comb BoyerMyrvold(seq_dt seq){
    /*Prend une séquence DT et renvoie le graphe combinatoire après exécution de l'algo de Boyer Myrvold*/
        //Precalcul
    //Transformation en un graphe
    graphe g = conversion_seqDT(seq);
    printf("Fini conversion\n");
    //Simplification du graphe
    graphe g_simple = precalcul_graphe(g);
    printf("Fini simplification\n");
    //Initialisation du graphe combinatoire
    graphe_comb gtilde = init_graphe_comb(g_simple);
    printf("Fini initialisation\n");
    precalcul(g_simple, &gtilde);
    printf("Fini precalcul;\n");
    print_graphe_simple(g_simple);
        //Boucle principale
    for(int i = g.n-1; i>=0; i--){ //On traite les sommets par ordre DFI descendant
        int v = g_simple.DFI[i];
        printf("Boucle pour v: %d\n", v);
        //Pour chaque enfant de v
        print_list(gtilde.S[v].enfantsDFS);
        double_liste* debut = gtilde.S[v].enfantsDFS;
        double_liste* suivant = debut;
        bool premiere_iteration = true;
        while(suivant != NULL && (suivant != debut || premiere_iteration)){
            premiere_iteration = false;
            int c = suivant->val;
            //On crée un sommet virtuel
            gtilde.R[c].parent = v;
            gtilde.R[c].lien[0].lieu = dansA;
            gtilde.R[c].lien[0].index = gtilde.m;
            gtilde.R[c].lien[1].lieu = dansA;
            gtilde.R[c].lien[1].index = gtilde.m;
            //On ajoute l'arête de parcours (vc, c) comme 2 demi-arêtes
            gtilde.A[gtilde.m].adj[0].lieu = dansA;
            gtilde.A[gtilde.m].adj[0].index = gtilde.m; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m].adj[1].index = gtilde.m; 
            gtilde.A[gtilde.m].jumelle = gtilde.m+1;
            gtilde.A[gtilde.m].voisin.index = c;
            gtilde.A[gtilde.m].voisin.lieu = dansR; //relie à vc
            gtilde.A[gtilde.m].type = 1; //initialisé à 1
            gtilde.A[gtilde.m].reelle = true;
            //gtilde.A[gtilde.m].signe = A FAIRE: pour se rappeler du type des aretes
            gtilde.A[gtilde.m+1].adj[0].lieu = dansA;
            gtilde.A[gtilde.m+1].adj[0].index = gtilde.m+1; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m+1].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m+1].adj[1].index = gtilde.m+1; 
            gtilde.A[gtilde.m+1].jumelle = gtilde.m;
            gtilde.A[gtilde.m+1].voisin.index = c;
            gtilde.A[gtilde.m+1].voisin.lieu = dansS; //relie à c
            gtilde.A[gtilde.m+1].type = 1; //initialisé à 1
            gtilde.A[gtilde.m+1].reelle = true;
            //gtilde.A[gtilde.m+1].signe = A FAIRE: pour se rappeler du type des aretes

            //On marque la nouvelle demi-arete comme incidente à c
            gtilde.S[c].adj[0].index = gtilde.m+1;
            gtilde.S[c].adj[0].lieu = dansA;
            gtilde.S[c].adj[1].index = gtilde.m+1;
            gtilde.S[c].adj[1].lieu = dansA;

            //On vient d'ajouter une arête: on incrémente le compteur
            gtilde.m++; gtilde.m++;
            printf("Cree le sommet %dR\n", c);
            suivant = suivant->suiv;
        }

        //Pour chaque arete retour entre v et un de ses descendants w
        for(int j = 0; j<g_simple.degre[v]; j++){
            if(g_simple.type[v][j] < 0 && gtilde.S[g_simple.adj[v][j]].DFI > gtilde.S[v].DFI){ //Si arete retour et descendant
                int w = g_simple.adj[v][j];
                montee(&gtilde, w, v); 
            }
        }

        //Pour chaque enfant de v
        debut = gtilde.S[v].enfantsDFS;
        suivant = debut;
        premiere_iteration = true;
        while(suivant != NULL && (suivant != debut || premiere_iteration)){
            premiere_iteration = false;
            int c = suivant->val;
            descente(&gtilde, c);
            suivant = suivant->suiv;
        }

        //On teste si la descente a bien réussi?
    }


    return gtilde;
}

//Fonctions de test
void test_conversion(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    graphe noeud_wiki_simple = precalcul_graphe(gnoeud_wiki);
    printf("Sequence:\n");
    print_seq_dt(&noeud_wiki);
    printf("Multigraphe:\n");
    print_multigraphe(gnoeud_wiki);
    printf("Graphe simple:\n");
    print_graphe_simple(noeud_wiki_simple);
    return;
}

void test_precalcul(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    graphe noeud_wiki_simple = precalcul_graphe(gnoeud_wiki);
    graphe_comb gtilde = init_graphe_comb(noeud_wiki_simple);
    precalcul(noeud_wiki_simple, &gtilde);
    printf("Precalcul:\n");
    print_graphe_comb_initial(gtilde);
}

void test_algo_wiki(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    graphe noeud_wiki_simple = precalcul_graphe(gnoeud_wiki);
    graphe_comb gtilde = init_graphe_comb(noeud_wiki_simple);
    precalcul(noeud_wiki_simple, &gtilde);
    //printf("Precalcul:\n");
    //print_graphe_comb_initial(gtilde);
    //On modifie gtilde pour arriver à l'état où l'appel à montée(gtilde, 1, 4)
    
    //On crée un sommet virtuel v4
    gtilde.R[4].parent = 2;
    gtilde.R[4].lien[0].lieu = dansA;
    gtilde.R[4].lien[0].index = 0;
    gtilde.R[4].lien[1].lieu = dansA;
    gtilde.R[4].lien[1].index = 0;
    
    //On ajoute l'arête (2,4)
    gtilde.A[0].adj[0].lieu = dansA;
    gtilde.A[0].adj[0].index = 0; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[0].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[0].adj[1].index = 0; 
    gtilde.A[0].jumelle = 1;
    gtilde.A[0].voisin.index = 4;
    gtilde.A[0].voisin.lieu = dansR; //relie à v4
    gtilde.A[0].type = 1; //initialisé à 1
    gtilde.A[0].reelle = true;
    gtilde.A[1].adj[0].lieu = dansA;
    gtilde.A[1].adj[0].index = 1; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[1].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[1].adj[1].index = 1; 
    gtilde.A[1].jumelle = 0;
    gtilde.A[1].voisin.index = 4;
    gtilde.A[1].voisin.lieu = dansS; //relie à 4
    gtilde.A[1].type = 1; //initialisé à 1
    gtilde.A[1].reelle = true;

    //On marque la nouvelle demi-arete comme incidente à 4
    gtilde.S[4].adj[0].index = 1;
    gtilde.S[4].adj[0].lieu = dansA;
    gtilde.S[4].adj[1].index = 1;
    gtilde.S[4].adj[1].lieu = dansA;

    //On crée un sommet virtuel v2
    gtilde.R[2].parent = 1;
    gtilde.R[2].lien[0].lieu = dansA;
    gtilde.R[2].lien[0].index = 2;
    gtilde.R[2].lien[1].lieu = dansA;
    gtilde.R[2].lien[1].index = 2;
    
    //On ajoute l'arête (1,2)
    gtilde.A[2].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[2].adj[1].index = 2; 
    gtilde.A[2].jumelle = 3;
    gtilde.A[2].voisin.index = 2;
    gtilde.A[2].voisin.lieu = dansR; //relie à v2
    gtilde.A[2].type = 1; //initialisé à 1
    gtilde.A[2].reelle = true;
    gtilde.A[3].adj[0].lieu = dansA;
    gtilde.A[3].adj[0].index = 3; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[3].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[3].adj[1].index = 3; 
    gtilde.A[3].jumelle = 2;
    gtilde.A[3].voisin.index = 2;
    gtilde.A[3].voisin.lieu = dansS; //relie à 2
    gtilde.A[3].type = 1; //initialisé à 1
    gtilde.A[3].reelle = true;

    //On marque la nouvelle demi-arete comme incidente à 4
    gtilde.S[2].adj[0].index = 3;
    gtilde.S[2].adj[0].lieu = dansA;
    gtilde.S[2].adj[1].index = 3;
    gtilde.S[2].adj[1].lieu = dansA;

    //On crée un sommet virtuel v3
    gtilde.R[3].parent = 4;
    gtilde.R[3].lien[0].lieu = dansA;
    gtilde.R[3].lien[0].index = 4;
    gtilde.R[3].lien[1].lieu = dansA;
    gtilde.R[3].lien[1].index = 4;
    
    //On ajoute l'arête (4,3)
    gtilde.A[4].adj[0].lieu = dansA;
    gtilde.A[4].adj[0].index = 4; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[4].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[4].adj[1].index = 4; 
    gtilde.A[4].jumelle = 5;
    gtilde.A[4].voisin.index = 3;
    gtilde.A[4].voisin.lieu = dansR; //relie à v3
    gtilde.A[4].type = 1; //initialisé à 1
    gtilde.A[4].reelle = true;
    gtilde.A[5].adj[0].lieu = dansA;
    gtilde.A[5].adj[0].index = 5; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[5].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[5].adj[1].index = 5; 
    gtilde.A[5].jumelle = 4;
    gtilde.A[5].voisin.index = 3;
    gtilde.A[5].voisin.lieu = dansS; //relie à 3
    gtilde.A[5].type = 1; //initialisé à 1
    gtilde.A[5].reelle = true;

    //On marque la nouvelle demi-arete comme incidente à 3
    gtilde.S[3].adj[0].index = 5;
    gtilde.S[3].adj[0].lieu = dansA;
    gtilde.S[3].adj[1].index = 5;
    gtilde.S[3].adj[1].lieu = dansA;

    //On crée un sommet virtuel v5
    gtilde.R[5].parent = 4;
    gtilde.R[5].lien[0].lieu = dansA;
    gtilde.R[5].lien[0].index = 6;
    gtilde.R[5].lien[1].lieu = dansA;
    gtilde.R[5].lien[1].index = 6;
    
    //On ajoute l'arête (4,5)
    gtilde.A[6].adj[0].lieu = dansA;
    gtilde.A[6].adj[0].index = 6; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[6].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[6].adj[1].index = 6; 
    gtilde.A[6].jumelle = 7;
    gtilde.A[6].voisin.index = 5;
    gtilde.A[6].voisin.lieu = dansR; //relie à v5
    gtilde.A[6].type = 1; //initialisé à 1
    gtilde.A[6].reelle = true;
    gtilde.A[7].adj[0].lieu = dansA;
    gtilde.A[7].adj[0].index = 7; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[7].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[7].adj[1].index = 7; 
    gtilde.A[7].jumelle = 6;
    gtilde.A[7].voisin.index = 5;
    gtilde.A[7].voisin.lieu = dansS; //relie à 5
    gtilde.A[7].type = 1; //initialisé à 1
    gtilde.A[7].reelle = true;

    //On marque la nouvelle demi-arete comme incidente à 5
    gtilde.S[5].adj[0].index = 7;
    gtilde.S[5].adj[0].lieu = dansA;
    gtilde.S[5].adj[1].index = 7;
    gtilde.S[5].adj[1].lieu = dansA;

    gtilde.m = 8;

    //print_graphe_comb(gtilde);
    
    //On teste la montée
    montee(&gtilde, 4, 1);
    montee(&gtilde, 5, 1);
    print_graphe_comb_final(gtilde);
    

    //On teste la descente
    descente(&gtilde, 2);
    printf("---------Apres la premiere descente: --------------");
    print_graphe_comb_final(gtilde);

    //OMG jusque là tout va bien?

    //On crée un sommet virtuel v1
    gtilde.R[1].parent = 0;
    gtilde.R[1].lien[0].lieu = dansA;
    gtilde.R[1].lien[0].index = 16;
    gtilde.R[1].lien[1].lieu = dansA;
    gtilde.R[1].lien[1].index = 16;
    
    //On ajoute l'arête (0,1)
    gtilde.A[16].adj[0].lieu = dansA;
    gtilde.A[16].adj[0].index = 16; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[16].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[16].adj[1].index = 16; 
    gtilde.A[16].jumelle = 17;
    gtilde.A[16].voisin.index = 1;
    gtilde.A[16].voisin.lieu = dansR; //relie à v1
    gtilde.A[16].type = 1; //initialisé à 1
    gtilde.A[16].reelle = true;
    gtilde.A[17].adj[0].lieu = dansA;
    gtilde.A[17].adj[0].index = 17; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
    gtilde.A[17].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
    gtilde.A[17].adj[1].index = 17; 
    gtilde.A[17].jumelle = 16;
    gtilde.A[17].voisin.index = 1;
    gtilde.A[17].voisin.lieu = dansS; //relie à 1
    gtilde.A[17].type = 1; //initialisé à 1
    gtilde.A[17].reelle = true;

    //On marque la nouvelle demi-arete comme incidente à 1
    gtilde.S[1].adj[0].index = 17;
    gtilde.S[1].adj[0].lieu = dansA;
    gtilde.S[1].adj[1].index = 17;
    gtilde.S[1].adj[1].lieu = dansA;
    gtilde.m++;
    gtilde.m++;
    //On teste la montée/descente
    montee(&gtilde, 2, 0);
    print_graphe_comb_final(gtilde);
    montee(&gtilde, 3, 0);
    printf("---------Apres la deuxieme montee: --------------\n");
    print_graphe_comb_final(gtilde);
    descente(&gtilde, 1);
    print_graphe_comb_final(gtilde);

}

//PROBLEME DANS AJOUT ARETE

int main(){
    //test_conversion();
    //test_precalcul();
    test_algo_wiki();
    // int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    // seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    // graphe_comb g = BoyerMyrvold(noeud_wiki);
    // printf("Resultat final:\n");
    // print_graphe_comb_final(g);


    printf("\nok!\n");
    return 0;
}