/*
 * test1_topo.c
 * Objectif : charger un .egads et afficher toute la hiérarchie topologique.
 *            Avec libération correcte de toutes les allocations EGADS.
 *
 * Compile :
 *   gcc test1_topo.c -o test1_topo \
 *       -I/home/eelleb/ESP/EngSketchPad/include \
 *       -L/home/eelleb/ESP/EngSketchPad/lib \
 *       -legads -lm
 *
 * Exécute :
 *   export LD_LIBRARY_PATH=/home/eelleb/ESP/EngSketchPad/lib:$LD_LIBRARY_PATH
 *   ./test1_topo /home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads
 */

#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

const char* className(int oclass) {
    switch (oclass) {
        case 20: return "NODE";
        case 21: return "EDGE";
        case 22: return "LOOP";
        case 23: return "FACE";
        case 24: return "SHELL";
        case 25: return "BODY";
        case 26: return "MODEL";
        default: return "INCONNU";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage : %s <fichier.egads>\n", argv[0]);
        return 1;
    }

    ego context, model;
    int status;

    status = EG_open(&context);
    if (status != EGADS_SUCCESS) {
        printf("Erreur EG_open : %d\n", status);
        return 1;
    }

    status = EG_loadModel(context, 0, argv[1], &model);
    if (status != EGADS_SUCCESS) {
        printf("Erreur EG_loadModel : %d\n", status);
        EG_close(context);
        return 1;
    }
    printf("Fichier charge : %s\n\n", argv[1]);

    /* --- Récupération des Bodies du modèle --- */
    ego   geom = NULL;
    int   oclass, mtype, nbody = 0, *senses = NULL;
    ego  *bodies = NULL;
    double limits[4];

    status = EG_getTopology(model, &geom, &oclass, &mtype,
                            limits, &nbody, &bodies, &senses);
    if (status != EGADS_SUCCESS) {
        printf("Erreur EG_getTopology(model) : %d\n", status);
        EG_close(context);
        return 1;
    }
    printf("MODEL  : oclass=%d (%s), %d body(ies)\n\n",
           oclass, className(oclass), nbody);

    /* --- Parcours des Bodies --- */
    for (int ib = 0; ib < nbody; ib++) {
        ego body = bodies[ib];
        int nshell = 0, nface_body = 0, nedge_body = 0, nnode_body = 0;
        ego *shells = NULL, *faces_body = NULL, *edges_body = NULL, *nodes_body = NULL;

        /* Récupération des shells */
        status = EG_getBodyTopos(body, NULL, SHELL, &nshell, &shells);
        if (status != EGADS_SUCCESS) {
            printf("  Erreur getBodyTopos SHELL : %d\n", status);
        }

        status = EG_getBodyTopos(body, NULL, FACE, &nface_body, &faces_body);
        status = EG_getBodyTopos(body, NULL, EDGE, &nedge_body, &edges_body);
        status = EG_getBodyTopos(body, NULL, NODE, &nnode_body, &nodes_body);

        printf("BODY %d : %d Shell(s), %d Face(s), %d Edge(s), %d Node(s)\n",
               ib, nshell, nface_body, nedge_body, nnode_body);

        /* --- Parcours des Faces --- */
        for (int iface = 0; iface < nface_body; iface++) {
            ego face = faces_body[iface];
            ego surf = NULL;
            int fc, fm, nloop = 0, *fs = NULL;
            ego *loops = NULL;
            double uv_range[4];

            status = EG_getTopology(face, &surf, &fc, &fm,
                                    uv_range, &nloop, &loops, &fs);
            if (status != EGADS_SUCCESS) {
                printf("  Erreur getTopology FACE %d : %d\n", iface, status);
                continue;
            }

            printf("  FACE %2d : mtype=%d, %d Loop(s), "
                   "u=[%.3f,%.3f] v=[%.3f,%.3f]\n",
                   iface, fm, nloop,
                   uv_range[0], uv_range[1],
                   uv_range[2], uv_range[3]);

            /* Libération des tableaux de cette face */
            EG_free(loops);
            EG_free(fs);
        }

        /* --- Afficher quelques Nœuds --- */
        int nshow = nnode_body < 5 ? nnode_body : 5;
        printf("\n  Premiers nodes (%d/%d) :\n", nshow, nnode_body);
        for (int in = 0; in < nshow; in++) {
            ego node = nodes_body[in];
            ego ngeom = NULL;
            int nc, nm, nch = 0, *ns = NULL;
            ego *nchildren = NULL;
            double xyz[3];

            status = EG_getTopology(node, &ngeom, &nc, &nm,
                                    xyz, &nch, &nchildren, &ns);
            if (status == EGADS_SUCCESS) {
                printf("    Node %2d : (%.4f, %.4f, %.4f)\n",
                       in, xyz[0], xyz[1], xyz[2]);
            }

            EG_free(nchildren);
            EG_free(ns);
        }
        printf("\n");

        /* Libération des tableaux propres à ce Body */
        EG_free(shells);
        EG_free(faces_body);
        EG_free(edges_body);
        EG_free(nodes_body);
    }

    /* --- Libération des tableaux du modèle --- */
    EG_free(bodies);
    EG_free(senses);

    /* --- Fermeture du contexte EGADS --- */
    EG_close(context);
    printf("Contexte ferme. OK.\n");
    return 0;
}