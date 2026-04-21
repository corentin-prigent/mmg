#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main(int argc, char *argv[]) {
    ego context, model;
    ego geom, *children;
    int status, oclass, mtype, nchild, *senses;

    /* Vérification des arguments */
    if (argc != 2) {
        fprintf(stderr, "Utilisation : %s <fichier.egads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 1. Initialisation */
    status = EG_open(&context);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur d'initialisation d'EGADS.\n");
        return EXIT_FAILURE;
    }

    /* 2. Chargement du modèle CAO */
    status = EG_loadModel(context, 0, argv[1], &model);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur : Échec du chargement du fichier %s (Code: %d)\n", argv[1], status);
        EG_close(context);
        return EXIT_FAILURE;
    }
    printf("Fichier '%s' chargé.\n", argv[1]);

    /* 3. Interrogation de la topologie principale */
    status = EG_getTopology(model, &geom, &oclass, &mtype, NULL, &nchild, &children, &senses);
    if (status == EGADS_SUCCESS) {
        printf("--- Topologie ---\n");
        printf("Classe racine   : %d\n", oclass);
        printf("Type de modèle  : %d\n", mtype);
        printf("Nombre de corps : %d\n", nchild);
    } else {
        fprintf(stderr, "Erreur de lecture topologique.\n");
    }

    /* 4. Libération de la mémoire */
    EG_deleteObject(model);
    EG_close(context);

    return EXIT_SUCCESS;
}

//gcc test3.c -o test_load -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legadslite -lm

// ./test_load /home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads