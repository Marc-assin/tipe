struct voisin {
    int id_arete;
    int sommet;
};

struct plongement {
    int n;
    voisin **adj; //dans la case i : les voisins du sommet i, dans leur ordre combinatoire
    int *deg; //dans la case i : les voisins du sommet i, dans leur ordre combinatoire
};

struct face {
    int n_sommets;
    voisin *sommets;
};

struct faces_arr {
    int n;
    face *faces;
};

struct graphe_tait {
    int n;
    face *sommets;
    int **adj;
    int *deg;
};

struct graphe {
    int n;
    int_vector **adj;
};

struct graphe_angles_reduit {
    graphe H;
    int *cycle_exterieur;
};

struct arete {
    int x, y;
};

struct arete_arr {
    int n;
    arete *aretes;
};

struct vec {
    double x, y;
};

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
            if (vus[sd][i]) continue;
            int taille_f = 1;
            vus[sd][i] = true;
            int s1 = sd;
            int s2 = p.adj[s1][i].sommet;
            proc_sommet[s1] = p.adj[s1][i];
            int last_j = i;
            
            while (s2 != sd){
                int j = 0;
                while (p.adj[s2][j].id_arete != p.adj[s1][last_j].id_arete){
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

/*
pour calculer le graphe de Tait :
    on calcule l'ensemble des faces
    a partir d'une face qcq, on fait un bfs, où les voisins de la face f sont les faces qui ne partagent pas d'arête avec f.
    l'ensemble des faces parcourues est l'ensemble des sommets du graphe de Tait, et les arêtes sont entre les faces qui partagent un sommet.
*/

graphe_tait calculer_graphe_tait(plongement p){
    faces_arr fa = calculer_faces(p);
    int max_n = fa.n;
    face *faces = fa.faces;
    graphe_tait gt;
    bool simple = true;

    bool *vus = malloc(sizeof(bool) * max_n);
    for (int i = 0; i < max_n; i++){
        vus[i] = false;
    }
    for (int attempts = 0; attempts < 2; attempts++){ // il y a seulement 2 graphes de Tait possibles
        int sommet_depart;
        for (int i = 0; i < max_n; i++){
            if (!vus[i]){
                sommet_depart = i;
                break;
            }
        }
        int *file = malloc(sizeof(int) * max_n);
        file[0] = sommet_depart; // a la fin du bfs, les indices des faces présentes dans file[0:r] sont les indices des faces qui sont les sommets du graphe de Tait
        vus[sommet_depart] = true;
        int r = 1;
        for (int l = 0; l < r; l++){
            // on calcule les voisins, et les ajoutes à la file si on ne les a pas encore vus
            for (int i = 0; i < max_n; i++){
                if (vus[i]) continue;
                if (test_faces_voisines(faces[file[l]], faces[i])){
                    vus[i] = true;
                    file[r] = i;
                    r++;
                }
            }
        }
        
        simple = true;
        gt.n = r;
        gt.deg = malloc(sizeof(int) * gt.n);
        gt.adj = malloc(sizeof(int*) * gt.n);
        gt.sommets = malloc(sizeof(face) * gt.n);

        
        for (int i = 0; i < r; i++){
            gt.sommets[i] = copie_face(faces[file[i]]);
            gt.deg[i] = gt.sommets[i].n_sommets;
            gt.adj[i] = malloc(sizeof(int) * gt.deg[i]);
        }

        int *temp_voisins = malloc(sizeof(int) * gt.n);
        for (int i = 0; i < r; i++){
            int nb_temp_voisins = 0;
            for (int j = 0; j < gt.n; j++){
                if (test_faces_voisines(gt.sommets[i], gt.sommets[j])){
                    temp_voisins[nb_temp_voisins] = j; // une face ne peut pas etre voisine d'elle même
                    nb_temp_voisins++;
                }
            }

            if (nb_temp_voisins < gt.deg[i]) { // deux voisins sont égaux, donc gt est un multigraphe
                simple = false;
                break;
            }
            
            for (int j = 0; j < gt.deg[i]; j++){
                for (int k = 0; k < nb_temp_voisins; k++){
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
        free(file);
        free(temp_voisins);
        if (simple) {
            break;
        } 
        free_graphe_tait(gt);
    }
    
    assert(simple); // Pas de graphe de Tait simple.
    
    for (int i = 0; i < fa.n; i++){
        free(fa.faces[i].sommets);
    }
    free(faces);
    free(vus);
    
    return gt;
}

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

plongement gt_vers_plongement(graphe_tait gt){
    plongement p;
    int n = gt.n;

    p.n = n;
    p.deg = malloc(sizeof(int) * n);
    p.adj = malloc(sizeof(voisin*) * n);

    int compteur_aretes = 0;

    for (int i = 0; i < n; i++){
        p.deg[i] = gt.deg[i];
        p.adj[i] = malloc(sizeof(voisin) * p.deg[i]);

        for (int j = 0; j < p.deg[i]; j++){
            int v = gt.adj[i][j];
            int s = gt.sommets[i].sommets[j].sommet; // à chaque sommet de G_noeud est associée une et une seule arête de gt

            voisin w;
            w.sommet = v;
            w.id_arete = s;
            p.adj[i][j] = w;
        }
    }

    return p;
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

/*
il faut ensuite trianguler le graphe de Tait T en un graphe G, puis calculer son graphe des angles réduit H' de la manière suivante : 
    on calcule un plongement combinatoire de T (ce que l'on peut faire sans algorithme compliqué, étant donné sa construction, car on a conservé l'ordre des sommets autout de chaque face, ce qui donne l'ordre des voisins de chaque face)
    on rajoute un sommet sur chaque face de T, que l'on relie à chaque sommet de T, en préservant le plongement (ce qui donne G)
    on calcule le dual de G
    on choisit une face quelconque pour etre la face exterieure
*/

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
            // on n'ajoute pas les arêtes originales de p
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

double phi_prime(int s, double *x, graphe H){
    double sum = 0.0;
    for (int i = 0; i < H.adj[s]->taille; i++){
        int j = get_ivec(H.adj[s], i);
        if (j == -1){ continue; } // on ne prend pas la face exterieure en compte
        sum += atan(exp(x[s]-x[j])) - atan(exp(x[j]-x[s])) - PI / 2.0;
    }
    sum += PI * 2.0;
    return sum;
}

double* calculer_rayons(graphe_angles_reduit gar, double coeff_apprentissage, double epsilon){
    graphe H = gar.H;
    int n = H.n;
    int s1 = gar.cycle_exterieur[0], s2 = gar.cycle_exterieur[1], s3 = gar.cycle_exterieur[2];
    double *x = malloc(sizeof(double) * n);
    double *dphidx = malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++){
        x[i] = log(tan(PI/3))/2;
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
    double MSE = 0.0, last_MSE;
    for (int i = 0; i < n; i++){
        MSE += dphidx[i]*dphidx[i];
    }
    
    while (MSE > epsilon){
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
        last_MSE = MSE;
        MSE = 0.0;
        for (int i = 0; i < n; i++){
            MSE += dphidx[i]*dphidx[i];
        }
    }
    double *r = malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++){
        r[i] = exp(x[i]);
    }
    free(x);
    free(dphidx);
    return r;
}

vec* calculer_positions(double *r, graphe_angles_reduit gar){
    int n = gar.H.n;
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
                t1 = i;
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
        int j = 0; 
        int deg = gar.H.adj[s]->taille;
        while (get_ivec(gar.H.adj[s], j) == -1 || places[get_ivec(gar.H.adj[s], j)] == false){ // on saute la face ext
            j = (j+1)%deg;
        }
        bool sens_inverse = false;
        for (int d = 0; d < deg - 1; d++){
            int voisin = get_ivec(gar.H.adj[s], j);
            j = (j+1)%deg;
            int w = get_ivec(gar.H.adj[s], j); 
            if (w == -1){
                sens_inverse = true;
                j = (deg + j - 1)%deg;
                break;
            }
            if (places[w]) {
                continue;
            }
            
            // 's' and 'w' sont voisins dans H, donc leurs cercles sont orthogonaux
            double r1 = sqrt(r[s]*r[s] + r[w]*r[w]); 
            
            // 'voisin' et 'w' sont voisins de 's', donc leurs cercles sont tangents
            double r2 = r[voisin] + r[w];  

            double x1 = positions[s].x;
            double y1 = positions[s].y;
            double x2 = positions[voisin].x;
            double y2 = positions[voisin].y;

            double dx = x2 - x1;
            double dy = y2 - y1;

            double rDiffCar = r1*r1 - r2*r2;
            double xDiffCar = x2*x2 - x1*x1;
            double yDiffCar = y2*y2 - y1*y1;
            double denom = 2.0 * (dx*dx + dy*dy);
            double crois = y1*x2 - y2*x1;

            vec barycentre;
            barycentre.x = (dx * (rDiffCar + xDiffCar + yDiffCar) - 2.0 * dy * crois) / denom;
            barycentre.y = (dy * (rDiffCar + xDiffCar + yDiffCar) + 2.0 * dx * crois) / denom;
            
            vec perp; 
            perp.x = positions[s].y - positions[voisin].y;
            perp.y = positions[voisin].x - positions[s].x;
            perp = normaliser(perp);
            
            double di = dist(positions[s], barycentre);
            double scale = r1*r1 - di*di; 
            double n_len = sqrt(scale); 
            perp.x *= n_len;
            perp.y *= n_len;
            
            positions[w] = add(barycentre, perp);
            places[w] = true;
            file[pointeur_droite] = w;
            pointeur_droite++;
        }
        if (sens_inverse){ // on est tombés sur la face exterieure, on ne peut pas la placer, donc on repart dans l'autre sens pour placer le reste des voisins.
        for (int d = 0; d < deg - 2; d++){
            int voisin = get_ivec(gar.H.adj[s], j);
            j = (deg + j - 1)%deg;
            if (places[get_ivec(gar.H.adj[s], j)]) {
                continue;
            }
            int w = get_ivec(gar.H.adj[s], j); 

            double r1 = sqrt(r[s]*r[s] + r[w]*r[w]); 
            
            double r2 = r[voisin] + r[w];  

            double x1 = positions[s].x;
            double y1 = positions[s].y;
            double x2 = positions[voisin].x;
            double y2 = positions[voisin].y;

            double dx = x2 - x1;
            double dy = y2 - y1;

            double rDiffCar = r1*r1 - r2*r2;
            double xDiffCar = x2*x2 - x1*x1;
            double yDiffCar = y2*y2 - y1*y1;
            double denom = 2.0 * (dx*dx + dy*dy);
            double crois = y1*x2 - y2*x1;

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

    free(file);
    free(places);
    return positions;
}

void dessiner_inter(vec pos, double dir, double stroke_width, FILE *svg){
    double epsilon = stroke_width;
    vec e_eps = {epsilon * cos(dir), epsilon * sin(dir)};
    fprintf(svg, "<path d=\"M %lf %lf L %lf %lf\" stroke-width=\"%lf\" stroke=\"white\"/>\n", pos.x - e_eps.x, pos.y - e_eps.y, pos.x + e_eps.x, pos.y + e_eps.y, stroke_width * 3);
    fprintf(svg, "<path d=\"M %lf %lf L %lf %lf\" stroke-width=\"%lf\" stroke=\"blue\"/>\n", pos.x - e_eps.x, pos.y - e_eps.y, pos.x + e_eps.x, pos.y + e_eps.y, stroke_width);
}

void calculer_svg(vec *pos, double *r, graphe_tait gt, plongement p, bool start_over, char* filename){
    FILE *f = fopen(filename, "w");
    double stroke_width = 0.008;
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"400\" viewBox=\"0 0 3 3\" stroke=\"blue\" stroke-width=\"%lf\" fill=\"none\">\n", stroke_width);
    int n = 0;
    for (int i = 0; i < gt.n; i++){
        for (int j = 0; j < gt.deg[i]; j++){
            if (i < gt.adj[i][j]) n++;
        }
    }
    vec *pos_intersections = malloc(sizeof(vec) * n);
    double **ddir = malloc(sizeof(double*) * gt.n);
    bool *inited = malloc(sizeof(bool) * n);
    for (int i = 0; i < n; i++){
        inited[i] = false;
    } 
    for (int i = 0; i < gt.n; i++){
        ddir[i] = malloc(sizeof(double) * gt.deg[i]);
    }

    for (int i = 0; i < gt.n; i++){
        for (int j = 0; j < gt.deg[i]; j++){
            int inter_idx = gt.sommets[i].sommets[j].sommet;
            if (!inited[inter_idx]) {
                inited[inter_idx] = true;
                pos_intersections[inter_idx].x = (pos[i].x * r[gt.adj[i][j]] + pos[gt.adj[i][j]].x * r[i]) / (r[i] + r[gt.adj[i][j]]);
                pos_intersections[inter_idx].y = (pos[i].y * r[gt.adj[i][j]] + pos[gt.adj[i][j]].y * r[i]) / (r[i] + r[gt.adj[i][j]]);
            }    
        }
    }

    // il devrait etre possible de faire un dfs, qui dessine tout le noeud
    for (int i = 0; i < gt.n; i++){
        for (int j = 0; j < gt.deg[i]; j++){
            int i0 = gt.sommets[i].sommets[j].sommet;
            int i1 = gt.sommets[i].sommets[(j+1)%gt.sommets[i].n_sommets].sommet;
            double alpha = angle(pos_intersections[i0], pos[i], pos_intersections[i1]);
            ddir[i][(j+1)%gt.sommets[i].n_sommets] = alpha; // on utilise que le noeud est alterné ; alpha vaut déjà déviation + 90° (alternance) 
            if (sin(PI/2 - alpha/2) == 0){
                fprintf(f, "<path d=\"M %lf %lf L %lf %lf\"/>\n", pos_intersections[i0].x, pos_intersections[i0].y, pos_intersections[i1].x, pos_intersections[i1].y);
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

    bool* vus = malloc(sizeof(bool) * p.n);
    for (int i = 0; i < p.n; i++){
        vus[i] = false;
    }
    int n_vus = 0;
    int s0, s1 = 0, fa = 0; 
    vec centre_plus_e_x = {pos[0].x+1, pos[0].y};

    double dir = PI/2 - angle(pos_intersections[0], pos[0], centre_plus_e_x) - PI/4 + (start_over?PI/2:0);
    dessiner_inter(pos_intersections[0], dir, stroke_width, f);
    vus[0] = true;
    n_vus++;
    int k = 0;
    bool sens_inverse = false;
    while (n_vus < p.n && k < 1000) {
        s0 = s1;
        for (int i = 0; i < gt.deg[fa]; i++){
            if (gt.sommets[fa].sommets[i].sommet == s0){
                int j; double dev;
                if (sens_inverse) {
                    j = (i+gt.deg[fa]-1)%gt.deg[fa];
                    dev = -ddir[fa][i];
                } else {
                    j = (i+1)%gt.deg[fa];
                    dev = ddir[fa][j];
                }
                s1 = gt.sommets[fa].sommets[j].sommet;
                dir += dev;
                if (!vus[s1]){
                    dessiner_inter(pos_intersections[s1], dir, stroke_width, f);
                    vus[s1] = true;
                    n_vus++;
                }
                fa = gt.adj[fa][j];
                sens_inverse = !sens_inverse;
                break;
            }
        }
        k++;
    }

    fprintf(f, "</svg>\n");
    fclose(f);
    for (int i = 0; i < gt.n; i++){
        free(ddir[i]);
    }
    free(ddir);
    free(vus);
    free(pos_intersections);
    free(inited);
}