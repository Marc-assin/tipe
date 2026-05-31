#include "knot_gen.c"
#include <stdio.h>
#include <assert.h>
#include <string.h>
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
    }

    *w = gtilde.A[e.jumelle];//on traverse
    x = w->voisin.index;
    if(w->voisin.lieu == dansS && gtilde.S[x].adj[0].index != gtilde.S[x].adj[1].index){ //Si le sommet n'est pas d'arité 1
            if(gtilde.A[gtilde.S[x].adj[0].index].jumelle == w->jumelle) *entree_w = 0; //Si la demi arete 0 est celle d'où on vient
            else *entree_w = 1;
    } else if(w->voisin.lieu == dansR && gtilde.R[x].lien[0].index != gtilde.R[x].lien[1].index){
            if(gtilde.A[gtilde.R[x].lien[0].index].jumelle == w->jumelle) *entree_w = 0; //Si la demi arete 0 est celle d'où on vient
            else *entree_w = 1;
    }//Si le sommet est d'arité 1, on continue du mm côté
}

//Precalcul

//Convertit une sequence DT en un multigraphe
graphe conversion_seqDT(seq_dt s){
    int n = s.taille;
    int** adj = malloc(n*sizeof(int*)); //sommet i correspond au numéro de séquence 2*i+1
    //Un tableau avec la paire de tous les numéros de la séquence
    int* paires = malloc((2*n+1)*sizeof(int)); //les numéros allant de 1 à 2n+1, la case 0 est inallouée et inutile
    paires[0] = -1;
    for(int i=0; i<n; i++){
        paires[2*i+1] = abs(2*s.seq[i]);
        paires[abs(2*s.seq[i])] = 2*i+1;
    };
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
    return res;
};

