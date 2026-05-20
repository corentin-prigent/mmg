#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define SPHERE_CX 0.5
#define SPHERE_CY 0.5
#define SPHERE_CZ 0.5
#define SPHERE_R  0.25

double determinant(double p1[3], double p2[3], double p3[3]) {
    return p1[0] * (p2[1]*p3[2] - p2[2]*p3[1]) - p1[1] * (p2[0]*p3[2] - p2[2]*p3[0])
         + p1[2] * (p2[0]*p3[1] - p2[1]*p3[0]);
}

double dot(double a[3], double b[3]) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void cross(double a[3], double b[3], double res[3]) {
    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
}

double norme(double a[3]) {
    return sqrt(dot(a, a));
}

double mon_atan2(double y, double x) {
    if (x > 0) return atan(y / x);
    else if (x < 0 && y >= 0) return atan(y / x) + M_PI;
    else if (x < 0 && y < 0)  return atan(y / x) - M_PI;
    else if (x == 0 && y > 0) return M_PI / 2.0;
    else if (x == 0 && y < 0) return -M_PI / 2.0;
    else return 0.0;
}

double Omega(double p1[3], double p2[3], double p3[3], double R) {
    double num = fabs(determinant(p1, p2, p3));
    double den = R*R*R + R * (dot(p1,p2) + dot(p2,p3) + dot(p3,p1));
    return 2.0 * mon_atan2(num, den);
}

double volume_tetraedre(double p1[3], double p2[3], double p3[3], double p4[3]) {
    double e12[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
    double e13[3] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};
    double e14[3] = {p4[0]-p1[0], p4[1]-p1[1], p4[2]-p1[2]};
    return fabs(determinant(e12, e13, e14)) / 6.0;
}

double volume_fraction_analytique(double p1[3], double p2[3], double p3[3], double centre[3], double R) {
    double q1[3] = {p1[0]-centre[0], p1[1]-centre[1], p1[2]-centre[2]};
    double q2[3] = {p2[0]-centre[0], p2[1]-centre[1], p2[2]-centre[2]};
    double q3[3] = {p3[0]-centre[0], p3[1]-centre[1], p3[2]-centre[2]};
    double O[3]  = {0.0, 0.0, 0.0};

    double Vs = Omega(q1, q2, q3, R) * R*R*R / 3.0;
    double Vt = volume_tetraedre(q1, q2, q3, O);
    return fabs(Vs - Vt);
}

double f_reelle(double P[3], double nT[3]) {
    double cx = P[0]-SPHERE_CX, cy = P[1]-SPHERE_CY, cz = P[2]-SPHERE_CZ;
    double r   = sqrt(cx*cx + cy*cy + cz*cz);
    double cos_theta = (cx*nT[0] + cy*nT[1] + cz*nT[2]) / r; // P̂·n̂_T
    return fabs((SPHERE_R*SPHERE_R*SPHERE_R - r*r*r) / (3.0*r*r) * cos_theta);
}


double f_reelle1(double P[3], double nT[3]) {
    double num= sqrt( (P[0] - SPHERE_CX)*(P[0] - SPHERE_CX) + 
                    (P[1] - SPHERE_CY)*(P[1] - SPHERE_CY)+
                    (P[2] - SPHERE_CZ)*(P[2] - SPHERE_CZ)
                 );
    return fabs(num - SPHERE_R );
}
double aire_triangle(double p1[3], double p2[3], double p3[3]) {
    double e12[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
    double e13[3] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};
    double n[3];
    cross(e12, e13, n);
    return norme(n) / 2.0;
}

double volume_fraction_gauss(double p1[3], double p2[3], double p3[3], double *u, double *v, double *w, int ng) {
    double e12[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
    double e13[3] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};

    double A = aire_triangle(p1,p2,p3);
    double m[3]; cross(e12, e13, m);
    double nm = norme(m);
    double nT[3] = {m[0]/nm, m[1]/nm, m[2]/nm};
    double somme = 0.0;
    for ( int k = 0; k < ng; k++ ) {
        // point de Gauss sur le triangle
        double P[3] = {
            p1[0] + u[k]*e12[0] + v[k]*e13[0],
            p1[1] + u[k]*e12[1] + v[k]*e13[1],
            p1[2] + u[k]*e12[2] + v[k]*e13[2]
        };
        somme += w[k] * f_reelle(P,nT); 
    }

    return 2.0 * A * somme;
}

typedef struct { double h; double erreur; } ResMesh;

ResMesh lire_mesh(const char *filename, int ref_cible, double centre[3], double R,
               double *u, double *v, double *w, int ng) {

    FILE *f = fopen(filename, "r");
    int    nv       = 0;
    double (*vertices)[3] = NULL;

    double h_moy      = 0.0;
    double erreur_abs = 0.0;

    char token[20];
    while ( fscanf(f, "%19s", token) == 1 ) {

        if ( strcmp(token, "Vertices") == 0 ) {
            fscanf(f, "%d", &nv);
            vertices = malloc(nv * sizeof(*vertices));
            for ( int i = 0; i < nv; i++ ) {
                int ref_v;
                fscanf(f, "%lf %lf %lf %d", &vertices[i][0], &vertices[i][1],
                       &vertices[i][2], &ref_v);
            }
        }

        if ( strcmp(token, "Triangles") == 0 ) {
            int nt;
            fscanf(f, "%d", &nt);

            int    n_filtre   = 0;
            double vol_ana    = 0.0;   
            double vol_gauss  = 0.0;   

            double vf_ana    = 0.0;
            double vf_gauss  = 0.0;
            double aire_tot   = 0.0;  
            
            for ( int i = 0; i < nt; i++ ) {
                int v1, v2, v3, ref;
                fscanf(f, "%d %d %d %d", &v1, &v2, &v3, &ref);

                if ( ref != ref_cible && ref != 8 ) continue;

                double *p1 = vertices[v1-1];
                double *p2 = vertices[v2-1];
                double *p3 = vertices[v3-1];

                // solution analytique
                vol_ana   += volume_fraction_analytique(p1, p2, p3, centre, R);
                vf_ana        += vol_ana;
                // approximation de Gauss
                vol_gauss += volume_fraction_gauss(p1, p2, p3, u, v, w, ng);
                vf_gauss      += vol_gauss;

                double aire_loc  = aire_triangle(p1, p2, p3);
                double h_loc     = sqrt(aire_loc);
                double err_loc   = fabs(vf_gauss - vf_ana);
            
                aire_tot        += aire_loc; 

                n_filtre++;
            }

            h_moy = sqrt(aire_tot / n_filtre);
            erreur_abs = fabs(vol_gauss - vol_ana);
            double erreur_rel = erreur_abs / fabs(vol_ana) * 100.0;

            printf("\nCOMPARAISON ANALYTIQUE vs GAUSS\n");
            printf("\nFichier              : %s\n",   filename);
            printf("Triangles ref=%d et 8  : %d\n",   ref_cible, n_filtre);
            printf("Points de Gauss      : %d\n",   ng);
            printf("\nDistance exacte maillage-sphere= %.16f\n", vol_ana);
            printf("Distance approchée maillage-sphere  = %.16f\n", vol_gauss);
            printf("h_moy = sqrt(A/n)    = %.8f\n", h_moy);
            printf("Erreur absolue       = %.6e\n",  erreur_abs);
            printf("Erreur relative      = %.6f %%\n", erreur_rel);
            printf("\n");
            return (ResMesh){h_moy, erreur_abs};
        }
    }

    free(vertices);
    fclose(f);
    // (ResMesh){h_moy, erreur_abs};
}

// intégrale analytique de f=1 => A
double test_f1(double p1[3], double p2[3], double p3[3]) {
    return aire_triangle(p1, p2, p3);
}

// intégrale analytique de f=x => A * (x1+x2+x3)/3
double test_fx(double p1[3], double p2[3], double p3[3]) {
    return aire_triangle(p1,p2,p3) * (p1[0]+p2[0]+p3[0]) / 3.0;
}

// intégrale analytique de f=x² => A*(x1²+x2²+x3²+x1x2+x1x3+x2x3)/6 
double test_fx2(double p1[3], double p2[3], double p3[3]) {
    double A = aire_triangle(p1,p2,p3);
    double x1=p1[0], x2=p2[0], x3=p3[0];
    return A * (x1*x1 + x2*x2 + x3*x3 + x1*x2 + x1*x3 + x2*x3) / 6.0;
}

