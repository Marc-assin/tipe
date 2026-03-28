#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/*
pour calculer le graphe de Tait :
    on calcule l'ensemble des faces
    a partir d'une face qcq, on fait un bfs, où les voisins de la face f sont les faces qui ne partagent pas d'arête avec f.
    l'ensemble des faces parcourues est l'ensemble des sommets du graphe de Tait, et les arêtes sont entre les faces qui partagent un sommet.
*/

struct voisin {
    int id_arete;
    int sommet;
};

typedef struct voisin voisin;

struct plongement {
    int n;
    voisin **adj; //dans la case i : les voisins du sommet i, dans leur ordre combinatoire
    int *deg; //dans la case i : les voisins du sommet i, dans leur ordre combinatoire
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
        vus[i] = malloc(sizeof(bool) * p.deg[i]);
        for (int j = 0; j < p.deg[i]; j++){
            vus[i][j] = false;
        }
    }
    
    face *f = malloc(sizeof(face) * n * 4); // le nombre de faces est au plus de 4n car le graphe est 4-régulier
    int i_face = 0;
    voisin *proc_sommet = malloc(sizeof(voisin) * n);

    for (int sd = 0; sd < n; sd++){ // sd = sommet de départ
        for (int i = 0; i < p.deg[sd]; i++){ // i = indice du sommet suivant
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
                    j = (j+1)%p.deg[s2];
                }
                j = (j+1)%p.deg[s2];
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

void test_calculer_faces(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    int *deg = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    
    adj[0][0].id_arete = 3;
    adj[0][0].sommet = 5;
    adj[0][1].id_arete = 2;
    adj[0][1].sommet = 2;
    adj[0][2].id_arete = 4;
    adj[0][2].sommet = 3;
    adj[0][3].id_arete = 5;
    adj[0][3].sommet = 4;
    deg[0] = 4;
    
    adj[1][0].id_arete = 11;
    adj[1][0].sommet = 2;
    adj[1][1].id_arete = 1;
    adj[1][1].sommet = 2;
    adj[1][2].id_arete = 0;
    adj[1][2].sommet = 5;
    adj[1][3].id_arete = 10;
    adj[1][3].sommet = 5;
    deg[1] = 4;
    
    adj[2][0].id_arete = 1;
    adj[2][0].sommet = 1;
    adj[2][1].id_arete = 11;
    adj[2][1].sommet = 1;
    adj[2][2].id_arete = 7;
    adj[2][2].sommet = 3;
    adj[2][3].id_arete = 2;
    adj[2][3].sommet = 0;
    deg[2] = 4;
    
    adj[3][0].id_arete = 7;
    adj[3][0].sommet = 2;
    adj[3][1].id_arete = 8;
    adj[3][1].sommet = 4;
    adj[3][2].id_arete = 6;
    adj[3][2].sommet = 4;
    adj[3][3].id_arete = 4;
    adj[3][3].sommet = 0;
    deg[3] = 4;
    
    adj[4][0].id_arete = 5;
    adj[4][0].sommet = 0;
    adj[4][1].id_arete = 6;
    adj[4][1].sommet = 3;
    adj[4][2].id_arete = 8;
    adj[4][2].sommet = 3;
    adj[4][3].id_arete = 9;
    adj[4][3].sommet = 5;
    deg[4] = 4;
    
    adj[5][0].id_arete = 0;
    adj[5][0].sommet = 1;
    adj[5][1].id_arete = 3;
    adj[5][1].sommet = 0;
    adj[5][2].id_arete = 9;
    adj[5][2].sommet = 4;
    adj[5][3].id_arete = 10;
    adj[5][3].sommet = 1;
    deg[5] = 4;

    plongement p = {n, adj, deg};

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
    free(deg);
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
        gt.adj[i] = malloc(sizeof(int) * gt.deg[i]);
        for (int j = 0; j < gt.deg[i]; j++){
            for (int k = 0; k < gt.deg[i]; k++){
                int s = gt.sommets[i].sommets[j].sommet;
                for (int l = 0; l < gt.sommets[temp_voisins[k]].n_sommets; l++){
                    if (gt.sommets[temp_voisins[k]].sommets[l].sommet == s){ 
                        // seulement un seul des voisins de f lui partage le sommet s. De cette manière, les voisins sont dans le meme ordre que les sommets autour de f, ce qui fait que gt est un plongement combinatoire
                        gt.adj[i][j] = temp_voisins[k];
                    }
                }
            }
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

void test_calculer_graphe_tait(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    int *deg = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    
    adj[0][0].id_arete = 3;
    adj[0][0].sommet = 5;
    adj[0][1].id_arete = 2;
    adj[0][1].sommet = 2;
    adj[0][2].id_arete = 4;
    adj[0][2].sommet = 3;
    adj[0][3].id_arete = 5;
    adj[0][3].sommet = 4;
    deg[0] = 4;
    
    adj[1][0].id_arete = 11;
    adj[1][0].sommet = 2;
    adj[1][1].id_arete = 1;
    adj[1][1].sommet = 2;
    adj[1][2].id_arete = 0;
    adj[1][2].sommet = 5;
    adj[1][3].id_arete = 10;
    adj[1][3].sommet = 5;
    deg[1] = 4;
    
    adj[2][0].id_arete = 1;
    adj[2][0].sommet = 1;
    adj[2][1].id_arete = 11;
    adj[2][1].sommet = 1;
    adj[2][2].id_arete = 7;
    adj[2][2].sommet = 3;
    adj[2][3].id_arete = 2;
    adj[2][3].sommet = 0;
    deg[2] = 4;
    
    adj[3][0].id_arete = 7;
    adj[3][0].sommet = 2;
    adj[3][1].id_arete = 8;
    adj[3][1].sommet = 4;
    adj[3][2].id_arete = 6;
    adj[3][2].sommet = 4;
    adj[3][3].id_arete = 4;
    adj[3][3].sommet = 0;
    deg[3] = 4;
    
    adj[4][0].id_arete = 5;
    adj[4][0].sommet = 0;
    adj[4][1].id_arete = 6;
    adj[4][1].sommet = 3;
    adj[4][2].id_arete = 8;
    adj[4][2].sommet = 3;
    adj[4][3].id_arete = 9;
    adj[4][3].sommet = 5;
    deg[4] = 4;
    
    adj[5][0].id_arete = 0;
    adj[5][0].sommet = 1;
    adj[5][1].id_arete = 3;
    adj[5][1].sommet = 0;
    adj[5][2].id_arete = 9;
    adj[5][2].sommet = 4;
    adj[5][3].id_arete = 10;
    adj[5][3].sommet = 1;
    deg[5] = 4;


    plongement p = {n, adj, deg};

    graphe_tait gt = calculer_graphe_tait(p);

    for (int i = 0; i < gt.n; i++){
        afficher_face(gt.sommets[i]);
        for (int j = 0; j < gt.deg[i]; j++){
            printf(" %d", gt.adj[i][j]);
        }
        printf("\n");
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

#include "int_vector.c"

struct graphe {
    int n;
    int_vector **adj;
};

typedef struct graphe graphe;

graphe trianguler_faces(plongement p){
    faces_arr faces = calculer_faces(p);
    graphe g;
    g.n = p.n+faces.n;
    g.adj = malloc(sizeof(int_vector*) * g.n);
    for (int i = 0; i < p.n; i++){
        g.adj[i] = init_ivec(p.deg[i]*2);
        for (int j = 0; j < p.deg[i]; j++){ // chaque demie arête est dans une et une seule face
            bool done = false;
            for (int f = 0; f < faces.n; f++){
                for (int s = 0; s < faces.faces[f].n_sommets; s++){
                    if (faces.faces[f].sommets[s].id_arete == p.adj[i][j].id_arete &&
                        faces.faces[f].sommets[s].sommet == p.adj[i][j].sommet){
                        append_ivec(g.adj[i], f+p.n);
                        done = true;
                        break;
                    }
                }
                if (done) break;
            }
            append_ivec(g.adj[i], p.adj[i][j].sommet);
        }
    }
    for (int i = 0; i < g.n-p.n; i++){
        g.adj[i+p.n] = init_ivec(faces.faces[i].n_sommets);
        for (int j = faces.faces[i].n_sommets - 1; j >= 0; j--){
            append_ivec(g.adj[i+p.n], faces.faces[i].sommets[j].sommet);
        }
    }
    for (int i = 0; i < faces.n; i++) {
        free(faces.faces[i].sommets);
    }
    free(faces.faces);
    return g;
}

plongement graphe_vers_plongement(graphe g){
    int n = g.n;
    plongement p;
    int **indice_arete;
    indice_arete = malloc(sizeof(int*) * n);
    for (int i = 0; i < n; i++){
        indice_arete[i] = malloc(sizeof(int) * n);
        for (int j = 0; j < n; j++){
            indice_arete[i][j] = -1;
        }
    }
    int compteur_indice_aretes = 0;
    p.n = n;
    p.deg = malloc(sizeof(int) * n);
    p.adj = malloc(sizeof(voisin*) * n);
    for (int i = 0; i < n; i++){
        p.deg[i] = g.adj[i]->taille;
        p.adj[i] = malloc(sizeof(voisin) * p.deg[i]);
        for (int j = 0; j < p.deg[i]; j++){
            voisin v;
            v.sommet = get_ivec(g.adj[i], j);
            if (indice_arete[i][v.sommet] == -1){
                indice_arete[i][v.sommet] = compteur_indice_aretes;
                indice_arete[v.sommet][i] = compteur_indice_aretes;
                compteur_indice_aretes++;
            }
            v.id_arete = indice_arete[i][v.sommet];
            p.adj[i][j] = v;
        }
    }
    for (int i = 0; i < n; i++){
        free(indice_arete[i]);
    }
    free(indice_arete);
    return p;
}

plongement gt_vers_plongement(graphe_tait gt){
    plongement p;
    int n = gt.n;

    p.n = n;
    p.deg = malloc(sizeof(int) * n);
    p.adj = malloc(sizeof(voisin*) * n);

    int **indice_arete = malloc(sizeof(int*) * n);
    for (int i = 0; i < n; i++){
        indice_arete[i] = malloc(sizeof(int) * n);
        for (int j = 0; j < n; j++){
            indice_arete[i][j] = -1;
        }
    }

    int compteur_aretes = 0;

    for (int i = 0; i < n; i++){
        p.deg[i] = gt.deg[i];
        p.adj[i] = malloc(sizeof(voisin) * p.deg[i]);

        for (int j = 0; j < p.deg[i]; j++){
            int v = gt.adj[i][j];

            voisin w;
            w.sommet = v;

            if (indice_arete[i][v] == -1){
                indice_arete[i][v] = compteur_aretes;
                indice_arete[v][i] = compteur_aretes;
                compteur_aretes++;
            }

            w.id_arete = indice_arete[i][v];
            p.adj[i][j] = w;
        }
    }

    for (int i = 0; i < n; i++){
        free(indice_arete[i]);
    }
    free(indice_arete);

    return p;
}

void afficher_plongement(plongement p){
    printf("Plongement (n = %d)\n", p.n);
    for (int i = 0; i < p.n; i++){
        printf("%d : ", i);
        for (int j = 0; j < p.deg[i]; j++){
            printf("[%d, %d] ",
                   p.adj[i][j].sommet,
                   p.adj[i][j].id_arete);
        }
        printf("\n");
    }
}

void afficher_graphe_tait(graphe_tait gt){
    printf("Graphe de Tait (n = %d)\n", gt.n);

    for (int i = 0; i < gt.n; i++){
        printf("Sommet %d : face = ", i);

        // afficher la face correspondante
        for (int j = 0; j < gt.sommets[i].n_sommets; j++){
            printf("%d ", gt.sommets[i].sommets[j].sommet);
        }

        printf("\n    deg = %d, voisins : ", gt.deg[i]);

        for (int j = 0; j < gt.deg[i]; j++){
            printf("%d ", gt.adj[i][j]);
        }

        printf("\n");
    }
}

void afficher_graphe(graphe g){
    printf("Graphe (n = %d)\n", g.n);
    for (int i = 0; i < g.n; i++){
        printf("%d : ", i);
        for (int j = 0; j < g.adj[i]->taille; j++){
            printf("%d ", get_ivec(g.adj[i], j));
        }
        printf("\n");
    }
}

void free_plongement(plongement p){
    for (int i = 0; i < p.n; i++){
        free(p.adj[i]);   // tableau de voisin
    }
    free(p.adj);
    free(p.deg);
}

void free_graphe_tait(graphe_tait gt){
    for (int i = 0; i < gt.n; i++){
        free(gt.sommets[i].sommets);
    }
    free(gt.sommets);

    for (int i = 0; i < gt.n; i++){
        free(gt.adj[i]);
    }
    free(gt.adj);
    free(gt.deg);
}

void free_graphe(graphe g){
    for (int i = 0; i < g.n; i++){
        free_ivec(g.adj[i]);
    }
    free(g.adj);
}

void afficher_faces_arr(faces_arr fa){
    printf("Faces_arr (n = %d)\n", fa.n);

    for (int i = 0; i < fa.n; i++){
        printf("Face %d (taille = %d) : ",
               i,
               fa.faces[i].n_sommets);

        for (int j = 0; j < fa.faces[i].n_sommets; j++){
            printf("%d ", fa.faces[i].sommets[j].sommet);
        }

        printf("\n");
    }
}

void free_faces_arr(faces_arr fa){
    if (fa.faces == NULL) return;

    for (int i = 0; i < fa.n; i++){
        free(fa.faces[i].sommets);
    }

    free(fa.faces);
}

// int main(){
void test_trianguler_graphe(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*)*n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    int *deg = malloc(sizeof(int)*n);
    
    adj[0][0].id_arete = 0;
    adj[0][0].sommet = 1;
    adj[0][1].id_arete = 3;
    adj[0][1].sommet = 3;
    adj[0][2].id_arete = 6;
    adj[0][2].sommet = 5;
    deg[0] = 3;
    
    adj[1][0].id_arete = 0;
    adj[1][0].sommet = 0;
    adj[1][1].id_arete = 7;
    adj[1][1].sommet = 5;
    adj[1][2].id_arete = 1;
    adj[1][2].sommet = 2;
    deg[1] = 3;
    
    adj[2][0].id_arete = 2;
    adj[2][0].sommet = 3;
    adj[2][1].id_arete = 1;
    adj[2][1].sommet = 1;
    deg[2] = 2;
    
    adj[3][0].id_arete = 2;
    adj[3][0].sommet = 2;
    adj[3][1].id_arete = 4;
    adj[3][1].sommet = 4;
    adj[3][2].id_arete = 3;
    adj[3][2].sommet = 0;
    deg[3] = 3;
    
    adj[4][0].id_arete = 4;
    adj[4][0].sommet = 3;
    adj[4][1].id_arete = 5;
    adj[4][1].sommet = 5;
    deg[4] = 2;
    
    adj[5][0].id_arete = 7;
    adj[5][0].sommet = 1;
    adj[5][1].id_arete = 6;
    adj[5][1].sommet = 0;
    adj[5][2].id_arete = 5;
    adj[5][2].sommet = 4;
    deg[5] = 3;
    
    plongement p = {n, adj, deg};

    graphe g = trianguler_faces(p);
    plongement p2 = graphe_vers_plongement(g);

    // afficher_plongement(p2);

    faces_arr fa = calculer_faces(p2);

    afficher_faces_arr(fa);
    free_faces_arr(fa);
    free_plongement(p);
    free_plongement(p2);
}

struct graphe_angles_reduit {
    graphe H;
    int *cycle_exterieur;
};

typedef struct graphe_angles_reduit graphe_angles_reduit;

graphe_angles_reduit calculer_graphe_angles_reduit(plongement p){
    faces_arr faces = calculer_faces(p);
    graphe g;
    g.n = p.n+faces.n-1;
    g.adj = malloc(sizeof(int_vector*) * g.n);
    for (int i = 0; i < p.n; i++){
        g.adj[i] = init_ivec(p.deg[i]);
        for (int j = 0; j < p.deg[i]; j++){ // chaque demie arête est dans une et une seule face
            bool done = false;
            for (int f = 0; f < faces.n; f++){
                for (int s = 0; s < faces.faces[f].n_sommets; s++){
                    if (faces.faces[f].sommets[s].id_arete == p.adj[i][j].id_arete &&
                        faces.faces[f].sommets[s].sommet == p.adj[i][j].sommet){
                        if (f+p.n < g.n) { // la face exterieure est la dernière de faces, il ne faut pas l'ajouter
                            append_ivec(g.adj[i], f+p.n);
                        } else {
                            append_ivec(g.adj[i], -1);
                        }
                        done = true;
                        break;
                    }
                }
                if (done) break;
            }
            // append_ivec(g.adj[i], p.adj[i][j].sommet); // on n'ajoute pas les arêtes originales de p
        }
    }
    for (int i = 0; i < g.n-p.n; i++){
        g.adj[i+p.n] = init_ivec(faces.faces[i].n_sommets);
        for (int j = faces.faces[i].n_sommets - 1; j >= 0; j--){
            append_ivec(g.adj[i+p.n], faces.faces[i].sommets[j].sommet);
        }
    }
    graphe_angles_reduit gar = {g, malloc(sizeof(int) * faces.faces[faces.n - 1].n_sommets)};
    for (int i = 0; i < faces.faces[faces.n - 1].n_sommets; i++){
        gar.cycle_exterieur[i] = faces.faces[faces.n - 1].sommets[i].sommet;
    }
    for (int i = 0; i < faces.n; i++) {
        free(faces.faces[i].sommets);
    }
    free(faces.faces);
    return gar;
}

struct arete {
    int x, y;
};

typedef struct arete arete;

struct arete_arr {
    int n;
    arete *aretes;
};

typedef struct arete_arr arete_arr;

arete_arr calculer_aretes(graphe g){
    int n_aretes = 0;
    for (int i = 0; i < g.n; i++){
        for (int j = 0; j < g.adj[i]->taille; j++){
            if (i < get_ivec(g.adj[i], j)) n_aretes ++;
        }
    }
    arete_arr aretes = {n_aretes, malloc(sizeof(arete) * n_aretes)};
    int i_arete = 0;
    for (int i = 0; i < g.n; i++){
        for (int j = 0; j < g.adj[i]->taille; j++){
            if (i < get_ivec(g.adj[i], j)) {
                aretes.aretes[i_arete].x = i;
                aretes.aretes[i_arete].y = get_ivec(g.adj[i], j);
                i_arete++;
            }
        }
    }
    return aretes;
}

#include <math.h>
#define PI 3.14159265358979323846

double phi_prime(int s, double *x, graphe H){
    double sum = 0.0;
    for (int i = 0; i < H.adj[s]->taille; i++){
        int j = get_ivec(H.adj[s], i);
        sum += atan(exp(x[s]-x[j])) - atan(exp(x[j]-x[s])) - PI / 2.0;
    }
    sum += PI * 2.0;
    return sum;
}

double* calculer_radii(graphe_angles_reduit gar, double coeff_apprentissage, double epsilon){
    graphe H = gar.H;
    int n = H.n;
    int s1 = gar.cycle_exterieur[0], s2 = gar.cycle_exterieur[1], s3 = gar.cycle_exterieur[2];
    double *x = malloc(sizeof(double) * n);
    double *dphidx = malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++){
        x[i] = 0.0;
    }
    x[s1] = log(tan(PI/3));
    x[s2] = log(tan(PI/3));
    x[s3] = log(tan(PI/3));
    for (int i = 0; i < n; i++){
        if (i != s1 && i != s2 && i != s3) {
            dphidx[i] = phi_prime(i, x, H);
        } else {
            dphidx[i] = 0;
        }
    }
    double MSE = 0.0;
    for (int i = 0; i < n; i++){
        MSE += dphidx[i]*dphidx[i];
    }
    
    while (MSE > epsilon){
        // printf("%e\n", MSE); 
        for (int i = 0; i < n; i++){
            if (i != s1 && i != s2 && i != s3) {
                x[i] -= coeff_apprentissage * dphidx[i];
            }
        }
        for (int i = 0; i < n; i++){
            if (i != s1 && i != s2 && i != s3) {
                dphidx[i] = phi_prime(i, x, H);
            } else {
                dphidx[i] = 0;
            }
        }
        MSE = 0.0;
        for (int i = 0; i < n; i++){
            MSE += dphidx[i]*dphidx[i];
        }
    }
    double *r = malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++){
        printf("%e\n", phi_prime(i, x, H));
        r[i] = exp(x[i]);
    }
    free(x);
    free(dphidx);
    return r;
}

// int main(){
void test_calculer_radii(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    int *deg = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    
    adj[0][0].id_arete = 3;
    adj[0][0].sommet = 5;
    adj[0][1].id_arete = 2;
    adj[0][1].sommet = 2;
    adj[0][2].id_arete = 4;
    adj[0][2].sommet = 3;
    adj[0][3].id_arete = 5;
    adj[0][3].sommet = 4;
    deg[0] = 4;
    
    adj[1][0].id_arete = 11;
    adj[1][0].sommet = 2;
    adj[1][1].id_arete = 1;
    adj[1][1].sommet = 2;
    adj[1][2].id_arete = 0;
    adj[1][2].sommet = 5;
    adj[1][3].id_arete = 10;
    adj[1][3].sommet = 5;
    deg[1] = 4;
    
    adj[2][0].id_arete = 1;
    adj[2][0].sommet = 1;
    adj[2][1].id_arete = 11;
    adj[2][1].sommet = 1;
    adj[2][2].id_arete = 7;
    adj[2][2].sommet = 3;
    adj[2][3].id_arete = 2;
    adj[2][3].sommet = 0;
    deg[2] = 4;
    
    adj[3][0].id_arete = 7;
    adj[3][0].sommet = 2;
    adj[3][1].id_arete = 8;
    adj[3][1].sommet = 4;
    adj[3][2].id_arete = 6;
    adj[3][2].sommet = 4;
    adj[3][3].id_arete = 4;
    adj[3][3].sommet = 0;
    deg[3] = 4;
    
    adj[4][0].id_arete = 5;
    adj[4][0].sommet = 0;
    adj[4][1].id_arete = 6;
    adj[4][1].sommet = 3;
    adj[4][2].id_arete = 8;
    adj[4][2].sommet = 3;
    adj[4][3].id_arete = 9;
    adj[4][3].sommet = 5;
    deg[4] = 4;
    
    adj[5][0].id_arete = 0;
    adj[5][0].sommet = 1;
    adj[5][1].id_arete = 3;
    adj[5][1].sommet = 0;
    adj[5][2].id_arete = 9;
    adj[5][2].sommet = 4;
    adj[5][3].id_arete = 10;
    adj[5][3].sommet = 1;
    deg[5] = 4;

    plongement p = {n, adj, deg};

    graphe_tait gt = calculer_graphe_tait(p);
    plongement plongement_gt = gt_vers_plongement(gt);
    graphe gt_triangule = trianguler_faces(plongement_gt);
    plongement plongement_gt_triangule = graphe_vers_plongement(gt_triangule);
    graphe_angles_reduit gar = calculer_graphe_angles_reduit(plongement_gt_triangule);

    // afficher_graphe(gar.H);

    double *r = calculer_radii(gar, 0.001, 1e-25);

    for (int i = 0; i < gar.H.n; i++){
        printf("%lf ", r[i]);
    }

    free_plongement(p);
    free_graphe_tait(gt);
    free_plongement(plongement_gt);
    free_graphe(gt_triangule);
    free_plongement(plongement_gt_triangule);
    free_graphe(gar.H);
    free(gar.cycle_exterieur);
    free(r);
}

struct vec {
    double x, y;
};

typedef struct vec vec;

double norme(vec v){
    return sqrt(v.x*v.x + v.y*v.y); 
}

vec normaliser(vec v){
    double n = norme(v);
    if (n == 0){
        vec vn = {1, 0};
        printf("On essaie de normaliser un vecteur nul\n");
        return vn;
    }
    vec vn = {v.x / n, v.y / n};
    return vn;
}

vec add(vec a, vec b){
    vec c = {a.x+b.x, a.y+b.y};
    return c;
}

vec diff(vec a, vec b){
    vec c = {a.x-b.x, a.y-b.y};
    return c;
}

double dist(vec a, vec b){
    return norme(diff(a, b));
}

double det(vec a, vec b) {
    return a.x * b.y - a.y * b.x;
}

void ecrire_entete_python(FILE *f) {
    if (!f) return;
    fprintf(f, "import matplotlib.pyplot as plt\n");
    // fprintf(f, "from matplotlib.patches import FancyArrowPatch\n\n");
}

void ecrire_footing_python(FILE *f) {
    if (!f) return;
    fprintf(f, "plt.gca().set_aspect('equal', adjustable='box')\n");
    fprintf(f, "plt.show()\n");
}

void ecrire_fleche_python(FILE *f, double x1, double y1, double x2, double y2) {
    if (!f) return;
    fprintf(f,
        "plt.annotate('', xy=(float('%f'), float('%f')), xytext=(float('%f'), float('%f')), "
        "arrowprops=dict(arrowstyle='->', color='r', lw=2))\n",
        x1 + x2, y1 + y2, x1, y1
    );
}

void ecrire_point_python(FILE *f, double x, double y, const char* color, int idx) {
    if (!f) return;
    fprintf(f, "plt.plot(float('%f'), float('%f'), 'o', color='%s')\n", x, y, color);
    if (idx >= 0){
        fprintf(f, "plt.annotate('%d', (float('%f'), float('%f')), textcoords='offset points', xytext=(5,5), ha='left')\n", idx, x, y);
    }
}

void ecrire_cercle_python(FILE *f, double x, double y, double r) {
    if (!f) return;
    fprintf(f, "circle = plt.Circle((float('%f'), float('%f')), float('%f'), fill=False, color='c')\n", x, y, r);
    fprintf(f, "plt.gca().add_patch(circle)\n");
}

vec* calculer_positions(double *r, graphe_angles_reduit gar){
    int n = gar.H.n;
    FILE *f = fopen("fleche.py", "w");
    ecrire_entete_python(f);
    vec* positions = malloc(sizeof(vec) * gar.H.n);
    bool* places = malloc(sizeof(bool) * gar.H.n);
    for (int i = 0; i < gar.H.n; i++){
        places[i] = false;
    }
    int t1 = -1;
    int s1 = gar.cycle_exterieur[0], s2 = gar.cycle_exterieur[1];
    for (int i = 0; i < gar.H.n; i++){
        for (int j = 0; j < gar.H.adj[i]->taille; j++){
            if (get_ivec(gar.H.adj[i], j) == s1 && 
                get_ivec(gar.H.adj[i], (j + 1) % gar.H.adj[i]->taille) == s2){
                if (t1 >= 0) printf("found 2\n");
                t1 = i;
                for (int k = 0; k < gar.H.adj[i]->taille; k++){
                    printf("%d ", gar.H.adj[i]->data[k]);
                }
                printf("\n");
                break;
            }
        }
    }
    positions[s1].x = 0;
    positions[s1].y = 0;
    positions[s2].x = r[s1] + r[s2];
    positions[s2].y = 0;
    positions[t1].x = r[s1];
    positions[t1].y = r[t1];
    places[s1] = true; places[s2] = true; places[t1] = true;
    int *file = malloc(sizeof(int) * gar.H.n);
    file[0] = s1;
    file[1] = s2;
    file[2] = t1;
    int pointeur_gauche = 0, pointeur_droite = 3;
    while (pointeur_gauche < pointeur_droite){
        int s = file[pointeur_gauche];
        printf("en train de regarder %d, %e, %e, %e\n", s, positions[s].x, positions[s].y, r[s]);
        // ecrire_point_python(f, positions[s].x, positions[s].y, "b", s);
        ecrire_cercle_python(f, positions[s].x, positions[s].y, r[s]);
        int j = 0; 
        int deg = gar.H.adj[s]->taille;
        while (places[get_ivec(gar.H.adj[s], j)] == false){
            j = (j+1)%deg;
        }
        bool sens_inverse = false;
        for (int d = 0; d < deg - 1; d++){
            int voisin = get_ivec(gar.H.adj[s], j);
            printf("voisin %d, %e, %e, %e\n", voisin, positions[voisin].x, positions[voisin].y, r[voisin]);
            j = (j+1)%deg;
            if (places[get_ivec(gar.H.adj[s], j)]) {
                continue;
            }
            int w = get_ivec(gar.H.adj[s], j); 
            if (w == -1){
                sens_inverse = true;
                j = (deg + j - 1)%deg;
                break;
            }
            printf("pour placer %d, %e\n", w, r[voisin]);
            double actual_dist = dist(positions[s], positions[voisin]);
            double expected_dist = sqrt(r[s]*r[s] + r[voisin]*r[voisin]);
            if (fabs(actual_dist - expected_dist) > 1e-6) {
                printf("Layout Error: Node %d and %d are not at the correct distance!\n", s, voisin);
            }
            
            // 1. Correct the distances based on the Bipartite incidence graph
            // 's' and 'w' are adjacent in H, so they are Orthogonal
            double r1 = sqrt(r[s]*r[s] + r[w]*r[w]); 
            
            // 'voisin' and 'w' share 's' as a neighbor, so they are Tangent
            double r2 = r[voisin] + r[w];  

            double x1 = positions[s].x;
            double y1 = positions[s].y;
            double x2 = positions[voisin].x;
            double y2 = positions[voisin].y;

            double dx = x2 - x1;
            double dy = y2 - y1;

            // Precompute some terms
            double rDiffCar = r1*r1 - r2*r2;
            double xDiffCar = x2*x2 - x1*x1;
            double yDiffCar = y2*y2 - y1*y1;
            double denom = 2.0 * (dx*dx + dy*dy);
            double crois = y1*x2 - y2*x1;

            // Compute solution
            vec barycentre;
            barycentre.x = (dx * (rDiffCar + xDiffCar + yDiffCar) - 2.0 * dy * crois) / denom;
            barycentre.y = (dy * (rDiffCar + xDiffCar + yDiffCar) + 2.0 * dx * crois) / denom;
            
            vec perp; 
            perp.x = positions[s].y - positions[voisin].y;
            perp.y = positions[voisin].x - positions[s].x;
            perp = normaliser(perp);
            
            double di = dist(positions[s], barycentre);
            double scale = r1*r1 - di*di; 
            // if (scale < 0) scale = 0;
            double n_len = sqrt(scale); 
            perp.x *= n_len;
            perp.y *= n_len;
            
            positions[w] = add(barycentre, perp);
            places[w] = true;
            file[pointeur_droite] = w;
            pointeur_droite++;
        }
        if (sens_inverse){
        for (int d = 0; d < deg - 2; d++){
            int voisin = get_ivec(gar.H.adj[s], j);
            printf("voisin %d, %e, %e, %e\n", voisin, positions[voisin].x, positions[voisin].y, r[voisin]);
            j = (deg + j - 1)%deg;
            if (places[get_ivec(gar.H.adj[s], j)]) {
                continue;
            }
            int w = get_ivec(gar.H.adj[s], j); 
            printf("pour placer %d, %e\n", w, r[voisin]);
            double actual_dist = dist(positions[s], positions[voisin]);
            double expected_dist = sqrt(r[s]*r[s] + r[voisin]*r[voisin]);
            if (fabs(actual_dist - expected_dist) > 1e-6) {
                printf("Layout Error: Node %d and %d are not at the correct distance!\n", s, voisin);
            }
            
            // 1. Correct the distances based on the Bipartite incidence graph
            // 's' and 'w' are adjacent in H, so they are Orthogonal
            double r1 = sqrt(r[s]*r[s] + r[w]*r[w]); 
            
            // 'voisin' and 'w' share 's' as a neighbor, so they are Tangent
            double r2 = r[voisin] + r[w];  

            double x1 = positions[s].x;
            double y1 = positions[s].y;
            double x2 = positions[voisin].x;
            double y2 = positions[voisin].y;

            double dx = x2 - x1;
            double dy = y2 - y1;

            // Precompute some terms
            double rDiffCar = r1*r1 - r2*r2;
            double xDiffCar = x2*x2 - x1*x1;
            double yDiffCar = y2*y2 - y1*y1;
            double denom = 2.0 * (dx*dx + dy*dy);
            double crois = y1*x2 - y2*x1;

            // Compute solution
            vec barycentre;
            barycentre.x = (dx * (rDiffCar + xDiffCar + yDiffCar) - 2.0 * dy * crois) / denom;
            barycentre.y = (dy * (rDiffCar + xDiffCar + yDiffCar) + 2.0 * dx * crois) / denom;
            
            vec perp; 
            perp.x = positions[s].y - positions[voisin].y;
            perp.y = positions[voisin].x - positions[s].x;
            perp = normaliser(perp);
            
            double di = dist(positions[s], barycentre);
            double scale = r1*r1 - di*di; 
            // if (scale < 0) scale = 0;
            double n_len = sqrt(scale); 
            perp.x *= n_len;
            perp.y *= n_len;
            
            positions[w] = diff(barycentre, perp);
            places[w] = true;
            file[pointeur_droite] = w;
            pointeur_droite++;
        }}
        pointeur_gauche++;
    }

    for (int i = 0; i < gar.H.n; i++){
        if (places[i]) {
            printf("%e, %e, %e\n", positions[i].x, positions[i].y, r[i]);
        } else {
            printf("non placé : %e\n", r[i]);
        }
    }

    // ecrire_footing_python(f);

    fclose(f);
    free(file);
    free(places);
    return positions;
}

// int main(){
void test_calculer_positions(){
    int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    int *deg = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    
    adj[0][0].id_arete = 3;
    adj[0][0].sommet = 5;
    adj[0][1].id_arete = 2;
    adj[0][1].sommet = 2;
    adj[0][2].id_arete = 4;
    adj[0][2].sommet = 3;
    adj[0][3].id_arete = 5;
    adj[0][3].sommet = 4;
    deg[0] = 4;
    
    adj[1][0].id_arete = 11;
    adj[1][0].sommet = 2;
    adj[1][1].id_arete = 1;
    adj[1][1].sommet = 2;
    adj[1][2].id_arete = 0;
    adj[1][2].sommet = 5;
    adj[1][3].id_arete = 10;
    adj[1][3].sommet = 5;
    deg[1] = 4;
    
    adj[2][0].id_arete = 1;
    adj[2][0].sommet = 1;
    adj[2][1].id_arete = 11;
    adj[2][1].sommet = 1;
    adj[2][2].id_arete = 7;
    adj[2][2].sommet = 3;
    adj[2][3].id_arete = 2;
    adj[2][3].sommet = 0;
    deg[2] = 4;
    
    adj[3][0].id_arete = 7;
    adj[3][0].sommet = 2;
    adj[3][1].id_arete = 8;
    adj[3][1].sommet = 4;
    adj[3][2].id_arete = 6;
    adj[3][2].sommet = 4;
    adj[3][3].id_arete = 4;
    adj[3][3].sommet = 0;
    deg[3] = 4;
    
    adj[4][0].id_arete = 5;
    adj[4][0].sommet = 0;
    adj[4][1].id_arete = 6;
    adj[4][1].sommet = 3;
    adj[4][2].id_arete = 8;
    adj[4][2].sommet = 3;
    adj[4][3].id_arete = 9;
    adj[4][3].sommet = 5;
    deg[4] = 4;
    
    adj[5][0].id_arete = 0;
    adj[5][0].sommet = 1;
    adj[5][1].id_arete = 3;
    adj[5][1].sommet = 0;
    adj[5][2].id_arete = 9;
    adj[5][2].sommet = 4;
    adj[5][3].id_arete = 10;
    adj[5][3].sommet = 1;
    deg[5] = 4;

    plongement p = {n, adj, deg};

    graphe_tait gt = calculer_graphe_tait(p);
    afficher_graphe_tait(gt);
    plongement plongement_gt = gt_vers_plongement(gt);
    afficher_plongement(plongement_gt);
    graphe gt_triangule = trianguler_faces(plongement_gt);
    afficher_graphe(gt_triangule);
    plongement plongement_gt_triangule = graphe_vers_plongement(gt_triangule);
    afficher_plongement(plongement_gt_triangule);
    graphe_angles_reduit gar = calculer_graphe_angles_reduit(plongement_gt_triangule);
    double *r = calculer_radii(gar, 0.001, 1e-23);
    for (int i = 0; i < 3; i++){
        printf("%d ", gar.cycle_exterieur[i]);
    }
    printf("\n");
    afficher_graphe(gar.H);

    vec *positions = calculer_positions(r, gar);

    for (int i = 0; i < gar.H.n; i++){
        printf("%e;%e\n", positions[i].x, positions[i].y);
    }

    free(positions);
    free_plongement(p);
    free_graphe_tait(gt);
    free_plongement(plongement_gt);
    free_graphe(gt_triangule);
    free_plongement(plongement_gt_triangule);
    free_graphe(gar.H);
    free(gar.cycle_exterieur);
    free(r);
}

/*
il faut ensuite trianguler le graphe de Tait T en un graphe G, puis calculer son graphe des angles réduit H' de la manière suivante : 
    on calcule un plongement combinatoire de T (ce que l'on peut faire sans algorithme compliqué, étant donné sa construction, car on a conservé l'ordre des sommets autout de chaque face, ce qui donne l'ordre des voisins de chaque face)
    on rajoute un sommet sur chaque face de T, que l'on relie à chaque sommet de T, en préservant le plongement (ce qui donne G)
    on calcule le dual de G
    on choisit une face quelconque pour etre la face exterieure
*/

double angle(vec A, vec B, vec C) {
    vec BA, BC;

    // Vecteurs BA = A - B et BC = C - B
    BA.x = A.x - B.x;
    BA.y = A.y - B.y;

    BC.x = C.x - B.x;
    BC.y = C.y - B.y;

    // Produit scalaire
    double dot = BA.x * BC.x + BA.y * BC.y;

    // Produit vectoriel 2D (scalaire)
    double cross = BA.x * BC.y - BA.y * BC.x;

    // Angle entre -pi et pi
    double angle = atan2(cross, dot);

    // Conversion vers [0, 2pi)
    if (angle < 0) {
        angle += 2.0 * PI;
    }

    return angle;
}

void calculer_svg(vec *pos, double *r, graphe_tait gt, char* filename){
    int n = 0;
    for (int i = 0; i < gt.n; i++){
        for (int j = 0; j < gt.deg[i]; j++){
            if (i < gt.adj[i][j]) n++;
        }
    }
    vec *pos_intersections = malloc(sizeof(vec) * n);
    bool *inited = malloc(sizeof(bool) * n);
    for (int i = 0; i < n; i++){
        inited[i] = false;
    } 

    FILE *f_python = fopen("fleche.py", "a");
    for (int i = 0; i < gt.n; i++){
        for (int j = 0; j < gt.deg[i]; j++){
            int inter_idx = gt.sommets[i].sommets[j].sommet;
            if (!inited[inter_idx]) {
                inited[inter_idx] = true;
                pos_intersections[inter_idx].x = (pos[i].x * r[gt.adj[i][j]] + pos[gt.adj[i][j]].x * r[i]) / (r[i] + r[gt.adj[i][j]]);
                pos_intersections[inter_idx].y = (pos[i].y * r[gt.adj[i][j]] + pos[gt.adj[i][j]].y * r[i]) / (r[i] + r[gt.adj[i][j]]);
                printf("je place %d avec %d et %d\n", inter_idx, i, gt.adj[i][j]);
                ecrire_point_python(f_python, pos_intersections[inter_idx].x, pos_intersections[inter_idx].y, "b", inter_idx);
            }    
        }
    }
    ecrire_footing_python(f_python);
    fclose(f_python);

    // logiquement il devrait etre possible de faire un dfs, qui dessine tout le noeud (à méditer)
    FILE *f = fopen(filename, "w");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"400\" viewBox=\"0 0 3 3\" stroke=\"blue\" stroke-width=\"0.008\" fill=\"none\">\n");
    for (int i = 0; i < gt.n; i++){
        for (int j = 0; j < gt.deg[i]; j++){
            int i0 = gt.sommets[i].sommets[j].sommet;
            int i1 = gt.sommets[i].sommets[(j+1)%gt.sommets[i].n_sommets].sommet;
            double alpha = angle(pos_intersections[i0], pos[i], pos_intersections[i1]);
            if (sin(PI/2 - alpha/2) == 0){
                printf("<path d=\"M %lf %lf L %lf %lf\"/>\n", pos_intersections[i0].x, pos_intersections[i0].y, pos_intersections[i1].x, pos_intersections[i1].y);
            } else {
                double rayon_arc = r[i] * fabs(sin(alpha/2) / sin(PI/4 - alpha/2));
                int large = 0;
                if (alpha >= 3 * PI / 2) large = 1;
                int sweep = 1;
                if (alpha <= PI / 2) sweep = 0;
                fprintf(f, "<path d=\"M %lf %lf A %lf %lf 0 %d %d %lf %lf\"/>\n", pos_intersections[i0].x, pos_intersections[i0].y, rayon_arc, rayon_arc, large, sweep, pos_intersections[i1].x, pos_intersections[i1].y);
            }
        }
    }
    fprintf(f, "</svg>\n");
    fclose(f);
}

int main(){
        int n = 6;
    voisin **adj = malloc(sizeof(voisin*) * n);
    int *deg = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++){
        adj[i] = malloc(sizeof(voisin) * 4);
    }
    
    adj[0][0].id_arete = 3;
    adj[0][0].sommet = 5;
    adj[0][1].id_arete = 2;
    adj[0][1].sommet = 2;
    adj[0][2].id_arete = 4;
    adj[0][2].sommet = 3;
    adj[0][3].id_arete = 5;
    adj[0][3].sommet = 4;
    deg[0] = 4;
    
    adj[1][0].id_arete = 11;
    adj[1][0].sommet = 2;
    adj[1][1].id_arete = 1;
    adj[1][1].sommet = 2;
    adj[1][2].id_arete = 0;
    adj[1][2].sommet = 5;
    adj[1][3].id_arete = 10;
    adj[1][3].sommet = 5;
    deg[1] = 4;
    
    adj[2][0].id_arete = 1;
    adj[2][0].sommet = 1;
    adj[2][1].id_arete = 11;
    adj[2][1].sommet = 1;
    adj[2][2].id_arete = 7;
    adj[2][2].sommet = 3;
    adj[2][3].id_arete = 2;
    adj[2][3].sommet = 0;
    deg[2] = 4;
    
    adj[3][0].id_arete = 7;
    adj[3][0].sommet = 2;
    adj[3][1].id_arete = 8;
    adj[3][1].sommet = 4;
    adj[3][2].id_arete = 6;
    adj[3][2].sommet = 4;
    adj[3][3].id_arete = 4;
    adj[3][3].sommet = 0;
    deg[3] = 4;
    
    adj[4][0].id_arete = 5;
    adj[4][0].sommet = 0;
    adj[4][1].id_arete = 6;
    adj[4][1].sommet = 3;
    adj[4][2].id_arete = 8;
    adj[4][2].sommet = 3;
    adj[4][3].id_arete = 9;
    adj[4][3].sommet = 5;
    deg[4] = 4;
    
    adj[5][0].id_arete = 0;
    adj[5][0].sommet = 1;
    adj[5][1].id_arete = 3;
    adj[5][1].sommet = 0;
    adj[5][2].id_arete = 9;
    adj[5][2].sommet = 4;
    adj[5][3].id_arete = 10;
    adj[5][3].sommet = 1;
    deg[5] = 4;

    plongement p = {n, adj, deg};

    graphe_tait gt = calculer_graphe_tait(p);
    afficher_graphe_tait(gt);
    plongement plongement_gt = gt_vers_plongement(gt);
    afficher_plongement(plongement_gt);
    graphe gt_triangule = trianguler_faces(plongement_gt);
    afficher_graphe(gt_triangule);
    plongement plongement_gt_triangule = graphe_vers_plongement(gt_triangule);
    afficher_plongement(plongement_gt_triangule);
    graphe_angles_reduit gar = calculer_graphe_angles_reduit(plongement_gt_triangule);
    double *r = calculer_radii(gar, 0.001, 1e-23);
    for (int i = 0; i < 3; i++){
        printf("%d ", gar.cycle_exterieur[i]);
    }
    printf("\n");
    afficher_graphe(gar.H);

    vec *positions = calculer_positions(r, gar);

    for (int i = 0; i < gar.H.n; i++){
        printf("%e;%e\n", positions[i].x, positions[i].y);
    }

    calculer_svg(positions, r, gt, "62.svg");

    free(positions);
    free_plongement(p);
    free_graphe_tait(gt);
    free_plongement(plongement_gt);
    free_graphe(gt_triangule);
    free_plongement(plongement_gt_triangule);
    free_graphe(gar.H);
    free(gar.cycle_exterieur);
    free(r);
}

