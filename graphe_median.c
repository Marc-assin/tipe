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
    int *deg = malloc(sizeof(int*) * n);
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
        gt.adj[i] = malloc(sizeof(face) * gt.deg[i]);
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
    int *deg = malloc(sizeof(int*) * n);
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
    // int c = p.n;
    // for (int i = 0; i < faces.n; i++){
    //     if (faces.faces[i].n_sommets > 3){
    //         c++;
    //     }
    // }
    graphe g;
    g.n = p.n+faces.n;
    g.adj = malloc(sizeof(int_vector) * g.n);
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
        p.adj[i] = malloc(sizeof(voisin*) * p.deg[i]);
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
        free_ivec(g.adj[i]);
    }
    free(indice_arete);
    free(g.adj);
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

/*
il faut ensuite trianguler le graphe de Tait T en un graphe G, puis calculer son graphe des angles réduit H' de la manière suivante : 
    on calcule un plongement combinatoire de T (ce que l'on peut faire sans algorithme compliqué, étant donné sa construction, car on a conservé l'ordre des sommets autout de chaque face, ce qui donne l'ordre des voisins de chaque face)
    on rajoute un sommet sur chaque face de T, que l'on relie à chaque sommet de T, en préservant le plongement (ce qui donne G)
    on calcule le dual de G
    on choisit une face quelconque pour etre la face exterieure
*/