// intégrale analytique de f=x^3
double test_fx3(double p1[3], double p2[3], double p3[3]) {
    double A = aire_triangle(p1, p2, p3);
    double x1 = p1[0], x2 = p2[0], x3 = p3[0];
    
    double x1_2 = x1 * x1;
    double x2_2 = x2 * x2;
    double x3_2 = x3 * x3;
    
    double somme = (x1_2*x1 + x2_2*x2 + x3_2*x3) 
                 + (x1_2*x2 + x1*x2_2) 
                 + (x1_2*x3 + x1*x3_2) 
                 + (x2_2*x3 + x2*x3_2) 
                 + (x1*x2*x3);
                 
    return A * somme / 10.0; // Correction: 10.0 au lieu de 20.0
}

// quadrature de Gauss 
double gauss(double p1[3], double p2[3], double p3[3],
             double *u, double *v, double *w, int ng,
             double (*f)(double[3])) {
    double e12[3]={p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]};
    double e13[3]={p3[0]-p1[0],p3[1]-p1[1],p3[2]-p1[2]};
    double A = aire_triangle(p1,p2,p3);
    double somme = 0.0;
    for ( int k = 0; k < ng; k++ ) {
        double P[3] = {
            p1[0] + u[k]*e12[0] + v[k]*e13[0],
            p1[1] + u[k]*e12[1] + v[k]*e13[1],
            p1[2] + u[k]*e12[2] + v[k]*e13[2]
        };
        somme += w[k] * f(P);
    }
    return 2.0 * A * somme;
}

// fonctions f a passer en paramètre
double f_1  (double P[3]) { return 1.0; }
double f_x   (double P[3]) { return P[0]; }
double f_x2  (double P[3]) { return P[0]*P[0]; }
double f_xy  (double P[3]) { return P[0]*P[1]; }
double f_x3  (double P[3]) { return P[0]*P[0]*P[0]; }
double f_xyz (double P[3]) { return P[0]*P[2]*P[1]; }

void test_quadrature(double *u, double *v, double *w, int ng) {
    // triangle test 
    double p1[3]={0.1, 0.2, 0.0};
    double p2[3]={0.7, 0.3, 0.0};
    double p3[3]={0.5, 0.8, 0.0};
    const char *statut;


    printf(" Test de quadrature d'ordre %d\n", ng);
    printf("\n%-16s %-18s  %-10s  %-7s\n",
           "f", "analytique", "gauss", "erreur rel");

    struct { 
        const char *nom; 
        double (*f)(double[3]); 
        double (*ana)(double[3],double[3],double[3]); 
        } 
        tests[] = {
        {"1", f_1,  test_f1  },
        {"x", f_x,   test_fx  },
        {"x*x", f_x2,  test_fx2 },
        {"x*x*x", f_x3, test_fx3 },

    };

    for ( int t = 0; t < 4; t++ ) {
        double ana  = tests[t].ana(p1, p2, p3);
        double gss  = gauss(p1, p2, p3, u, v, w, ng, tests[t].f);
        double err  = fabs(gss-ana);

        if (err < 1e-10) {
            statut = " OK";
        } else {
            statut = " ERREUR";
        }
        printf("%-10s  %16.10f  %16.10f  %.2e%s\n",
               tests[t].nom, ana, gss, err,
               statut);
    }
}

double f_x4(double P[3]) {
    return sin( P[0] + P[1] );
}

void test_ordre_erreur() {
    int ng = 4;
    double u[4] = {1.0/3, 3.0/5, 1.0/5, 1.0/5};
    double v[4] = {1.0/3, 1.0/5, 3.0/5, 1.0/5};
    double w[4] = {-27.0/96, 25.0/96, 25.0/96, 25.0/96};

    // Intégrale exacte de f(x,y) = x^4 sur le carré [0,1]x[0,1] = 1/5 = 0.2
    double analytique = 2*sin(1) - sin(2); 
    //test avec f_x3
    //double analytique = 1/4; 
    
    printf("\n======================================================================\n");
    printf("VALIDATION DE L'ORDRE DE L'ERREUR (f = x^4 sur domaine plat)\n");
    printf("======================================================================\n");
    printf("%-8s %-12s %-15s %-10s\n", "N div", "h (1/N)", "Erreur abs", "Ordre (p)");
    printf("----------------------------------------------------------------------\n");

    double err_prev = 0.0;
    double h_prev = 0.0;
    int N_vals[] = {1, 2, 4, 8, 16, 32, 64}; // Division du carré N x N
    
    for (int i = 0; i < 7; i++) {
        int N = N_vals[i];
        double h = 1.0 / N;
        double integrale_num = 0.0;

        for (int ix = 0; ix < N; ix++) {
            for (int iy = 0; iy < N; iy++) {
                double x0 = ix * h;
                double y0 = iy * h;
                
                // Découpage du sous-carré en 2 triangles plans
                double p1[3] = {x0, y0, 0.0};
                double p2[3] = {x0+h, y0, 0.0};
                double p3[3] = {x0, y0+h, 0.0};
                double p4[3] = {x0+h, y0+h, 0.0};
                
                integrale_num += gauss(p1, p2, p3, u, v, w, ng, f_x4);
                integrale_num += gauss(p4, p3, p2, u, v, w, ng, f_x4); 
            }
        }

        double err = fabs(integrale_num - analytique);
        double ordre = 0.0;
        
        if (i > 0 && err > 0 && err_prev > 0) {
            ordre = log(err_prev / err) / log(h_prev / h);
        }

        if (i == 0) {
            printf("%-8d %-12.5f %-15.6e %-10s\n", N, h, err, "-");
        } else {
            printf("%-8d %-12.5f %-15.6e %-10.4f\n", N, h, err, ordre);
        }

        err_prev = err;
        h_prev = h;
    }
    printf("======================================================================\n\n");
}

