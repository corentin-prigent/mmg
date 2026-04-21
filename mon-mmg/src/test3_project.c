/*
 * test3_project.c
 * Objectif : simuler exactement ce que MMG ferait.
 *   1. Prendre un nœud "approximatif" (légèrement en dehors de la surface)
 *   2. Appeler EG_invEvaluate pour le projeter sur la vraie surface
 *   3. Afficher l'écart avant/après projection
 *
 * C'est le coeur de l'intégration MMG + EGADSlite.
 *
 * Compile :
 *   gcc test3_project.c -o test3_project \
 *       -I/home/eelleb/ESP/EngSketchPad/include \
 *       -L/home/eelleb/ESP/EngSketchPad/lib \
 *       -legads -lm
 *
 * Exécute :
 *   ./test3_project /home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "egads.h"

double dist3(const double *a, const double *b) {
    double dx=a[0]-b[0], dy=a[1]-b[1], dz=a[2]-b[2];
    return sqrt(dx*dx + dy*dy + dz*dz);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage : %s <fichier.egads>\n", argv[0]);
        return 1;
    }

    ego context, model;
    int status;

    status = EG_open(&context);
    if (status != EGADS_SUCCESS) { printf("EG_open : %d\n", status); return 1; }

    status = EG_loadModel(context, 0, argv[1], &model);
    if (status != EGADS_SUCCESS) {
        printf("EG_loadModel : %d\n", status);
        EG_close(context); return 1;
    }

    /* Récupérer le premier Body et ses Faces */
    ego geom;
    int oclass, mtype, nbody, *senses;
    ego *bodies;
    double limits[4];
    EG_getTopology(model, &geom, &oclass, &mtype, limits, &nbody, &bodies, &senses);

    int nface;
    ego *faces;
    status = EG_getBodyTopos(bodies[0], NULL, FACE, &nface, &faces);
    if (status != EGADS_SUCCESS || nface == 0) {
        printf("Erreur getBodyTopos : %d\n", status);
        EG_close(context); return 1;
    }
    printf("Modele charge : %d face(s).\n\n", nface);

    /*
     * Pour chaque face (jusqu'à 5), on fait le test suivant :
     *
     *   a) Evaluer le point exact au centre paramétrique -> P_exact
     *   b) Perturber P_exact d'un petit vecteur -> P_approx  (comme MMG)
     *   c) Projeter P_approx via EG_invEvaluate -> P_proj
     *   d) Comparer dist(P_approx, surface) et dist(P_proj, surface)
     */

    int nlimit = nface < 5 ? nface : 5;

    for (int iface = 0; iface < nlimit; iface++) {
        ego face = faces[iface];
        ego surf;
        int fc, fm, nloop, *fs;
        ego *loops;
        double uv_range[4];

        status = EG_getTopology(face, &surf, &fc, &fm,
                                uv_range, &nloop, &loops, &fs);
        if (status != EGADS_SUCCESS) { EG_free(loops); continue; }

        double uc = (uv_range[0] + uv_range[1]) / 2.0;
        double vc = (uv_range[2] + uv_range[3]) / 2.0;
        double uv_center[2] = {uc, vc};

        /* a) Point exact sur la surface */
        double res_exact[18];
        status = EG_evaluate(face, uv_center, res_exact);
        if (status != EGADS_SUCCESS) {
            printf("Face %d : EG_evaluate erreur %d\n", iface, status);
            EG_free(loops); continue;
        }
        double P_exact[3] = {res_exact[0], res_exact[1], res_exact[2]};

        /* Normale au point exact (dX/du x dX/dv) */
        double nx = res_exact[4]*res_exact[8] - res_exact[5]*res_exact[7];
        double ny = res_exact[5]*res_exact[6] - res_exact[3]*res_exact[8];
        double nz = res_exact[3]*res_exact[7] - res_exact[4]*res_exact[6];
        double nn = sqrt(nx*nx+ny*ny+nz*nz);
        if (nn < 1e-14) { EG_free(loops); continue; }
        nx/=nn; ny/=nn; nz/=nn;

        /* b) Perturber dans la direction de la normale (simule MMG) */
        double perturbation = 0.05;
        double P_approx[3] = {
            P_exact[0] + perturbation * nx,
            P_exact[1] + perturbation * ny,
            P_exact[2] + perturbation * nz
        };

        /* c) Projeter P_approx sur la surface via EG_invEvaluate */
        double uv_proj[2];
        double P_proj[3];

        status = EG_invEvaluate(face, P_approx, uv_proj, P_proj);

        printf("=== FACE %d ===\n", iface);
        printf("  P_exact  : (%.5f, %.5f, %.5f)\n",
               P_exact[0], P_exact[1], P_exact[2]);
        printf("  P_approx : (%.5f, %.5f, %.5f)  [+%.3f selon normale]\n",
               P_approx[0], P_approx[1], P_approx[2], perturbation);

        if (status != EGADS_SUCCESS) {
            printf("  EG_invEvaluate : ECHEC (code %d)\n\n", status);
            /*
             * Code -15 = EGADS_OUTSIDE : le point est hors des bornes
             * paramétriques de la face. Essayer avec EG_invEvaluateGuess
             * en fournissant un point de départ proche.
             */
            double uv_guess[2] = {uc, vc};
            status = EG_invEvaluateGuess(face, P_approx, uv_guess, P_proj);
            if (status == EGADS_SUCCESS) {
                printf("  EG_invEvaluateGuess : OK\n");
                printf("  P_proj   : (%.5f, %.5f, %.5f)\n",
                       P_proj[0], P_proj[1], P_proj[2]);
                printf("  Ecart avant proj : %.6f\n", dist3(P_approx, P_exact));
                printf("  Ecart apres proj : %.6f\n\n", dist3(P_proj, P_exact));
            } else {
                printf("  EG_invEvaluateGuess : aussi en echec (%d)\n\n", status);
            }
        } else {
            printf("  P_proj   : (%.5f, %.5f, %.5f)\n",
                   P_proj[0], P_proj[1], P_proj[2]);
            printf("  (u*,v*)  : (%.5f, %.5f)\n", uv_proj[0], uv_proj[1]);
            printf("  Ecart avant projection : %.6f\n", dist3(P_approx, P_exact));
            printf("  Ecart apres  projection : %.6f  (doit etre ~0)\n\n",
                   dist3(P_proj, P_exact));
        }

        EG_free(loops);
    }

    EG_free(faces);
    EG_free(bodies);
    EG_close(context);
    return 0;
}
