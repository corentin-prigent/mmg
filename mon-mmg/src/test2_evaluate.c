/*
 * test2_evaluate.c
 * Objectif : prendre la première Face du modèle, évaluer plusieurs points
 *            paramétriques et afficher position + normale.
 *
 * Compile :
 *   gcc test2_evaluate.c -o test2_evaluate \
 *       -I/home/eelleb/ESP/EngSketchPad/include \
 *       -L/home/eelleb/ESP/EngSketchPad/lib \
 *       -legads -lm
 *
 * Exécute :
 *   ./test2_evaluate /home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "egads.h"

/* Calcule la normale unitaire à partir du résultat d'EG_evaluate */
void computeNormal(const double *r, double *n) {
    /* r[3..5] = dX/du,  r[6..8] = dX/dv */
    n[0] = r[4]*r[8] - r[5]*r[7];
    n[1] = r[5]*r[6] - r[3]*r[8];
    n[2] = r[3]*r[7] - r[4]*r[6];
    double norm = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
    if (norm > 1e-14) { n[0]/=norm; n[1]/=norm; n[2]/=norm; }
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
    if (status != EGADS_SUCCESS) { printf("EG_loadModel : %d\n", status); EG_close(context); return 1; }

    /* Récupérer les Bodies */
    ego geom;
    int oclass, mtype, nbody, *senses;
    ego *bodies;
    double limits[4];
    EG_getTopology(model, &geom, &oclass, &mtype, limits, &nbody, &bodies, &senses);

    /* Récupérer toutes les Faces du premier Body */
    int nface;
    ego *faces;
    status = EG_getBodyTopos(bodies[0], NULL, FACE, &nface, &faces);
    if (status != EGADS_SUCCESS || nface == 0) {
        printf("Erreur getBodyTopos FACE : %d\n", status);
        EG_close(context); return 1;
    }
    printf("Body 0 : %d faces disponibles.\n\n", nface);

    /* Pour les 3 premières faces, évaluer en plusieurs points (u,v) */
    int nshow = nface < 3 ? nface : 3;

    for (int iface = 0; iface < nshow; iface++) {
        ego face = faces[iface];

        /* Récupérer les bornes paramétriques de la Face */
        ego surf;
        int fc, fm, nloop, *fs;
        ego *loops;
        double uv_range[4];  /* [umin, umax, vmin, vmax] */

        status = EG_getTopology(face, &surf, &fc, &fm,
                                uv_range, &nloop, &loops, &fs);
        if (status != EGADS_SUCCESS) {
            printf("Face %d : erreur getTopology %d\n", iface, status);
            EG_free(loops);
            continue;
        }

        double umin = uv_range[0], umax = uv_range[1];
        double vmin = uv_range[2], vmax = uv_range[3];

        printf("=== FACE %d === u=[%.3f,%.3f]  v=[%.3f,%.3f]\n",
               iface, umin, umax, vmin, vmax);

        /* Évaluer en 4 points : coins et centre */
        double test_uv[5][2] = {
            {umin, vmin},
            {umax, vmin},
            {umin, vmax},
            {umax, vmax},
            {(umin+umax)/2.0, (vmin+vmax)/2.0}   /* centre */
        };
        const char *labels[5] = {
            "(umin,vmin)", "(umax,vmin)", "(umin,vmax)",
            "(umax,vmax)", "  (centre) "
        };

        for (int k = 0; k < 5; k++) {
            double uv[2]     = {test_uv[k][0], test_uv[k][1]};
            double result[18]; /* x,y,z | dX/du | dX/dv | d2X/du2 | d2X/dudv | d2X/dv2 */

            status = EG_evaluate(face, uv, result);
            if (status != EGADS_SUCCESS) {
                printf("  %s -> EG_evaluate erreur %d\n", labels[k], status);
                continue;
            }

            double normal[3];
            computeNormal(result, normal);

            printf("  %s  xyz=(%.4f, %.4f, %.4f)  n=(%.3f, %.3f, %.3f)\n",
                   labels[k],
                   result[0], result[1], result[2],
                   normal[0], normal[1], normal[2]);
        }
        printf("\n");
        EG_free(loops);
    }

    EG_free(faces);
    EG_free(bodies);
    EG_close(context);
    return 0;
}