int main(int argc, char *argv[]) {

    if ( argc < 2 ) {
        fprintf(stderr, "Usage: %s <fichier.mesh>\n", argv[0]);
        return 1;
    }

    const char  *fichier  = argv[1];
    const int    ref_cible = 7;
    const double R         = SPHERE_R;
    double centre[3]       = {SPHERE_CX, SPHERE_CY, SPHERE_CZ};

    // int    ng   = 4;
    // double u[4] = {1.0/3, 3.0/5, 1.0/5, 1.0/5};
    // double v[4] = {1.0/3, 1.0/5, 3.0/5, 1.0/5};
    // double w[4] = {-27.0/96, 25.0/96, 25.0/96, 25.0/96};

    // int    ng   = 3;
    // double u[3] = {1.0/6, 2.0/3, 1.0/6};
    // double v[3] = {1.0/6, 1.0/6, 2.0/3};
    // double w[3] = {1.0/6, 1.0/6, 1.0/6};

    //     int    ng   = 7;
    // double u[7] = {1.0/3, 0.797427, 0.101287, 0.101287, 0.470142, 0.059716, 0.470142};
    // double v[7] = {1.0/3, 0.101287, 0.797427, 0.101287, 0.059716, 0.470142, 0.470142};
    // double w[7] = {0.225/2, 0.12594/2, 0.12594/2, 0.12594/2,
    //                0.13239/2, 0.13239/2, 0.13239/2};

    // Quadrature de Gauss à 12 points (Degré d'exactitude : 6)
    int ng = 12;
    double u[12] = {
        0.873821971016996, 0.063089014491502, 0.063089014491502,
        0.501426509658179, 0.249286745170910, 0.249286745170910,
        0.636502499121399, 0.310352451033785, 0.053145049844816,
        0.636502499121399, 0.053145049844816, 0.310352451033785
    };

    double v[12] = {
        0.063089014491502, 0.873821971016996, 0.063089014491502,
        0.249286745170910, 0.501426509658179, 0.249286745170910,
        0.310352451033785, 0.053145049844816, 0.636502499121399,
        0.053145049844816, 0.310352451033785, 0.636502499121399
    };

    double w[12] = {
        0.050844906370207 / 2.0, 0.050844906370207 / 2.0, 0.050844906370207 / 2.0,
        0.116786275726379 / 2.0, 0.116786275726379 / 2.0, 0.116786275726379 / 2.0,
        0.082851075618374 / 2.0, 0.082851075618374 / 2.0, 0.082851075618374 / 2.0,
        0.082851075618374 / 2.0, 0.082851075618374 / 2.0, 0.082851075618374 / 2.0
    };

    //test_quadrature(u, v, w, ng);
    //test_ordre_erreur();
    //lire_mesh(fichier, ref_cible, centre, R, u, v, w, ng);

    double h_prev = 0.0;
    double err_prev = 0.0;
    double h_array[argc];
    double err_array[argc];
    int count = 0; // Compteur de maillages valides

    for (int i = 1; i < argc; i++) {
        if(i == 1){
            printf("\n======================================================================\n");
            printf("ANALYSE GLOBALE ET CALCUL DE L'ORDRE\n");
            printf("======================================================================\n");
        }
        const char *fichier = argv[i];
        
        // Appel de la fonction et récupération des métriques
        ResMesh res = lire_mesh(fichier, ref_cible, centre, R, u, v, w, ng);
        
        if (res.h > 0.0) { // S'assure que la lecture a réussi
            // Stockage pour l'export final
            h_array[count] = res.h;
            err_array[count] = res.erreur;
            
            // Calcul de l'ordre à partir de la 2ème itération valide
            if (count > 0 && err_array[count] > 0 && err_array[count-1] > 0) {
                double ordre = log(err_array[count-1] / err_array[count]) / log(h_array[count-1] / h_array[count]);
                printf(">>> ORDRE OBSERVE (p) = %.4f <<<\n", ordre);
                printf("----------------------------------------------------------------------\n");
            }
            
            count++; // Incrémentation des maillages traités
        }
    }

    // Affichage formaté pour Python à la fin de l'exécution
    if (count > 0) {
        
        // Impression du tableau h_moy
        printf("h_moy = np.array([");
        for (int j = 0; j < count; j++) {
            printf("%.8f", h_array[j]);
            if (j < count - 1) printf(", "); // Ajoute la virgule sauf pour le dernier élément
        }
        printf("])\n");

        // Impression du tableau erreur en format scientifique
        printf("erreur = np.array([");
        for (int j = 0; j < count; j++) {
            printf("%e", err_array[j]);
            if (j < count - 1) printf(", ");
        }
        printf("])\n\n");
        printf("ng = %d\n", ng);
    }

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////
//////// Grossier 

/*
gcc /home/eelleb/Desktop/mmg/src/mmg3d/gaussv1.1.c -o compare -lm && ./compare \
/home/eelleb/Desktop/GMSH/cube4.mesh \
/home/eelleb/Desktop/GMSH/cube5.mesh \
/home/eelleb/Desktop/test_egads/cube_sphere_gmsh_0.1.mesh
*/

/*
gcc /home/eelleb/Desktop/mmg/src/mmg3d/gaussv1.1.c -o compare -lm && ./compare \
/home/eelleb/Desktop/test_egads/piece1.mesh \
/home/eelleb/Desktop/test_egads/piece2.mesh \
/home/eelleb/Desktop/test_egads/piece3.mesh \
/home/eelleb/Desktop/test_egads/piece4.mesh \
/home/eelleb/Desktop/test_egads/piece5.mesh \
/home/eelleb/Desktop/test_egads/piece6.mesh \
/home/eelleb/Desktop/test_egads/piece7.mesh \
/home/eelleb/Desktop/test_egads/piece8.mesh \
/home/eelleb/Desktop/test_egads/piece9.mesh \
/home/eelleb/Desktop/test_egads/piece10.mesh \
/home/eelleb/Desktop/test_egads/piece11.mesh

*/
////////////////////////////////////////////////////////////////////////////////
/*
gcc /home/eelleb/Desktop/mmg/src/mmg3d/gaussv1.1.c -o compare -lm      && for mesh in 0.1 0.05 0.02 0.01; do \
        ./compare /home/eelleb/Desktop/GMSH/cube_creux_${mesh}_v1.mesh; \
    done
*/

/*
gcc /home/eelleb/Desktop/mmg/src/mmg3d/gaussv1.1.c -o compare -lm && for mesh in 0.1 0.05 0.02 0.01; \
do ./compare /home/eelleb/Desktop/test_egads/cube_sphere_gmsh_${mesh}.mesh; done
*/

///////////////////////////////////////////////////////////////////////////////
// Pour l'ordre d'erreur


/*

gcc /home/eelleb/Desktop/mmg/src/mmg3d/gaussv1.1.c -o compare -lm && ./compare \
 /home/eelleb/Desktop/test_egads/cube_sphere_gmsh_0.1.mesh \
 /home/eelleb/Desktop/test_egads/cube_sphere_gmsh_0.05.mesh \
 /home/eelleb/Desktop/test_egads/cube_sphere_gmsh_0.02.mesh \
 /home/eelleb/Desktop/test_egads/cube_sphere_gmsh_0.01.mesh

*/