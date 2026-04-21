#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main() {
    ego context, model, geom, *bodies, *faces;
    int status, oclass, mtype, nbody, nface, *senses;
    double pt[3] = {10.0, 5.0, 0.0}; // Point à projeter
    double result[3], uv[2];

    EG_open(&context);
    if (EG_loadModel(context, 0, "/home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads", &model) != EGADS_SUCCESS) return 1;

    /* 1. Récupérer le premier corps (Body) */
    EG_getTopology(model, &geom, &oclass, &mtype, NULL, &nbody, &bodies, &senses);
    
    /* 2. Récupérer les faces de ce corps */
    status = EG_getTopology(bodies[0], &geom, &oclass, &mtype, NULL, &nface, &faces, &senses);
    printf("Modèle chargé : %d faces trouvées.\n", nface);

    /* 3. Projeter un point sur la première face */
    /* EG_project(objet, point_entrée, point_sortie_XYZ, coordonnées_UV) */
    status = EG_project(faces[0], pt, result, uv);

    if (status == EGADS_SUCCESS) {
        printf("--- Résultat de la projection ---\n");
        printf("Point initial : (%.2f, %.2f, %.2f)\n", pt[0], pt[1], pt[2]);
        printf("Sur la surface : (%.2f, %.2f, %.2f)\n", result[0], result[1], result[2]);
        printf("Paramètres UV : U=%.4f, V=%.4f\n", uv[0], uv[1]);
    } else {
        printf("Échec de la projection (Code %d).\n", status);
    }

    EG_close(context);
    return 0;
}

// gcc test_project.c -o test_project -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legads -lm