graphe simplifier_graphe(graphe g, int* associations){
    /*Prend un graphe de noeud
    Ajoute deux sommets pour chaque arête
    Met des arêtes entre: 
     - une arête et le sommet qu'elle relie
     - deux sommets correspondant à la même arête
     - des aretes consécutives
     - retient qui est une arête double
     - retient le signe d'une arête
    Résultat peut contenir des -1 !
    Fait des associations entre des sommets-arêtes d'une même arête*/

    int** adj = malloc(5*(g.n)*sizeof(int*));
    int* DFI = malloc(5*(g.n)*sizeof(int));
    int** types = malloc(5*(g.n)*sizeof(int*));
    int* indices_DFI = malloc(5*(g.n)*sizeof(int));
    int nb_sommets = g.n;

    //On initialise les sommets
    for(int s = 0; s<g.n; s++){
        adj[s] = malloc(4*sizeof(int));
        types[s] = malloc(4*sizeof(int));
        for(int a = 0; a<4; a++){
            types[s][a] = -1;
        }
        associations[s] = -1;
    }


    for(int s = 0; s<g.n; s++){
        for(int j = 0; j<4; j++){
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
                    assert(false);
                }
                //On teste si l'arête n'est pas en réalité double
                if(j>0 && g.adj[s][j] == g.adj[s][j-1]){
                    associations[nb_sommets] = -adj[s][j-1];
                    associations[nb_sommets+1] = -adj[adj[s][j-1]][2];
                    adj[nb_sommets][2] = -1;
                    types[nb_sommets][2] = 0;
                    adj[nb_sommets+1][2] = -1;
                    types[nb_sommets+1][2] = 0;
                } else if(j>0 && g.adj[s][j] == g.adj[s][0]) {
                    associations[nb_sommets] = -adj[s][0];
                    associations[nb_sommets+1] = -adj[adj[s][0]][2];
                    adj[nb_sommets][2] = -1;
                    types[nb_sommets][2] = 0;
                    adj[nb_sommets+1][2] = -1;
                    types[nb_sommets+1][2] = 0;
                } else{ //Il y a réellement une arête à créer
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

    }

    
    //ATTENTION: la taille du tableau est 5*g.n, pas nb_sommets
    //Contient des -1
    graphe res = {
        .n = nb_sommets,
        .adj = adj,
        .type = types,
        .DFI = DFI,
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
		sommets[i].signe = 1;
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

void precalcul(graphe g, graphe_comb *gtilde){
    /*Modifie gtilde précédemment initialisé: (g est le graphe simplifié déjà parcouru)
        - calcule le parent DFS de chaque sommet
        - calcule les petit_ancetre des sommets
        - calcule les points_min des sommets
        - donne les enfants DFS des sommets*/
    //Reporte l'indice DFI et le type
    for(int s = 0; s<g.n; s++){ 
        gtilde->S[s].DFI = g.indices_DFI[s];
        gtilde->S[s].petit_ancetre = g.indices_DFI[s];
    }


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
    //Calcul du parent DFS et insertion dans enfantDFS
    for(int s=0; s<g.n; s++){
        for(int v = 0; v<4; v++){
            if( g.type[s][v] > 0 && gtilde->S[s].DFI > gtilde->S[g.adj[s][v]].DFI){
                int parent = g.adj[s][v];
                gtilde->S[s].parentDFS = parent;
				check_list(gtilde->S[parent].enfantsDFS);
                gtilde->S[parent].enfantsDFS = preinsertion(gtilde->S[parent].enfantsDFS, s);
				check_list(gtilde->S[parent].enfantsDFS);
                gtilde->S[s].p_parentDFS = gtilde->S[parent].enfantsDFS;
            }
        }
    }
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
    gtilde->S[w].flag_arete_retour = v; //On marque w comme pertinent
    demi_arete x = gtilde->A[gtilde->S[w].adj[0].index] , y = gtilde->A[gtilde->S[w].adj[1].index];
    int entree_x = 0, entree_y = 1; //L'un part sens horaire, l'autre sens trigo

    //Tant qu'on n'est pas arrivé à v, et qu'on n'est pas déjà passé par là
    while(x.voisin.index != v && y.voisin.index != v && (gtilde->S[x.voisin.index].visite != v || x.voisin.lieu != dansS)
            && (gtilde->S[y.voisin.index].visite != v || y.voisin.lieu != dansS)){ 


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
            int c = z.voisin.index; //parce que c = vc
            int p = gtilde->S[c].parentDFS;
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
}

void fusion_compo_biconnexes(graphe_comb* gtilde){
    /*La pile contient (du haut vers le bas):
        sortie_wc, wc, entree_w, w*/

    int sortie_rc = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int rc = top(gtilde->P); //racine de la CBC enfant de r
    gtilde->P = pop(gtilde->P);
    int entree_r = top(gtilde->P);
    gtilde->P = pop(gtilde->P);
    int r = top(gtilde->P); //parent de rc
    gtilde->P = pop(gtilde->P);

    if(entree_r == sortie_rc){ //on a la garantie que les sommets traversés ne sont pas ext actifs
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
            temp = gtilde->A[pos].adj[0].index;
            gtilde->A[pos].adj[0].index = gtilde->A[pos].adj[1].index;
            gtilde->A[pos].adj[1].index = temp;
            pos = temp;
        } //pos est arrivée à la fin

        //On change le signe de c: la composante en-dessous est à retourner
        gtilde->S[rc].signe = -1;
        
        sortie_rc = 1 - sortie_rc;
    }
    
    //On fusionne les composantes biconnexes, et on supprime le sommet racine
    //On redirige les arêtes
    int pos = gtilde->R[rc].lien[0].index;
    while(gtilde->A[pos].voisin.index != r){ //On dit que les arêtes pointent maintenant vers r
        gtilde->A[pos].voisin.index = r;
        gtilde->A[pos].voisin.lieu = dansS;

        pos = gtilde->A[pos].adj[1].index;
    }
	//On retire rc des racines pertinentes de r
	gtilde->S[r].racines_pertinentes = suppression(gtilde->R[rc].pertinence);
    gtilde->R[rc].pertinence = NULL;
    //On retire c de la liste des enfants DFS de r
	check_list(gtilde->S[rc].p_parentDFS);
    gtilde->S[r].enfantsDFS = suppression(gtilde->S[rc].p_parentDFS);
	gtilde->S[rc].p_parentDFS = NULL;
	check_list(gtilde->S[r].enfantsDFS);

    //Enfin, on fusionne les listes d'adjacence dans le bon ordre 
    lien_adjacence rin = gtilde->S[r].adj[entree_r];
    lien_adjacence rcout = gtilde->R[rc].lien[sortie_rc];
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
    //c est l'enfant d'où part la descente
    sommet_racine vc = gtilde->R[c];
    int v = vc.parent;

    //On vide la pile
    while (gtilde->P != NULL){
        gtilde->P = pop(gtilde->P);
    }

    for(int dir = 0; dir <2; dir++){ //On part dans les deux directions
        demi_arete w = gtilde->A[gtilde->R[c].lien[1-dir].index]; //modif dir
        int entree_w = 1-dir;
        successeur_face_ext(*gtilde, &w, &entree_w);
        while(w.voisin.lieu != dansR || w.voisin.index != c){ //Tant qu'on n'est pas revenu au point de départ
            //Invariant de boucle: w est une demi-arete liée à un sommet réel (pas racine)
            //w est la demi arête par laquelle on est entré dans le sommet
            assert(w.voisin.lieu == dansS);
            sommet sw = gtilde->S[w.voisin.index];

            if(sw.flag_arete_retour == v){ //On a trouvé une arete retour à ajouter
                while(gtilde->P != NULL){
                    fusion_compo_biconnexes(gtilde);
                }
                ajout_arete_retour(gtilde, c, dir, w.voisin.index, entree_w);
                gtilde->S[w.voisin.index].flag_arete_retour = gtilde->n;
            }

            if(sw.racines_pertinentes != NULL){
                //On doit push w et entree w
                gtilde->P = push(gtilde->P, w.voisin.index);
                gtilde->P = push(gtilde->P, entree_w);
                //On continue de descendre: wc est le suivant
                int swc = sw.racines_pertinentes->val; //Premier élément des racines pertinentes (sommet)
				demi_arete x = gtilde->A[gtilde->R[swc].lien[1].index]; int entree_x = 1; 
                demi_arete y = gtilde->A[gtilde->R[swc].lien[0].index]; int entree_y = 0;
                successeur_face_ext_actif(*gtilde, &x, &entree_x, v); 
                successeur_face_ext_actif(*gtilde, &y, &entree_y, v);

                if(actif_interne(*gtilde, v, x.voisin.index)){w = x; entree_w = entree_x;}
                else if(actif_interne(*gtilde, v, y.voisin.index)){w = y; entree_w = entree_y;}
                else if(pertinent(*gtilde, v, x.voisin.index)){w = x; entree_w = entree_x;}
                else {w = y; entree_w = entree_y;}
                fflush(stdout);
                int sortie_w; 
                if(w.adj[1].index == x.adj[1].index) sortie_w = 0;
                else sortie_w = 1;
                //On push wc et sortie_w
                gtilde->P = push(gtilde->P, swc);
                gtilde->P = push(gtilde->P, sortie_w);

            } else if(inactif(*gtilde, v, w.voisin.index)){
                successeur_face_ext(*gtilde, &w, &entree_w);
            } else { //w est un sommet stoppant
                if(gtilde->P == NULL && gtilde->S[c].point_min < v){
                    if(gtilde->A[gtilde->S[c].adj[0].index].voisin.index == w.voisin.index){
                    } else{
                        ajout_arete_raccourci(gtilde, c, dir, w.voisin.index, entree_w);
                    }
                    
                }
                break;
            }
        }
        if(gtilde->P != NULL) break;
    }
}

void DFS_final(graphe_comb gtilde, graphe* res, int orientation, int s, double_liste** arbre_DFS){
    /*  arbre_DFS: tableau contenant des listes chainées des enfants DFS du graphe de base
        associations: tableau contenant les associations de sommets-arêtes correspondant à la même arête*/
    //N'emprunte que des arêtes du premier DFS
    int signe_sommet = gtilde.S[s].signe*orientation;
    if(s < gtilde.n/5){
        //Recopie des arêtes réelles
        if(signe_sommet > 0){
            res->adj[s] = malloc(4*sizeof(int));
            int i = 0;
            int w = gtilde.S[s].adj[0].index;
            assert(w!=-1);
            if(gtilde.A[w].reelle){
                res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                i++;
            }
            w = gtilde.A[w].adj[1].index;
            while(w != gtilde.S[s].adj[0].index){
                if(gtilde.A[w].reelle){
                    assert(i<4);
                    res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                    i++;
                }
                w = gtilde.A[w].adj[1].index;
            }
        } else { //retourne la liste
            res->adj[s] = malloc(4*sizeof(int));
            int i = 0;
            int w = gtilde.S[s].adj[1].index;
            assert(w != -1);
            if(gtilde.A[w].reelle){
                res->adj[s][i] = gtilde.A[gtilde.A[w].jumelle].voisin.index;
                i++;
            }
            w = gtilde.A[w].adj[0].index;
            while(w != gtilde.S[s].adj[1].index){
                if(gtilde.A[w].reelle){
                    assert(i<4);
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
    res.type = malloc(res.n*sizeof(int*));
    for(int s=0; s<res.n; s++){
        res.type[s] = malloc(4*sizeof(int));
    }

    DFS_final(gtilde, &res, 1, 0, arbre_DFS);

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

    //On veut donner des identifiants aux arêtes
    int nb_aretes = 0;
    //Les identifiants seront stockés dans type
    for(int s=0; s<res.n; s++){
        for(int j = 0; j<4; j++){
            res.type[s][j] = -1;
        }
    }
    //On commence par les arêtes doubles
    for(int s=0; s<res.n; s++){
        for(int j = 0; j<4; j++){
            if(res.type[s][j] == -1){
                if(res.adj[s][j] == res.adj[s][(j+1)%4]){ //arête double, forcément consécutives
                    //On cherche l'ancrage double de l'autre côté
                    int v = res.adj[s][j];
                    int opp;
                    for(int k = 0; k<4; k++){
                        if(res.adj[v][k] == res.adj[v][(k+1)%4] && res.adj[v][k] == s){ //arête double vers s
                            opp = k;
                        }
                    }
                    res.type[s][j] = nb_aretes;
                    res.type[v][(opp+1)%4] = nb_aretes++;
                    res.type[s][(j+1)%4] = nb_aretes;
                    res.type[v][opp] = nb_aretes++;
                }
            }
        }
    }
    //Puis les arêtes simples
    for(int s=0; s<res.n; s++){
        for(int j = 0; j<4; j++){
            if(res.type[s][j] == -1){ //Pas déjà traitée
                res.type[s][j] = nb_aretes;
                int v = res.adj[s][j];
                for(int k = 0; k<4; k++){
                    if(res.adj[v][k] == s){
                        res.type[v][k] = nb_aretes++;
                    }
                }
            }
        }
    }
    return res;
}

void print_graphe_final(graphe g){
    FILE* f = fopen("noeuds.txt", "a");
    fprintf(f, "%d ", g.n);
    for(int i=0; i<g.n; i++){
        // printf("%d: ", i);
        for(int j = 0; j<4; j++){
            fprintf(f, "%d %d ", g.adj[i][j], g.type[i][j]);
        }
        // printf("\n");
    }
    fprintf(f, "\n");
}


graphe BoyerMyrvold(seq_dt seq){
    /*Prend une séquence DT et renvoie le graphe combinatoire après exécution de l'algo de Boyer Myrvold*/
        //Precalcul
    //Transformation en un graphe
    graphe g = conversion_seqDT(seq);
    //Simplification du graphe
    int* associations = malloc(5*g.n*sizeof(int));
    graphe g_simple = simplifier_graphe(g, associations);
    double_liste** arbre_DFS = precalcul_graphe(&g_simple);
    //Initialisation du graphe combinatoire
    graphe_comb gtilde = init_graphe_comb(g_simple);
    precalcul(g_simple, &gtilde);

        //Boucle principale
    for(int i = g_simple.n-1; i>=0; i--){ //On traite les sommets par ordre DFI descendant
        int v = g_simple.DFI[i];
        //Pour chaque enfant de v
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
            suivant = suivant->suiv;
        }

        //Pour chaque arete retour entre v et un de ses descendants w
        for(int j = 0; j<4; j++){
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
    }
    
    //On rajoute le sommet 0
    double_liste* c = gtilde.S[0].enfantsDFS;
    //On redirige les arêtes
    int pos = gtilde.R[c->val].lien[0].index;
    while(gtilde.A[pos].voisin.index != 0){ //On dit que les arêtes pointent maintenant vers 0
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

    //A FAIRE Tout libérer

    graphe res = extraction_BM(gtilde, g_simple, arbre_DFS, associations);
    return res;
}

/*Changer le truc des liens/lieu d'adjacence c'est ridicule un peu*/
int main(int argc, char *argv[]){
    // int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    // seq_dt noeud_wiki = {.taille = 6, .seq = seq};
    // graphe g = BoyerMyrvold(noeud_wiki);
    
    // printf("\nok!\n");
    // fflush(stdout);

    // int seq[6] = {3, (-6), 1, 4, (-2), (-5)};
    // seq_dt noeud = {.taille = 6, .seq = seq};
    // graphe g = BoyerMyrvold(noeud);
    // printf("%d ", atoi(argv[1]));

    seq_dt noeud;
    init_seq_dt(&noeud, atoi(argv[1]));

    for(int i=2; i<argc; i++){
        noeud.seq[i-2] = atoi(argv[i]);
    }
    graphe g = BoyerMyrvold(noeud);
    print_graphe_final(g);
    FILE* f = fopen("sequencesok2.txt", "a");
    fprint_seq_dt(&noeud, f);
    printf("ok\n");
    fclose(f);
    return 0;
}