#include "knot_gen.c"
#include <stdio.h>
#include <assert.h>
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
    assert(lst != NULL);
	if (lst->prec == lst || lst->suiv == lst) {
		assert(lst->prec == lst);
		assert(lst->suiv == lst);

		free(lst);
		//printf("A renvoye NULL\n");
		return NULL;
	}
    lst->prec->suiv = lst->suiv;
    lst->suiv->prec = lst->prec;
    double_liste* res = lst->suiv;
    free(lst);
    return res;
};

//Verification A FAIRE: supprimer
void check_list(double_liste* lst)
{
    if (lst == NULL)
        return;

    double_liste* cur = lst;

    do {
        assert(cur->suiv != NULL);
        assert(cur->prec != NULL);

        assert(cur->suiv->prec == cur);
        assert(cur->prec->suiv == cur);

        cur = cur->suiv;

    } while (cur != lst);
}

//Graphe d'entrée
struct graphe{
    int n;
    int** adj; //tableau de tableaux de taille au plus 4 (4 si multigraphe, moins si graphe simplifié)
    int** signes; //tableau indiquant si l'arête passe au-dessus (>0), en-dessous (<0), ou 0 si entre deux sommet-arete
    //données pour les graphes simplifiés:
    int** type; //tableau indiquant si l'arête est une arete directe (>0) ou retour (<0)
    int* type_sommet; //Indique si le sommet représente: un sommet = 0, une arete simple = 1, une arete double = 2 
    int* DFI; //tableau indiquant l'ordre du DFS: les sommets dans l'ordre
    int* indices_DFI; //L'indice DFI de chaque sommet
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
    int type; //Sommet réel = 0, sommet-arete simple = 1, double = 2
    int signe; //Doit-on retourner les listes des descendants
    lien_adjacence adj[2]; //tableau de taille 2: relie aux demi arêtes de la face extérieure
    int parentDFS;
    int DFI;
    int petit_ancetre; //DFI de l'ancetre direct de plus petit indice
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
    lien_adjacence voisin; //sommet correspondant
    //int type; // 1 ou -1 selon le sens de rotation, 0 si n'est pas une arete de parcours
    bool reelle; //arête réelle ou raccourci
};

typedef struct demi_arete demi_arete;

struct graphe_comb{
    int n; //nombre de sommets
    int m; //nombre d'arêtes
    sommet* S;
    sommet_racine* R; //sommets racines correspondants aux enfants
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
    else {
        e = gtilde.A[gtilde.R[x].lien[1-*entree_w].index];
        //printf("w: %d, e:%d\n", gtilde.R[x].lien[*entree_w].index, gtilde.R[x].lien[1-*entree_w].index);
    }


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
}

//Fonctions d'affichage
void print_graphe_simple(graphe g){
    for(int i=0; i<g.n; i++){
        printf("%d: ", i);
        for(int j = 0; j<4; j++){
            printf(" (%d: type %d) ,", g.adj[i][j], g.type[i][j]);
        }
        printf("\n");
    }
}

void print_graphe_final(graphe g){
    for(int i=0; i<g.n; i++){
        printf("%d: ", i);
        for(int j = 0; j<4; j++){
            printf("%d, ", g.adj[i][j]);
        }
        printf("\n");
    }
}

