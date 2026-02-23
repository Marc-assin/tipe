#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

struct voisin {
    int id_arete;
    int sommet;
};

typedef struct voisin voisin;

struct plongement {
    int n;
    voisin **adj; //dans la cas i : les 4 voisins du sommet i, dans leur ordre combinatoire
};

typedef struct plongement plongement;

struct face {
    int n_sommets;
    voisin *sommets;
};

typedef struct face face;

struct faces_arr {
    int n;
    face *faces;
};

typedef struct faces_arr faces_arr;

faces_arr calculer_faces(plongement p){
    int n = p.n;
    bool **vus = malloc(sizeof(bool*) * n);
    for (int i = 0; i < n; i++){
        vus[i] = malloc(sizeof(bool) * 4);
        for (int j = 0; j < 4; j++){
            vus[i][j] = false;
        }
    }
    
    face *f = malloc(sizeof(face) * n * 4); // le nombre de faces est au plus de 4n car le graphe est 4-régulier
    int i_face = 0;
    voisin *proc_sommet = malloc(sizeof(voisin) * n);

    for (int sd = 0; sd < n; sd++){ // sd = sommet de départ
        for (int i = 0; i < 4; i++){ // i = indice du sommet suivant
            // printf("working on sd=%d, i=%d\n", sd, i);
            if (vus[sd][i]) continue;
            int taille_f = 1;
            vus[sd][i] = true;
            int s1 = sd;
            int s2 = p.adj[s1][i].sommet;
            proc_sommet[s1] = p.adj[s1][i];
            int last_j = i;
            
            while (s2 != sd){
                // printf("s1=%d, s2=%d", s1, s2);
                int j = 0;
                while (p.adj[s2][j].id_arete != p.adj[s1][last_j].id_arete){
                    // printf("j=%d, p.adj[s2][j].id_arete=%d, p.adj[s1][i].id_arete=%d, s1=%d, s2=%d\n", j, p.adj[s2][j].id_arete, p.adj[s1][i].id_arete, s1, s2);
                    j = (j+1)%4;
                }
                j = (j+1)%4;
                int s3 = p.adj[s2][j].sommet;
                proc_sommet[s2] = p.adj[s2][j];
                vus[s2][j] = true;
                s1 = s2;
                s2 = s3;
                taille_f++;
                last_j = j;
            }

            f[i_face].n_sommets = taille_f;
            f[i_face].sommets = malloc(sizeof(voisin) * taille_f);
            f[i_face].sommets[0] = proc_sommet[sd];
            for (int fi = 1; fi < taille_f; fi++){
                f[i_face].sommets[fi] = proc_sommet[f[i_face].sommets[fi-1].sommet];
            }

            i_face++;
        }
    }

    for (int i = 0; i < n; i++){
        free(vus[i]);
    }
    free(vus);
    free(proc_sommet);

    faces_arr res = {.n = i_face, .faces = f};
    return res;
}

void afficher_face(face f){
    for (int i = 0; i < f.n_sommets; i++){
        printf("%d ", f.sommets[i].sommet);
    }
    printf("\n");
}

int test_calculer_faces(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    {
        adj[0][0].id_arete = 3;
        adj[0][0].sommet = 5;
        adj[0][1].id_arete = 2;
        adj[0][1].sommet = 2;
        adj[0][2].id_arete = 4;
        adj[0][2].sommet = 3;
        adj[0][3].id_arete = 5;
        adj[0][3].sommet = 4;
        
        adj[1][0].id_arete = 11;
        adj[1][0].sommet = 2;
        adj[1][1].id_arete = 1;
        adj[1][1].sommet = 2;
        adj[1][2].id_arete = 0;
        adj[1][2].sommet = 5;
        adj[1][3].id_arete = 10;
        adj[1][3].sommet = 5;
        
        adj[2][0].id_arete = 1;
        adj[2][0].sommet = 1;
        adj[2][1].id_arete = 11;
        adj[2][1].sommet = 1;
        adj[2][2].id_arete = 7;
        adj[2][2].sommet = 3;
        adj[2][3].id_arete = 2;
        adj[2][3].sommet = 0;
        
        adj[3][0].id_arete = 7;
        adj[3][0].sommet = 2;
        adj[3][1].id_arete = 8;
        adj[3][1].sommet = 4;
        adj[3][2].id_arete = 6;
        adj[3][2].sommet = 4;
        adj[3][3].id_arete = 4;
        adj[3][3].sommet = 0;
        
        adj[4][0].id_arete = 5;
        adj[4][0].sommet = 0;
        adj[4][1].id_arete = 6;
        adj[4][1].sommet = 3;
        adj[4][2].id_arete = 8;
        adj[4][2].sommet = 3;
        adj[4][3].id_arete = 9;
        adj[4][3].sommet = 5;
        
        adj[5][0].id_arete = 0;
        adj[5][0].sommet = 1;
        adj[5][1].id_arete = 3;
        adj[5][1].sommet = 0;
        adj[5][2].id_arete = 9;
        adj[5][2].sommet = 4;
        adj[5][3].id_arete = 10;
        adj[5][3].sommet = 1;
    }

    plongement p = {n, adj};

    faces_arr fa = calculer_faces(p);

    for (int i = 0; i < fa.n; i++){
        afficher_face(fa.faces[i]);
    }

    for (int i = 0; i < fa.n; i++){
        free(fa.faces[i].sommets);
    }
    free(fa.faces);
    for (int i = 0; i < n; i++){
        free(adj[i]);
    }
    free(adj);
    return 0;
}


struct graphe_tait {
    int n;
    face *sommets;
    int **adj;
    int *deg;
};

