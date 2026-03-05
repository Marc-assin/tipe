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
    lien_adjacence* adj; //tableau de taille 2: relie aux demi arêtes de la face extérieure
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
    lien_adjacence* lien; //tableau de taille 2: demi arêtes de part et d'autre
    int parent; //indice dans S
    double_liste* pertinence; //lien vers la cellule de racines_pertinentes éventuelle de son parent
};

typedef struct sommet_racine sommet_racine;

struct demi_arete{
    lien_adjacence* adj; //tableau de taille 2: liens vers les aretes de la liste d'adjacence sur sommet
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

int signe(int k){
    if (k<0) return (-1); 
    return 1;
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
    if(lst ==  NULL) return;
    printf("%d, ", lst->val);
    double_liste* suiv = lst->suiv;
    while(suiv != lst){
        printf("%d, ", suiv->val);
        suiv = suiv->suiv;
    }
    
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
//L'initialisation de m et des arêtes est laissé au DFS
graphe_comb init_graphe_comb(graphe g){
    sommet* sommets = malloc(g.n*sizeof(sommet));
    sommet_racine* R = malloc(g.n*sizeof(sommet_racine));
    demi_arete* A = malloc(8*g.n*sizeof(demi_arete));
    pile* P = init_pile();
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
    printf("fini parcours\n");
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

void successeur_face_ext(graphe_comb gtilde, demi_arete* w, int* entree_w){
   demi_arete e = gtilde.A[w->jumelle];
   int x = e.voisin.index;
   if(e.voisin.lieu == dansS && gtilde.S[x].adj[0].index != gtilde.S[x].adj[1].index){ //Si le sommet n'est pas d'arité 1
        if(gtilde.A[gtilde.A[gtilde.S[x].adj[0].index].jumelle].jumelle == w->jumelle) *entree_w = 1; //Si la demi arete 0 est celle d'où on vient
        else *entree_w = 0;
   } else if(e.voisin.lieu == dansR && gtilde.R[x].lien[0].index != gtilde.R[x].lien[1].index){
        if(gtilde.A[gtilde.A[gtilde.R[x].lien[0].index].jumelle].jumelle == w->jumelle) *entree_w = 1; //Si la demi arete 0 est celle d'où on vient
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

    while(x.voisin.index != v && y.voisin.index != v && gtilde->S[x.voisin.index].visite != v 
            && gtilde->S[y.voisin.index].visite != v){ //Tant qu'on n'est pas arrivé à v, et qu'on n'est pas déjà passé par là
        if(x.voisin.lieu == dansS) gtilde->S[x.voisin.index].visite = v; //On a visité
        if(y.voisin.lieu == dansS) gtilde->S[y.voisin.index].visite = v; //On a visité
        demi_arete z = {.adj = NULL};
        if(x.voisin.lieu == dansR){
            z = x;
        } else if (y.voisin.lieu == dansR){ 
            z = y;
        }
        if(z.adj == NULL){ //On a trouvé le sommet racine de notre composante biconnexe actuelle, on cherche donc à continuer notre remontée
            //On retrouve le parent lié à notre sommet racine
            int c = x.voisin.index;
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

void fusion_compo_biconnexes(graphe_comb* gtilde){
    /*La pile contient (du haut vers le bas):
        sortie_wc, wc, entree_w, w*/
    int sortie_rc = top(gtilde->P);
    pop(gtilde->P);
    int rc = top(gtilde->P);
    pop(gtilde->P);
    int entree_r = top(gtilde->P);
    pop(gtilde->P);
    int r = top(gtilde->P);
    pop(gtilde->P);

    if(entree_r == sortie_rc){
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
    }

    //On fusionne les composantes biconnexes, et on supprime le sommet racine
    int pos = gtilde->R[rc].lien[0].index;
    while(gtilde->A[gtilde->A[pos].jumelle].voisin.index != r){ //On dit que les arêtes pointent maintenant vers r
        gtilde->A[gtilde->A[pos].jumelle].voisin.index = r;
        pos = gtilde->A[pos].adj[1].index;
    }
    //On ne retire pas rc des racines pertinentes de r car deja fait dans la descente
    //On retire c de la liste des enfants DFS de r
    gtilde->S[r].enfantsDFS = suppression(gtilde->S[rc].p_parentDFS);
    //Enfin, on fusionne les listes d'adjacence dans le bon ordre 
    //Louche un peu sur la fin
    int temp = gtilde->A[gtilde->S[r].adj[entree_r].index].adj[1].index; //le suivant du r_in
    gtilde->A[gtilde->A[gtilde->S[rc].adj[sortie_rc].index].adj[0].index].adj[1].index = temp; //le suivant du précédent de rc_out est le suivant du r_in
    gtilde->A[temp].adj[0].index =  gtilde->A[gtilde->S[rc].adj[sortie_rc].index].adj[0].index; //le précédent du suivant de r_in est le précédent de rc_out
    gtilde->A[gtilde->S[r].adj[entree_r].index].adj[1].index = gtilde->S[rc].adj[sortie_rc].index; //le suivant du r_in est le rc_out
    gtilde->A[gtilde->S[rc].adj[sortie_rc].index].adj[0].index = gtilde->S[r].adj[entree_r].index; //le precedent du rc_out est le r_in
    gtilde->S[r].adj[entree_r].index = gtilde->S[rc].adj[1 - sortie_rc].index; // on pointe vers l'autre côté de la face liée à rc

}

//Il reste: ajout arete retour,  ajour arete raccourci
bool pertinent(graphe_comb gtilde, int v, int s){
    return gtilde.S[s].flag_arete_retour == v || gtilde.S[s].racines_pertinentes != NULL;
}

bool actif_externe(graphe_comb gtilde, int v, int s){
    return gtilde.S[s].petit_ancetre < gtilde.S[v].DFI || gtilde.S[s].enfantsDFS != NULL && gtilde.S[gtilde.S[s].enfantsDFS->val].point_min < gtilde.S[v].DFI;
}

bool inactif(graphe_comb gtilde, int v,  int s){
    return ! pertinent(gtilde, v, s) && ! actif_externe(gtilde, v, s);
}

bool actif_interne(graphe_comb gtilde, int v, int s){
    return pertinent(gtilde, v, s) && ! actif_externe(gtilde,v,s);
}

void ajout_arete_retour(graphe_comb* gtilde, int vc, int dir, int w, int entree_w){
    //On ajoute la demi arete incidente à vc
    gtilde->A[gtilde->m].adj[0].index = gtilde->R[vc].lien[dir].index;
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].index = gtilde->A[gtilde->R[vc].lien[dir].index].adj[1].index;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m + 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = true;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansR;
    gtilde->A[gtilde->m].voisin.index = vc;
    //On la place dans la liste circulaire
    gtilde->A[gtilde->A[gtilde->m].adj[0].index].adj[1].index = gtilde->m; //la suivante de la précédente est moi
    gtilde->A[gtilde->A[gtilde->m].adj[1].index].adj[0].index = gtilde->m; //la précédente de la suivante est moi
    gtilde->R[vc].lien[dir].index = gtilde->m; //je suis la nouvelle arete à suivre
    gtilde->m++;
    //On ajoute la demi arete incidente à w
    gtilde->A[gtilde->m].adj[0].index = gtilde->S[w].adj[entree_w].index;
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].index = gtilde->A[gtilde->S[w].adj[entree_w].index].adj[1].index;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m - 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = true;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansS;
    gtilde->A[gtilde->m].voisin.index = w;
    //On la place dans la liste circulaire
    gtilde->A[gtilde->A[gtilde->m].adj[0].index].adj[1].index = gtilde->m; //la suivante de la précédente est moi
    gtilde->A[gtilde->A[gtilde->m].adj[1].index].adj[0].index = gtilde->m; //la précédente de la suivante est moi
    gtilde->S[w].adj[entree_w].index = gtilde->m; //je suis la nouvelle arete à suivre
    gtilde->m++;
}

void ajout_arete_raccourci(graphe_comb* gtilde, int c, int dir, int w, int entree_w){
    //On ajoute la demi arete incidente à vc
    gtilde->A[gtilde->m].adj[0].index = gtilde->R[c].lien[dir].index;
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].index = gtilde->A[gtilde->R[c].lien[dir].index].adj[1].index;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m + 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = false;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansR;
    gtilde->A[gtilde->m].voisin.index = c;
    //On la place dans la liste circulaire
    gtilde->A[gtilde->A[gtilde->m].adj[0].index].adj[1].index = gtilde->m; //la suivante de la précédente est moi
    gtilde->A[gtilde->A[gtilde->m].adj[1].index].adj[0].index = gtilde->m; //la précédente de la suivante est moi
    gtilde->R[c].lien[dir].index = gtilde->m; //je suis la nouvelle arete à suivre
    gtilde->m++;
    //On ajoute la demi arete incidente à w
    gtilde->A[gtilde->m].adj[0].index = gtilde->S[w].adj[entree_w].index;
    gtilde->A[gtilde->m].adj[0].lieu = dansA;
    gtilde->A[gtilde->m].adj[1].index = gtilde->A[gtilde->S[w].adj[entree_w].index].adj[1].index;
    gtilde->A[gtilde->m].adj[1].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m - 1;
    gtilde->A[gtilde->m].signe = 1;
    gtilde->A[gtilde->m].reelle = false;
    //A FAIRE gtilde->A[gtilde->m].type = 
    gtilde->A[gtilde->m].voisin.lieu = dansS;
    gtilde->A[gtilde->m].voisin.index = w;
    //On la place dans la liste circulaire
    gtilde->A[gtilde->A[gtilde->m].adj[0].index].adj[1].index = gtilde->m; //la suivante de la précédente est moi
    gtilde->A[gtilde->A[gtilde->m].adj[1].index].adj[0].index = gtilde->m; //la précédente de la suivante est moi
    gtilde->S[w].adj[entree_w].index = gtilde->m; //je suis la nouvelle arete à suivre
    gtilde->m++;
}

void descente(graphe_comb* gtilde, int c){
    sommet_racine vc = gtilde->R[c];
    int v = vc.parent;
    //On vide la pile
    while (gtilde->P != NULL){
        gtilde->P = pop(gtilde->P);
    }

    for(int dir = 0; dir <2; dir++){ //On part dans les deux directions
        demi_arete w = gtilde->A[gtilde->R[c].lien[1-dir].index]; 
        int entree_w = 1-dir;
        successeur_face_ext(*gtilde, &w, &entree_w);
        
        while(w.voisin.lieu != dansR && w.voisin.index != c){ //Tant qu'on n'est pas revenu au point de départ
            //Invariant de boucle: w est une demi-arete liée à un sommet réel (pas racine)
            if(w.voisin.lieu != dansS) printf("sommet virtuel dans le while"); return;
            sommet sw = gtilde->S[w.voisin.index];

            if(sw.flag_arete_retour == v){
                while(gtilde->P != NULL){
                    fusion_compo_biconnexes(gtilde);
                }
                ajout_arete_retour(gtilde, c, dir, w.voisin.index, entree_w);
                gtilde->S[w.voisin.index].flag_arete_retour = gtilde->n;
            }

            if(sw.racines_pertinentes != NULL){
                //On doit push w et entree w
                push(gtilde->P, w.voisin.index);
                push(gtilde->P, entree_w);

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
                push(gtilde->P, wc);
                push(gtilde->P, sortie_w);

            } else if(inactif(*gtilde, v, w.voisin.index)){
                successeur_face_ext(*gtilde, &w, &entree_w);
            } else { //w est un sommet stoppant
                if(gtilde->P == NULL && gtilde->S[c].point_min < v){
                    ajout_arete_raccourci(gtilde, c, dir, w.voisin.index, entree_w);
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
            sommet_racine vc = {.lien = lien, .parent = v, .pertinence = NULL};
            gtilde.R[c] = vc;
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
            gtilde.A[gtilde.m].voisin.index = c;
            gtilde.A[gtilde.m].voisin.lieu = dansR; //relie à c
            gtilde.A[gtilde.m+1].type = 1; //initialisé à 1
            gtilde.A[gtilde.m].reelle = true;
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

        //Pour chaque enfant de v
        double_liste* debut = gtilde.S[v].enfantsDFS;
        double_liste* suivant = debut;
        while(suivant != NULL && suivant->suiv != debut){
            int c = suivant->val;
            descente(&gtilde, c);
        }

        //On teste si la descente a bien réussi

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

void test_montee(){

}

/*Ce qui a été testé:
conversion, precalcul*/

//Bug potentiel: dans le papier v est le numéro DFI

int main(){
    test_conversion();
    test_precalcul();
    printf("\nok!\n");
    return 0;
}