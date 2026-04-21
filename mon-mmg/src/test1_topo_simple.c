/*
 * test1_topo_simple.c
 * Version minimale : charge le modèle, affiche la hiérarchie des faces,
 * libère toutes les allocations EGADS.
 */
#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier.egads>\n", argv[0]);
        return 1;
    }

    ego context, model;
    int status;

    status = EG_open(&context);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "EG_open failed\n");
        return 1;
    }

    status = EG_loadModel(context, 0, argv[1], &model);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "EG_loadModel failed\n");
        EG_close(context);
        return 1;
    }

    /* Récupérer les bodies du modèle */
    ego geom;
    int oclass, mtype, nbody, *senses = NULL;
    ego *bodies = NULL;
    double limits[4];

    status = EG_getTopology(model, &geom, &oclass, &mtype, limits,
                            &nbody, &bodies, &senses);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "EG_getTopology(model) failed\n");
        EG_close(context);
        return 1;
    }

    printf("Modèle chargé : %d body(s)\n", nbody);

    /* Parcourir chaque body */
    for (int ib = 0; ib < nbody; ib++) {
        ego body = bodies[ib];
        int nshell, nface, nedge, nnode;
        ego *shells = NULL, *faces = NULL, *edges = NULL, *nodes = NULL;

        /* Récupérer les faces (on ne s'intéresse qu'aux faces ici) */
        status = EG_getBodyTopos(body, NULL, FACE, &nface, &faces);
        if (status != EGADS_SUCCESS) {
            fprintf(stderr, "EG_getBodyTopos(FACE) failed for body %d\n", ib);
            continue;
        }

        printf("Body %d : %d faces\n", ib, nface);

        /* Parcourir chaque face */
        for (int iface = 0; iface < nface; iface++) {
            ego face = faces[iface];
            ego surf;
            int fc, fm, nloop, *fs = NULL;
            ego *loops = NULL;
            double uv[4];

            status = EG_getTopology(face, &surf, &fc, &fm, uv,
                                    &nloop, &loops, &fs);
            if (status != EGADS_SUCCESS) {
                fprintf(stderr, "  EG_getTopology(FACE) failed for face %d\n", iface);
                continue;
            }

            printf("  Face %2d : mtype=%d, %d loop(s), u=[%.3f,%.3f] v=[%.3f,%.3f]\n",
                   iface, fm, nloop, uv[0], uv[1], uv[2], uv[3]);

            /* Libérer les tableaux alloués pour cette face */
            EG_free(loops);
            EG_free(fs);
        }

        /* Libérer les tableaux alloués pour ce body */
        EG_free(faces);
        /* On pourrait aussi libérer shells, edges, nodes si on les avait récupérés */
    }

    /* Libérer les tableaux du modèle */
    EG_free(bodies);
    EG_free(senses);

    /* Fermeture du contexte EGADS */
    EG_close(context);
    printf("Fin normale.\n");
    return 0;
}