typedef struct graphe_tait graphe_tait;

bool test_faces_voisines(face f1, face f2){
    bool sommet_commun = false;
    for (int i = 0; i < f1.n_sommets; i++){
        for (int j = 0; j < f2.n_sommets; j++){
            if (f1.sommets[i].sommet == f2.sommets[j].sommet) sommet_commun = true;
            if (f1.sommets[i].id_arete == f2.sommets[j].id_arete) return false;
        }
    }
    return sommet_commun;
    // il faut avoir un sommet en commun, et pas d'arete en commun pour etre voisin
}

graphe_tait calculer_graphe_tait(plongement p){
    faces_arr fa = calculer_faces(p);
    int max_n = fa.n;
    face *faces = fa.faces;

    bool *vus = malloc(sizeof(bool) * max_n);
    for (int i = 0; i < max_n; i++){
        vus[i] = false;
    }
    int *file = malloc(sizeof(int) * max_n);
    file[0] = 0; // a la fin du bfs, les indices des faces présentes dans file[0:r] sont les indices des faces qui sont les sommets du graphe de Tait
    vus[0] = true;
    int r = 1;
    for (int l = 0; l < r; l++){
        // calculer les voisins, et les ajouter à la file si on ne les a pas encore vus
        for (int i = 0; i < max_n; i++){
            if (vus[i]) continue;
            if (test_faces_voisines(faces[file[l]], faces[i])){
                vus[i] = true;
                file[r] = i;
                r++;
            }
        }
    }
    
    graphe_tait gt;
    gt.n = r;
    gt.deg = malloc(sizeof(int) * gt.n);
    gt.adj = malloc(sizeof(int*) * gt.n);
    gt.sommets = malloc(sizeof(face) * gt.n);
    for (int i = 0; i < r; i++){
        gt.sommets[i] = faces[file[i]];
    }
    int *temp_voisins = malloc(sizeof(int) * gt.n);
    for (int i = 0; i < r; i++){
        gt.deg[i] = 0;
        for (int j = 0; j < gt.n; j++){
            if (test_faces_voisines(gt.sommets[i], gt.sommets[j])){
                temp_voisins[gt.deg[i]] = j; // une face ne peut pas etre voisine d'elle même
                gt.deg[i]++;
            }
        }
        gt.adj[i] = malloc(sizeof(face) * gt.deg[i]);
        for (int j = 0; j < gt.deg[i]; j++){
            gt.adj[i][j] = temp_voisins[j];
        }
    }
    
    
    for (int i = 0; i < fa.n; i++){
        if (vus[i] == false){
            free(fa.faces[i].sommets);
        }
    }
    free(temp_voisins);
    free(faces);
    free(vus);
    free(file);
    return gt;
}

int main(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    {
        adj[0][0].id_arete = 3;
        adj[0][0].sommet = 5;
        adj[0][1].id_arete = 2;
        adj[0][1].sommet = 2;
        adj[0][2].id_arete = 4;
        adj[0][2].sommet = 3;
        adj[0][3].id_arete = 5;
        adj[0][3].sommet = 4;
        
        adj[1][0].id_arete = 11;
        adj[1][0].sommet = 2;
        adj[1][1].id_arete = 1;
        adj[1][1].sommet = 2;
        adj[1][2].id_arete = 0;
        adj[1][2].sommet = 5;
        adj[1][3].id_arete = 10;
        adj[1][3].sommet = 5;
        
        adj[2][0].id_arete = 1;
        adj[2][0].sommet = 1;
        adj[2][1].id_arete = 11;
        adj[2][1].sommet = 1;
        adj[2][2].id_arete = 7;
        adj[2][2].sommet = 3;
        adj[2][3].id_arete = 2;
        adj[2][3].sommet = 0;
        
        adj[3][0].id_arete = 7;
        adj[3][0].sommet = 2;
        adj[3][1].id_arete = 8;
        adj[3][1].sommet = 4;
        adj[3][2].id_arete = 6;
        adj[3][2].sommet = 4;
        adj[3][3].id_arete = 4;
        adj[3][3].sommet = 0;
        
        adj[4][0].id_arete = 5;
        adj[4][0].sommet = 0;
        adj[4][1].id_arete = 6;
        adj[4][1].sommet = 3;
        adj[4][2].id_arete = 8;
        adj[4][2].sommet = 3;
        adj[4][3].id_arete = 9;
        adj[4][3].sommet = 5;
        
        adj[5][0].id_arete = 0;
        adj[5][0].sommet = 1;
        adj[5][1].id_arete = 3;
        adj[5][1].sommet = 0;
        adj[5][2].id_arete = 9;
        adj[5][2].sommet = 4;
        adj[5][3].id_arete = 10;
        adj[5][3].sommet = 1;
    }

    plongement p = {n, adj};

    graphe_tait gt = calculer_graphe_tait(p);

    for (int i = 0; i < gt.n; i++){
        afficher_face(gt.sommets[i]);
    }

    
    for (int i = 0; i < gt.n; i++){
        free(gt.sommets[i].sommets);
    }
    free(gt.sommets);
    for (int i = 0; i < gt.n; i++){
        free(gt.adj[i]);
    }
    free(gt.adj);
    free(gt.deg);
    for (int i = 0; i < n; i++){
        free(adj[i]);
    }
    free(adj);
}

/*
pour calculer le graphe de Tait :
    on calcule l'ensemble des faces
    a partir d'une face qcq, on fait un bfs, où les voisins de la face f sont les faces qui ne partagent pas d'arête avec f.
    l'ensemble des faces parcourues est l'ensemble des sommets du graphe de Tait, et les arêtes sont entre les faces qui partagent un sommet.
*/