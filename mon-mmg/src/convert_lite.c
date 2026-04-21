#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main(int argc, char *argv[]) {
    ego context, model, geom, *bodies, tess;
    int status, oclass, mtype, nbody, *senses;
    double params[3] = {0.1, 0.01, 20.0}; 

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.egads> <output.tess>\n", argv[0]);
        return EXIT_FAILURE;
    }

    EG_open(&context);
    
    // 1. Chargement
    if (EG_loadModel(context, 0, argv[1], &model) != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur chargement\n");
        return 1;
    }

    // 2. Récupération du corps
    EG_getTopology(model, &geom, &oclass, &mtype, NULL, &nbody, &bodies, &senses);
    if (nbody < 1) return 1;

    // 3. Génération de la tessellation (le maillage de surface)
    status = EG_makeTessBody(bodies[0], params, &tess);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur tessellation: %d\n", status);
        return 1;
    }

    // 4. Sauvegarde directe de la tessellation
    // Note : On utilise .tess ou .egads, mais EG_saveTess est la clé
    remove(argv[2]);
    status = EG_saveTess(tess, argv[2]);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur écriture avec EG_saveTess (Code %d)\n", status);
        return 1;
    }

    printf("Fichier tessellé généré avec succès : %s\n", argv[2]);
    
    EG_close(context);
    return EXIT_SUCCESS;
}

// gcc convert_lite.c -o convert_egads -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legads -lm

// ./convert_egads /home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads demo0c.lite