void print_res_BM(graphe g){
    for(int i=0; i<g.n; i++){
        printf("%d: ", i);
        for(int j = 0; j<4; j++){
            printf(" %d ,", g.adj[i][j]);
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
		if(g.S[s].adj[0].index == -1) return;
        if(g.A[g.A[g.S[s].adj[0].index].jumelle].voisin.lieu == dansS) lieu0 = 'S'; else lieu0 = 'R';
        if(g.A[g.A[g.S[s].adj[1].index].jumelle].voisin.lieu == dansS) lieu1 = 'S'; else lieu1 = 'R';
        printf("(%d, %d %c) <- %d -> (%d, %d %c) ", g.S[s].adj[0].index, g.A[g.A[g.S[s].adj[0].index].jumelle].voisin.index, lieu0, 
            s, g.S[s].adj[1].index, g.A[g.A[g.S[s].adj[1].index].jumelle].voisin.index, lieu1);
    } else if(l == dansR){
		if(g.R[s].lien[0].index == -1) return;
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

void print_pile_propre(pile* P){
    print_pile(P);
    printf("\n");
}

void print_position(char nom, int x, lieu_adjacence l){
    char c;
    if(l==dansS) c = 'S'; else if(l==dansR) c = 'R'; else c = 'A';
    printf("%c: %d %c\n", nom, x, c);
}

void print_aretes(graphe_comb g, lieu_adjacence l, int s){
    char lieu, reelle;
    if(l==dansS){
        int w = g.S[s].adj[0].index;
        if (w == -1) return;
        if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
        if(g.A[w].reelle) reelle = 'V'; else reelle = 'F';
        printf("(%d %c, %d %c) ->", w, reelle, g.A[g.A[w].jumelle].voisin.index, lieu);
        w = g.A[w].adj[1].index;
        while(w != g.S[s].adj[0].index){
            if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
            if(g.A[w].reelle) reelle = 'V'; else reelle = 'F';
            printf("(%d %c, %d %c) ->", w, reelle, g.A[g.A[w].jumelle].voisin.index, lieu);
            w = g.A[w].adj[1].index;
        }
    } else {
        int w = g.R[s].lien[0].index;
        if (w == -1) return;
        if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
        if(g.A[w].reelle) reelle = 'V'; else reelle = 'F';
        printf("(%d %c, %d %c) ->", w, reelle, g.A[g.A[w].jumelle].voisin.index, lieu);
        w = g.A[w].adj[1].index;
        while(w != g.R[s].lien[0].index){
            if(g.A[g.A[w].jumelle].voisin.lieu == dansS) lieu = 'S'; else lieu = 'R';
            if(g.A[w].reelle) reelle = 'V'; else reelle = 'F';
            printf("(%d %c, %d %c) ->", w, reelle, g.A[g.A[w].jumelle].voisin.index, lieu);
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
        printf("Aretes face exterieure: ");
        print_aretes_ext(g, dansS, i);
        printf("Racines pertinentes: ");
        print_list(g.S[i].racines_pertinentes);
        printf("enfantsDFS: ");
        print_list(g.S[i].enfantsDFS);
    }
    printf("\nSommets virtuels:\n");
    for(int i = 0; i<g.n; i++){
        if(g.R[i].parent != -1){
            printf("\nv%d:\n", i);
            printf("Aretes:");
            print_aretes(g, dansR, i);
            printf("Aretes face exterieure: ");
            print_aretes_ext(g, dansR, i);
			printf("Pertinence: ");
			if(g.R[i].pertinence == NULL) printf("NULL\n");
			else printf("pertinent\n");
        }
    }
    //print_aretes(g, dansR, 1);
    printf("\n\n");
}

//Precalcul

//Convertit une sequence DT en un multigraphe
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

//Debuggé
graphe simplifier_graphe(graphe g, int* associations){
    /*Prend un graphe de noeud
    Ajoute deux sommets pour chaque arête
    Met des aretes entre: 
     - une arete et le sommet qu'elle relie
     - deux sommets correspondant à la même arete
     - des aretes consécutives
     - retient qui est une arete double
     - retient le signe d'une arete
    Resultat contient encore des -1 !
    Fait des associations entre des sommet-arêtes d'une même arête*/

    int** adj = malloc(5*(g.n)*sizeof(int*));
    int* DFI = malloc(5*(g.n)*sizeof(int));
    int** types = malloc(5*(g.n)*sizeof(int*));
    int* type_sommets = malloc(5*(g.n)*sizeof(int));
    int* indices_DFI = malloc(5*(g.n)*sizeof(int));
    int nb_sommets = g.n;

    //On initialise les sommets
    for(int s = 0; s<g.n; s++){
        adj[s] = malloc(4*sizeof(int));
        types[s] = malloc(4*sizeof(int));
        type_sommets[s] = 0;
        for(int a = 0; a<4; a++){
            types[s][a] = -1;
        }
        associations[s] = -1;
    }


    for(int s = 0; s<g.n; s++){
        // printf("Traitement sommet %d\n", s);
        // fflush(stdout);
        for(int j = 0; j<4; j++){
            // printf("Traitement arete %d, nb sommets: %d\n", j, nb_sommets);
            // fflush(stdout);
            if(g.adj[s][j] != -1){ //On n'a pas déjà traité l'arête
                adj[nb_sommets] = malloc(4*sizeof(int));
                types[nb_sommets] = malloc(4*sizeof(int));
                adj[nb_sommets+1] = malloc(4*sizeof(int));
                types[nb_sommets+1] = malloc(4*sizeof(int));

                int j_voisin = -1; //le numéro d'arête chez le voisin
                for(int k=0; k<4; k++){
                    if(g.adj[g.adj[s][j]][k] == s) j_voisin = k;
                }
                if(j_voisin == -1){
                    printf("Pas chez le voisin????\n");
                    assert(false);
                }
                // printf("j vois ok\n");
                // fflush(stdout);
                //On teste si l'arête n'est pas en réalité double
                if(j>0 && g.adj[s][j] == g.adj[s][j-1]){
                    type_sommets[nb_sommets] = -1; //N'est relié à rien
                    type_sommets[nb_sommets+1] = -1;
                    type_sommets[adj[s][j-1]] = 2;
                    type_sommets[adj[adj[s][j-1]][2]] = 2;
                    associations[nb_sommets] = -adj[s][j-1];
                    associations[nb_sommets+1] = -adj[adj[s][j-1]][2];
                    adj[nb_sommets][2] = -1;
                    types[nb_sommets][2] = 0;
                    adj[nb_sommets+1][2] = -1;
                    types[nb_sommets+1][2] = 0;
                } else if(j>0 && g.adj[s][j] == g.adj[s][0]) {
                    type_sommets[nb_sommets] = -1; //N'est relié à rien
                    type_sommets[nb_sommets+1] = -1;
                    type_sommets[adj[s][0]] = 2;
                    type_sommets[adj[adj[s][0]][2]] = 2;
                    associations[nb_sommets] = -adj[s][0];
                    associations[nb_sommets+1] = -adj[adj[s][0]][2];
                    adj[nb_sommets][2] = -1;
                    types[nb_sommets][2] = 0;
                    adj[nb_sommets+1][2] = -1;
                    types[nb_sommets+1][2] = 0;
                } else{ //Il y a réellement une arête à créer
                    // printf("Arete simple\n");
                    // fflush(stdout);
                    type_sommets[nb_sommets] = 1;
                    type_sommets[nb_sommets+1] = 1;
                    adj[nb_sommets][2] = nb_sommets+1;
                    types[nb_sommets][2] = -1;
                    adj[nb_sommets+1][2] = nb_sommets;
                    types[nb_sommets+1][2] = -1;
                    associations[nb_sommets] = nb_sommets+1;
                    associations[nb_sommets+1] = nb_sommets;
                }
                adj[s][j] = nb_sommets;
                adj[nb_sommets][0] = s;
                types[nb_sommets][0] = -1;
                adj[g.adj[s][j]][j_voisin] = nb_sommets+1;
                adj[nb_sommets+1][0] = g.adj[s][j];
                types[nb_sommets+1][0] = -1;

                //On marque l'arête comme traitée
                g.adj[g.adj[s][j]][j_voisin] = -1;
                g.adj[s][j] = -1;
                nb_sommets++;
                nb_sommets++;
            }
            
        }

        //On relie les arêtes entre elles

        for(int j=0; j<4; j++){
            if(adj[s][(j+3)%4] != adj[adj[s][j]][2]) {
                adj[adj[s][j]][1] = adj[s][(j+3)%4];
				types[adj[s][j]][1] = -1;
            } else{
                adj[adj[s][j]][1] = -1;
				types[adj[s][j]][1] = 0;
            }
            if(adj[s][(j+1)%4] != adj[adj[s][j]][2]) {
                adj[adj[s][j]][3] = adj[s][(j+1)%4];
				types[adj[s][j]][3] = -1;
            } else {
                adj[adj[s][j]][3] = -1;
				types[adj[s][j]][3] = 0;
            }
        }
        // printf("Fini sommet %d\n", s);
        // fflush(stdout);
    }

    
    //ATTENTION: la taille du tableau est 5*g.n, pas nb_sommets
    //Contient des -1
    graphe res = {
        .n = nb_sommets,
        .adj = adj,
        .type = types,
        .DFI = DFI,
        .type_sommet = type_sommets,
        .indices_DFI = indices_DFI
    };
    return res;
}

//Initialise un graphe de bon cardinal
graphe_comb init_graphe_comb(graphe g){
    sommet* sommets = malloc(g.n*sizeof(sommet));
    sommet_racine* R = malloc(g.n*sizeof(sommet_racine));
    demi_arete* A = malloc(8*g.n*sizeof(demi_arete));
    pile* P = init_pile();

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
        R[i].parent = -1;
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


void DFS(graphe* g, bool* vus, int s, int* index, double_liste** arbre_DFS){
    /*Le DFS:   donne un index DFI a chaque sommet
                trie les aretes en aretes retour/aretes de parcours
    */
//    printf("traite sommet %d\n", s);
//    fflush(stdout);
    vus[s] = true;
    g->indices_DFI[s] = *index;
    g->DFI[(*index)++] = s;
    for(int i = 0; i<4; i++){
        if(g->adj[s][i] > -1 && !vus[g->adj[s][i]]){ //On continue
            arbre_DFS[s] = postinsertion(arbre_DFS[s], g->adj[s][i]);
            g->type[s][i] = 1;
            DFS(g,vus,g->adj[s][i], index, arbre_DFS);
        } else if (g->adj[s][i] > -1){ 
            //on identifie l'arête correspondante et on match son type
            for(int j = 0; j<4; j++){
                if(g->adj[g->adj[s][i]][j] == s) g->type[s][i] = g->type[g->adj[s][i]][j] ;
            } 
        }
    }
}

double_liste** precalcul_graphe(graphe* g){
    /* Prend un graphe g simplifié ayant été extrait d'une sequence DT, et utilise un DFS pour:
    - donner un ordre DFS aux sommets
    - trier les aretes de parcours ou de retour
    - renvoyer l'arbre DFS*/
    bool* vus = malloc(g->n*sizeof(bool));
    double_liste** arbre_DFS = malloc(g->n*sizeof(double_liste*));
    for(int i = 0; i<g->n; i++){
        vus[i] = false;
        arbre_DFS[i] = NULL;
    }
    int index = 0;
    DFS(g, vus, 0, &index, arbre_DFS);
    free(vus);
    return arbre_DFS;
}

void tri_enfants_DFS_broken(graphe_comb* gtilde, int s){
    /*Tri de la liste (à au plus 4 éléments) des enfants DFS d'un sommet*/
    // printf("tri enfants de %d\n", s);
    // fflush(stdout);
    if(gtilde->S[s].enfantsDFS != NULL){
        double_liste* depart = gtilde->S[s].enfantsDFS;
        double_liste* pos = gtilde->S[s].enfantsDFS->suiv;
        while(pos != gtilde->S[s].enfantsDFS){
            if(gtilde->S[pos->val].point_min < gtilde->S[depart->val].point_min){
                //echange de place
                int tmp = depart->val;
                depart->val = pos->val;
                pos->val = tmp;
                gtilde->S[depart->val].p_parentDFS = pos;
                gtilde->S[pos->val].p_parentDFS = depart;
				
            }
            pos = pos->suiv;
        }
        depart = depart->suiv;
        while(depart != gtilde->S[s].enfantsDFS){
            pos = depart->suiv;
            while(pos != gtilde->S[s].enfantsDFS){
                if(gtilde->S[pos->val].point_min < gtilde->S[depart->val].point_min){
                    //echange de place
                    int tmp = depart->val;
                    depart->val = pos->val;
                    pos->val = tmp;
                    gtilde->S[depart->val].p_parentDFS = pos;
                    gtilde->S[pos->val].p_parentDFS = depart;
                }
                pos = pos->suiv;
            }
            depart = depart->suiv;
        }
    }
}

void tri_enfants_DFS(graphe_comb* gtilde, int s){
	/*Tri de la liste (à au plus 4 éléments) des enfants DFS d'un sommet par insertion*/
	if(gtilde->S[s].enfantsDFS != NULL){
		double_liste* tete = NULL;
		while(gtilde->S[s].enfantsDFS){
			//On prend le plus petit point_min parmi la liste
			int m = -1;
			double_liste* plus_petit;
			double_liste* pos = gtilde->S[s].enfantsDFS;
			while(m == -1 || pos != gtilde->S[s].enfantsDFS){
				if(m == -1 || gtilde->S[pos->val].point_min < m){
					m = gtilde->S[pos->val].point_min;
					plus_petit = pos;
				}
				pos = pos->suiv;
			}
			//On le copie dans la nouvelle liste
			tete = postinsertion(tete, plus_petit->val);
			gtilde->S[plus_petit->val].p_parentDFS = tete->prec;
			//On le supprime de la liste
			gtilde->S[s].enfantsDFS = suppression(plus_petit);
		}
		//On remplace la liste
		gtilde->S[s].enfantsDFS = tete;
	}
}


//Marche
void precalcul(graphe g, graphe_comb *gtilde){
    /*Modifie gtilde précédemment initialisé: (g est le graphe simplifié déjà parcouru)
        - calcule le parent DFS de chaque sommet
        - calcule les petit_ancetre des sommets
        - calcule les points_min des sommets
        - donne les enfants DFS des sommets*/
    print_tab(g.DFI, g.n);
	printf("g.n: %d\n", g.n);
    //Reporte l'indice DFI et le type
    for(int s = 0; s<g.n; s++){ 
        gtilde->S[s].DFI = g.indices_DFI[s];
        gtilde->S[s].petit_ancetre = g.indices_DFI[s];
        gtilde->S[s].type = g.type_sommet[s];
    }
    // printf("Fini DFI\n");
    // fflush(stdout);

    //Calcul de l'ancêtre direct de plus petit indice et DFI
    for(int s = 0; s<g.n; s++){
        for(int v = 0; v<4; v++){
			assert(g.type[s][v] == 0 || g.adj[s][v] > -1);
			assert(g.adj[s][v] < g.n);
            if( g.type[s][v] < 0 && gtilde->S[g.adj[s][v]].DFI < gtilde->S[s].petit_ancetre){ //Arete retour et DFI plus petit
                gtilde->S[s].petit_ancetre = gtilde->S[g.adj[s][v]].DFI;
            }
        }
    }
    // printf("Fini ancetre\n");
    // fflush(stdout);
    //Calcul du parent DFS et insertion dans enfantDFS
    for(int s=0; s<g.n; s++){
        for(int v = 0; v<4; v++){
            //printf("s: %d, p: %d, dfi s: %d, dfi p: %d, type: %d\n", s,g.adj[s][v],gtilde->S[s].DFI,gtilde->S[g.adj[s][v]].DFI, g.type[s][v]);
            if( g.type[s][v] > 0 && gtilde->S[s].DFI > gtilde->S[g.adj[s][v]].DFI){
                //printf("trouve un parent\n");
                int parent = g.adj[s][v];
                gtilde->S[s].parentDFS = parent;
				check_list(gtilde->S[parent].enfantsDFS);
                gtilde->S[parent].enfantsDFS = preinsertion(gtilde->S[parent].enfantsDFS, s);
				check_list(gtilde->S[parent].enfantsDFS);
                gtilde->S[s].p_parentDFS = gtilde->S[parent].enfantsDFS;
            }
        }
    }
    // printf("Fini parent\n");
    // fflush(stdout);
    //Calcul du point_min
    for(int i = g.n-1; i>=0; i--){
        int s = g.DFI[i];
        gtilde->S[s].point_min = gtilde->S[s].petit_ancetre;
        for(int v = 0; v<4; v++){
            if( g.type[s][v] > 0 && gtilde->S[g.adj[s][v]].DFI > gtilde->S[s].DFI //arête de parcours descendante
                && gtilde->S[g.adj[s][v]].point_min < gtilde->S[s].point_min){
                gtilde->S[s].point_min = gtilde->S[g.adj[s][v]].point_min;
            }
        }
    }
    // printf("Fini point min\n");
    // fflush(stdout);

    //Tri des enfantsDFS
    //on trie les sommets par ordre croissant de point min
    for(int s=0; s<g.n; s++){
        tri_enfants_DFS(gtilde, s);
    }
}

void montee(graphe_comb* gtilde, int w, int v){
    /* Identifie le sous-graphe pertinent pour l'arête retour (v,w)
        On parcourt le chemin dans l'arbre de v à w en identifiant les sommets coupants, ie les sommets racines
        Le parcours se fait de demi arête en demi arête*/

    printf("-----Montee depuis %d:\n\n", w);
    gtilde->S[w].flag_arete_retour = v; //On marque w comme pertinent
    demi_arete x = gtilde->A[gtilde->S[w].adj[0].index] , y = gtilde->A[gtilde->S[w].adj[1].index];
    int entree_x = 0, entree_y = 1; //L'un part sens horaire, l'autre sens trigo

    //Tant qu'on n'est pas arrivé à v, et qu'on n'est pas déjà passé par là
    while(x.voisin.index != v && y.voisin.index != v && (gtilde->S[x.voisin.index].visite != v || x.voisin.lieu != dansS)
            && (gtilde->S[y.voisin.index].visite != v || y.voisin.lieu != dansS)){ 

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
            int c = z.voisin.index; //parce que c = vc
            int p = gtilde->S[c].parentDFS;
            // printf("c: %d, p: %d\n\n", c, p);
            if(p != v){ //On continue la remontée
				if(gtilde->R[c].pertinence == NULL){
					if(gtilde->S[c].point_min >= gtilde->S[v].DFI){ //Je viens de changer ça
						gtilde->S[p].racines_pertinentes = preinsertion(gtilde->S[p].racines_pertinentes, c);
						gtilde->R[c].pertinence = gtilde->S[p].racines_pertinentes;
					}
					else {
						gtilde->S[p].racines_pertinentes = postinsertion(gtilde->S[p].racines_pertinentes, c);
						gtilde->R[c].pertinence = gtilde->S[p].racines_pertinentes->prec;
					}
				}
                x = gtilde->A[gtilde->S[p].adj[0].index];
                y = gtilde->A[gtilde->S[p].adj[1].index];
                entree_x = 0;
                entree_y = 1;
            } else { //Si on est arrivé à v, on veut s'areter à la prochaine itération
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
    printf("Fusion de compo biconnexes: \nAvant:\n");
    print_graphe_comb_final(*gtilde);
    print_pile_propre(gtilde->P);
    fflush(stdout);
    int sortie_rc = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int rc = top(gtilde->P); //racine de la CBC enfant de r
    gtilde->P = pop(gtilde->P);
    int entree_r = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int r = top(gtilde->P); //parent de rc
    gtilde->P = pop(gtilde->P);

    if(entree_r == sortie_rc){ //on a la garantie que les sommets traversés ne sont pas ext actifs
        printf("\n---On a inverse le sens\n");
        fflush(stdout);
        //On inverse le sens de la liste des demi-arêtes de rc
        int temp = gtilde->R[rc].lien[0].index;
        gtilde->R[rc].lien[0].index = gtilde->R[rc].lien[1].index;
        gtilde->R[rc].lien[1].index = temp;
        int pos = temp;
        int debut = pos;
        bool premiere_iteration = true;
        while(premiere_iteration || pos != debut){
            premiere_iteration = false;
            // printf("pos: %d\n", pos);
            temp = gtilde->A[pos].adj[0].index;
            gtilde->A[pos].adj[0].index = gtilde->A[pos].adj[1].index;
            gtilde->A[pos].adj[1].index = temp;
            // printf("Inverse %d et %d", gtilde->A[pos].adj[0].index, gtilde->A[pos].adj[1].index);
            pos = temp;
            //printf("pos: %d\n", pos);
        } //pos est arrivée à la fin
        //printf("sorti du premier while\n"); fflush(stdout);

        //On change le signe de c: la composante en-dessous est à retourner
        gtilde->S[rc].signe = -1;
        
        sortie_rc = 1 - sortie_rc;
        //print_graphe_comb_final(*gtilde);
    }
    
    //On fusionne les composantes biconnexes, et on supprime le sommet racine
    //On redirige les arêtes
    int pos = gtilde->R[rc].lien[0].index;
    while(gtilde->A[pos].voisin.index != r){ //On dit que les arêtes pointent maintenant vers r
        printf("Redirige l'arete %d vers %d\n", pos, r);fflush(stdout);
        gtilde->A[pos].voisin.index = r;
        gtilde->A[pos].voisin.lieu = dansS;

        pos = gtilde->A[pos].adj[1].index;
    }
    //printf("redirection ok\n");fflush(stdout);
	//On retire rc des racines pertinentes de r
	gtilde->S[r].racines_pertinentes = suppression(gtilde->R[rc].pertinence);
    gtilde->R[rc].pertinence = NULL;
    //On retire c de la liste des enfants DFS de r
	printf("parent: %d, enfant a retirer: %d\n", r, rc);
	check_list(gtilde->S[rc].p_parentDFS);
	printf("Avant suppression:\n");
	print_list(gtilde->S[r].enfantsDFS);
	printf("Cellule pointee: %d\n", gtilde->S[rc].p_parentDFS->val);
    gtilde->S[r].enfantsDFS = suppression(gtilde->S[rc].p_parentDFS);
	printf("Après suppression: \n");
	print_list(gtilde->S[r].enfantsDFS);
	gtilde->S[rc].p_parentDFS = NULL;
	check_list(gtilde->S[r].enfantsDFS);

    //Enfin, on fusionne les listes d'adjacence dans le bon ordre 
    lien_adjacence rin = gtilde->S[r].adj[entree_r];
    lien_adjacence rcout = gtilde->R[rc].lien[sortie_rc];
    //printf("Les aretes sont: rin %d, suiv rin %d, rcout %d, prec rcout %d\n", rin.index, suiv_rin.index, rcout.index, prec_rcout.index);
    printf("Aretes de v%d: ", rc);
    print_aretes(*gtilde, dansR, rc);
    printf("Aretes de %d: ", r);
    print_aretes(*gtilde, dansS, r);
    fflush(stdout);
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
    //Petit luxe pour l'affichage: on supprime le sommet virtuel
    gtilde->R[rc].parent = -1;

    printf("Reussi fusion\n");
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
    //printf("On a %d aretes\n", gtilde->m);
	FILE *f = fopen("aretes.txt", "a");
	fprintf(f, "arete vc: %d, w: %d\n", c, w);
	fclose(f);
	
	
    //On ajoute la demi arete incidente à vc
    gtilde->A[gtilde->m].adj[dir].lieu = dansA;
    gtilde->A[gtilde->m].adj[1-dir].lieu = dansA;
    gtilde->A[gtilde->m].jumelle = gtilde->m + 1;
    gtilde->A[gtilde->m].reelle = true;
    //gtilde->A[gtilde->m].type = 1;
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
    gtilde->A[gtilde->m].reelle = true;
    //gtilde->A[gtilde->m].type = 1;
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
    gtilde->A[gtilde->m].reelle = false;
    //gtilde->A[gtilde->m].type = 0;
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
    gtilde->A[gtilde->m].reelle = false;
    //gtilde->A[gtilde->m].type = 0;
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

void successeur_face_ext_actif(graphe_comb gtilde, demi_arete* w, int* entree_w, int v){
    /*w est la demi-arête d'où l'on est entré sur x, du côté entree_w
    v est le sommet duquel on fait la descente*/
    /*On cherche le prochain sommet pertinent ou stoppant sur la face extérieure*/
    successeur_face_ext(gtilde, w, entree_w); //on quitte le sommet-racine
    int x = w->voisin.index; //forcément réel
    while (inactif(gtilde, v, x)){
        assert(w->voisin.lieu == dansS);
        successeur_face_ext(gtilde, w, entree_w);
    }
}

void descente(graphe_comb* gtilde, int c){
    printf("-----Descente depuis v%d:\n\n", c);
    //c est l'enfant d'où part la descente

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
            char l;
            if(w.voisin.lieu == dansR){l = 'R';} else l = 'S';
            printf("Entre dans le while: %d%c par %d\n", w.voisin.index, l, entree_w); 
            
            //Invariant de boucle: w est une demi-arete liée à un sommet réel (pas racine)
            //w est la demi arête par laquelle on est entré dans le sommet
            
            if(w.voisin.lieu != dansS) {printf("sommet virtuel dans le while\n");fflush(stdout); return;}
            // printf("Inactif: %d\n", inactif(*gtilde, v, w.voisin.index));
            // printf("Pertinent: %d\n", pertinent(*gtilde, v, w.voisin.index));
            // printf("Actif Externe: %d\n", actif_externe(*gtilde, v, w.voisin.index));
            // print_graphe_comb(*gtilde);
            sommet sw = gtilde->S[w.voisin.index];

            if(sw.flag_arete_retour == v){ //On a trouvé une arete retour à ajouter
                printf("On doit ajouter arete retour v%d, %d\n", c, w.voisin.index);
                while(gtilde->P != NULL){
                    fusion_compo_biconnexes(gtilde);
                    //print_graphe_comb_final(*gtilde);
                }
                printf("Ajout arete retour v%d, %d\n", c, w.voisin.index);fflush(stdout);
                ajout_arete_retour(gtilde, c, dir, w.voisin.index, entree_w);
                gtilde->S[w.voisin.index].flag_arete_retour = gtilde->n;
                print_graphe_comb_final(*gtilde);
            }

            if(sw.racines_pertinentes != NULL){
                //On doit push w et entree w
                printf("Racines pertinentes trouvees: %d -> %d\n", w.voisin.index, sw.racines_pertinentes->val);fflush(stdout);
                gtilde->P = push(gtilde->P, w.voisin.index);
                gtilde->P = push(gtilde->P, entree_w);
                //On continue de descendre: wc est le suivant
                int swc = sw.racines_pertinentes->val; //Premier élément des racines pertinentes (sommet)
				demi_arete x = gtilde->A[gtilde->R[swc].lien[1].index]; int entree_x = 1; 
                demi_arete y = gtilde->A[gtilde->R[swc].lien[0].index]; int entree_y = 0;
                printf("swc: %d\n",swc);fflush(stdout);
                successeur_face_ext_actif(*gtilde, &x, &entree_x, v); 
                successeur_face_ext_actif(*gtilde, &y, &entree_y, v);

                printf("Successeurs face ext: x: %d, y: %d\n", x.voisin.index, y.voisin.index);

                if(actif_interne(*gtilde, v, x.voisin.index)){w = x; entree_w = entree_x; printf("Interne x\n");fflush(stdout);}
                else if(actif_interne(*gtilde, v, y.voisin.index)){w = y; entree_w = entree_y;printf("Interne y\n");fflush(stdout);}
                else if(pertinent(*gtilde, v, x.voisin.index)){w = x; entree_w = entree_x;printf("Pertinent x\n");fflush(stdout);}
                else {w = y; entree_w = entree_y;printf("else y\n");}
                fflush(stdout);
                int sortie_w; 
                if(w.adj[1].index == x.adj[1].index) sortie_w = 0;
                else sortie_w = 1;
                //On push wc et sortie_w
                gtilde->P = push(gtilde->P, swc);
                gtilde->P = push(gtilde->P, sortie_w);
                printf("Suivant: %d\n", w.voisin.index);
                fflush(stdout);

            } else if(inactif(*gtilde, v, w.voisin.index)){
                printf("inactif\n");fflush(stdout);
                successeur_face_ext(*gtilde, &w, &entree_w);
            } else { //w est un sommet stoppant
                if(gtilde->P == NULL && gtilde->S[c].point_min < v){
                    if(gtilde->A[gtilde->S[c].adj[0].index].voisin.index == w.voisin.index){
                        printf("Pas de double raccourci!\n");fflush(stdout);
                    } else{
                        printf("Ajout arete raccourci %d %d, %d %d\n\n", c, dir, w.voisin.index, entree_w);
                        printf("%d vs %d\n", gtilde->A[gtilde->S[c].adj[dir].index].voisin.index, w.voisin.index);fflush(stdout);
                        ajout_arete_raccourci(gtilde, c, dir, w.voisin.index, entree_w);
                        //print_graphe_comb_final(*gtilde);
                        fflush(stdout);
                    }
                    
                }
                printf("Break: %d\n", w.voisin.index);fflush(stdout);
                break;
            }
        }
        char l;
        if(w.voisin.lieu == dansR) l = 'R'; else l = 'P';
        printf("Fin de while: %c, %d\n", l, w.voisin.index);fflush(stdout);
        if(gtilde->P != NULL) break;
    }
}

void DFS_final(graphe_comb gtilde, graphe* res, int orientation, int s, double_liste** arbre_DFS){
    /*  arbre_DFS: tableau contenant des listes chainées des enfants DFS du graphe de base
        associations: tableau contenant les associations de sommets-arêtes correspondant à la même arête*/
    //N'emprunte que des arêtes du premier DFS
    int signe_sommet = gtilde.S[s].signe*orientation;
    if(gtilde.S[s].type == 0){
        //Recopie des arêtes réelles
        if(signe_sommet > 0){
            res->adj[s] = malloc(4*sizeof(int));
            int i = 0;
            int w = gtilde.S[s].adj[0].index;
            if (w == -1) {printf("Arite 0??\n"); return;}
            if(gtilde.A[w].reelle){
                res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                i++;
            }
            w = gtilde.A[w].adj[1].index;
            while(w != gtilde.S[s].adj[0].index){
                if(gtilde.A[w].reelle){
                    if(i>3){printf("trop d'aretes\n"); return;}
                    res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                    i++;
                }
                w = gtilde.A[w].adj[1].index;
            }
        } else { //retourne la liste
            res->adj[s] = malloc(4*sizeof(int));
            int i = 0;
            int w = gtilde.S[s].adj[1].index;
            if (w == -1) {printf("Arite 0??\n"); return;}
            if(gtilde.A[w].reelle){
                res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                i++;
            }
            w = gtilde.A[w].adj[0].index;
            while(w != gtilde.S[s].adj[1].index){
                if(gtilde.A[w].reelle){
                    if(i>3){printf("trop d'aretes\n"); return;}
                    res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                    i++;
                }
                w = gtilde.A[w].adj[0].index;
            }
        }
    }
    //On passe au suivant
    while(arbre_DFS[s] != NULL){
        int suivant = arbre_DFS[s]->val;
        arbre_DFS[s] = suppression(arbre_DFS[s]);
        DFS_final(gtilde, res, signe_sommet, suivant, arbre_DFS);
    }
}

graphe extraction_BM(graphe_comb gtilde, graphe g_simple, double_liste** arbre_DFS, int* associations){
	/*  arbre_DFS: tableau contenant des listes chainées des enfants DFS du graphe de base
        associations: tableau contenant les associations de sommets-arêtes correspondant à la même arête
    A partir du graphe combinatoire renvoyé par BM, donne un graphe où
	 - Seuls les sommets des croisements restent
     - Les arêtes sont dans l'ordre autour des sommets*/
    graphe res;
    res.n = gtilde.n/5;
    res.adj = malloc(res.n*sizeof(int*));
    int orientation = 1;
    DFS_final(gtilde, &res, &orientation, 0, arbre_DFS);

    //On cherche à raccorder les demi-arêtes
    for(int s=0; s<res.n; s++){
        for(int j = 0; j<4; j++){
            int k = res.adj[s][j];
            if(g_simple.type[k][2] != 0){
                res.adj[s][j] = g_simple.adj[associations[k]][0];
            } else { //On est une arête double, on prend le même voisin que notre doublon
                res.adj[s][j] = g_simple.adj[associations[associations[k]]][0];
            }
        }
    }
    return res;
}

graphe_comb BoyerMyrvold(seq_dt seq){
    /*Prend une séquence DT et renvoie le graphe combinatoire après exécution de l'algo de Boyer Myrvold*/
        //Precalcul
    //Transformation en un graphe
    graphe g = conversion_seqDT(seq);
    printf("Fini conversion\n");
    fflush(stdout);
    print_multigraphe(g);
    fflush(stdout);
    //Simplification du graphe
    int* associations = malloc(5*g.n*sizeof(int));
    graphe g_simple = simplifier_graphe(g, associations);
    printf("Fini simplification\n");
    //print_graphe_simple(g_simple);
    fflush(stdout);
    double_liste** arbre_DFS = precalcul_graphe(&g_simple);
    printf("Fini dfs\n");
    fflush(stdout);
    //Initialisation du graphe combinatoire
    graphe_comb gtilde = init_graphe_comb(g_simple);
    printf("Fini initialisation\n");
    fflush(stdout);
    precalcul(g_simple, &gtilde);
    printf("Fini precalcul:\n");
    fflush(stdout);
    print_graphe_simple(g_simple);

        //Boucle principale
    for(int i = g_simple.n-1; i>=0; i--){ //On traite les sommets par ordre DFI descendant
        int v = g_simple.DFI[i];
        printf("------------- Boucle pour v: %d\n", v);
        //Pour chaque enfant de v
        print_list(gtilde.S[v].enfantsDFS);
        double_liste* debut = gtilde.S[v].enfantsDFS;
        double_liste* suivant = debut;
        bool premiere_iteration = true;
        //Creation des enfants virtuels
        while(suivant != NULL && (suivant != debut || premiere_iteration)){
            premiere_iteration = false;
            int c = suivant->val;
            //On crée le sommet-racine vc, relié à c
            //C'est la seule arete de c et de vc, car si c a des enfants, il ne peut pas encore avoir été fusionné dans la même CBC
            //On crée un sommet virtuel
            gtilde.R[c].parent = v;
            gtilde.R[c].lien[0].lieu = dansA;
            gtilde.R[c].lien[0].index = gtilde.m;
            gtilde.R[c].lien[1].lieu = dansA;
            gtilde.R[c].lien[1].index = gtilde.m;
			gtilde.R[c].pertinence = NULL;
            //On ajoute l'arête de parcours (vc, c) comme 2 demi-arêtes
            gtilde.A[gtilde.m].adj[0].lieu = dansA;
            gtilde.A[gtilde.m].adj[0].index = gtilde.m; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m].adj[1].index = gtilde.m; 
            gtilde.A[gtilde.m].jumelle = gtilde.m+1;
            gtilde.A[gtilde.m].voisin.index = c;
            gtilde.A[gtilde.m].voisin.lieu = dansR; //relie à vc
            //gtilde.A[gtilde.m].type = 1; //initialisé à 1
            gtilde.A[gtilde.m].reelle = true;
            gtilde.A[gtilde.m+1].adj[0].lieu = dansA;
            gtilde.A[gtilde.m+1].adj[0].index = gtilde.m+1; //indique la demi-arete précédente, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m+1].adj[1].lieu = dansA; //indique la demi-arete suivante, il n'y en a pas: c'est elle-même
            gtilde.A[gtilde.m+1].adj[1].index = gtilde.m+1; 
            gtilde.A[gtilde.m+1].jumelle = gtilde.m;
            gtilde.A[gtilde.m+1].voisin.index = c;
            gtilde.A[gtilde.m+1].voisin.lieu = dansS; //relie à c
            //gtilde.A[gtilde.m+1].type = 1; //initialisé à 1
            gtilde.A[gtilde.m+1].reelle = true;

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
        for(int j = 0; j<4; j++){
            if(g_simple.type[v][j] < 0 && gtilde.S[g_simple.adj[v][j]].DFI > gtilde.S[v].DFI){ //Si arete retour et descendant
                int w = g_simple.adj[v][j];
				printf("Montee depuis %d\n", w);
                montee(&gtilde, w, v); 
            }
        }
        printf("----------------- Fin des montees: \n");
        print_graphe_comb_final(gtilde);

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
        // printf("Fin des descentes: \n");
        // print_graphe_comb_final(gtilde);
        //On teste si la descente a bien réussi?
    }
    
    //On rajoute le sommet 0
    double_liste* c = gtilde.S[0].enfantsDFS;
    //On redirige les arêtes
    int pos = gtilde.R[c->val].lien[0].index;
    while(gtilde.A[pos].voisin.index != 0){ //On dit que les arêtes pointent maintenant vers 0
        //printf("Redirige l'arete %d vers %d\n", pos, r);
        gtilde.A[pos].voisin.index = 0;
        gtilde.A[pos].voisin.lieu = dansS;
        pos = gtilde.A[pos].adj[1].index;
    }
    gtilde.S[0].adj[0].index = gtilde.R[c->val].lien[0].index;
    gtilde.S[0].adj[1].index = gtilde.R[c->val].lien[1].index;
    c = c->suiv;
    while(c != gtilde.S[0].enfantsDFS){
        //On redirige les aretes
        int pos = gtilde.R[c->val].lien[0].index;
        while(gtilde.A[pos].voisin.index != 0){ //On dit que les arêtes pointent maintenant vers 0
            //printf("Redirige l'arete %d vers %d\n", pos, r);
            gtilde.A[pos].voisin.index = 0;
            gtilde.A[pos].voisin.lieu = dansS;
            pos = gtilde.A[pos].adj[1].index;
        }
        //on fusionne les listes
        gtilde.A[gtilde.S[0].adj[0].index].adj[0].index = gtilde.R[c->val].lien[1].index;
        gtilde.A[gtilde.S[0].adj[1].index].adj[1].index = gtilde.R[c->val].lien[0].index;
        gtilde.A[gtilde.R[c->val].lien[1].index].adj[1].index = gtilde.S[0].adj[0].index;
        gtilde.A[gtilde.R[c->val].lien[0].index].adj[0].index = gtilde.S[0].adj[1].index;
        gtilde.S[0].adj[1].index = gtilde.R[c->val].lien[1].index;
    }
    printf("Fini le sommet 0\n");
    fflush(stdout);

    printf("Resultat final:\n");
    print_graphe_comb_final(gtilde);

    //A FAIRE Tout libérer
    printf("Extraction:\n");
    graphe res = extraction_BM(gtilde, g_simple, arbre_DFS, associations);
    print_graphe_final(res);
    return gtilde;
}

//Fonctions de test
/*
void test_conversion(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    printf("Sequence:\n");
    print_seq_dt(&noeud_wiki);
    fflush(stdout);
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    printf("Multigraphe:\n");
    print_multigraphe(gnoeud_wiki);
    fflush(stdout);
    graphe noeud_wiki_simple = simplifier_graphe(gnoeud_wiki);
    precalcul_graphe(&noeud_wiki_simple);
    printf("Graphe simple:\n");
    print_graphe_simple(noeud_wiki_simple);
    fflush(stdout);
}

void test_precalcul(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    graphe noeud_wiki_simple = simplifier_graphe(gnoeud_wiki);
    precalcul_graphe(&noeud_wiki_simple);
    graphe_comb gtilde = init_graphe_comb(noeud_wiki_simple);
    precalcul(noeud_wiki_simple, &gtilde);
    printf("Precalcul:\n");
    print_graphe_comb_initial(gtilde);
}

void test_algo_wiki(){
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe gnoeud_wiki = conversion_seqDT(noeud_wiki);
    graphe noeud_wiki_simple = simplifier_graphe(gnoeud_wiki);
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

*/

/*Pour se rappeler du signe de l'arête: on regarde que le signe de 0 à 1 ehe*/
/*A FAIRE (par ordre de prio)
La conversion finale
Changer le truc des liens/lieu d'adjacence c'est ridicule un peu*/
int main(){
    //test_conversion();
    //test_precalcul();
    //test_algo_wiki();
	FILE* f = fopen("aretes.txt", "w");
	fprintf(f, "Aretes:\n");
	fclose(f);
    int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    graphe_comb g = BoyerMyrvold(noeud_wiki);

    // int seq[6] = {3, -6, 1, 4, -2, -5};
    // seq_dt noeud = {.taille = 6, .seq = seq};
    // graphe g = BoyerMyrvold(noeud);
    // printf("Resultat final:\n");
    // fflush(stdout);
    // print_res_BM(g);


    printf("\nok!\n");
    fflush(stdout);
    return 0;
}