#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main() {
    ego context, model, geom, *bodies, *faces;
    int status, oclass, mtype, nbody, nface, *senses;
    double range[4], uv[2], result[3], uv_final[2];
    int periodic[2]; // Argument manquant : [0]=U périodique?, [1]=V périodique?

    EG_open(&context);
    if (EG_loadModel(context, 0, "/home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads", &model) != EGADS_SUCCESS) return 1;

    EG_getTopology(model, &geom, &oclass, &mtype, NULL, &nbody, &bodies, &senses);
    EG_getTopology(bodies[0], &geom, &oclass, &mtype, NULL, &nface, &faces, &senses);

    /* 1. Récupérer les limites UV (3 arguments maintenant) */
    status = EG_getRange(faces[0], range, periodic);
    if (status != EGADS_SUCCESS) {
        printf("Erreur getRange\n");
        return 1;
    }
    printf("Limites Face 0 : U=[%.2f, %.2f] V=[%.2f, %.2f]\n", range[0], range[1], range[2], range[3]);

    /* 2. Évaluation au centre de la face */
    uv[0] = (range[0] + range[1]) / 2.0;
    uv[1] = (range[2] + range[3]) / 2.0;
    
    // Note: Si EG_evaluate échoue à la compilation, vérifiez s'il manque aussi un argument
    status = EG_evaluate(faces[0], uv, result);
    printf("Point XYZ calculé : [%.2f, %.2f, %.2f]\n", result[0], result[1], result[2]);

    /* 3. Projection (Inversion) */
    status = EG_invEvaluate(faces[0], result, uv_final, result);

    if (status == EGADS_SUCCESS) {
        printf("Projection réussie !\n");
        printf("UV d'origine : U=%.4f, V=%.4f\n", uv[0], uv[1]);
        printf("UV retrouvés : U=%.4f, V=%.4f\n", uv_final[0], uv_final[1]);
    } else {
        printf("Échec de la projection (Code %d).\n", status);
    }

    EG_close(context);
    return 0;
}

// gcc test_mmg_logic.c -o test_mmg_logic -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legads -lm

// ./test_mmg_logic