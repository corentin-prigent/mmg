#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main() {
    ego context;
    int status;

    /* 1. Initialisation du contexte */
    status = EG_open(&context);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur : Échec de l'initialisation d'EGADS (Code: %d)\n", status);
        return EXIT_FAILURE;
    }
    printf("Contexte initialisé avec succès.\n");

    /* 2. Fermeture et libération de la mémoire */
    status = EG_close(context);
    if (status != EGADS_SUCCESS) {
        fprintf(stderr, "Erreur : Échec de la fermeture (Code: %d)\n", status);
        return EXIT_FAILURE;
    }
    printf("Contexte fermé avec succès.\n");

    return EXIT_SUCCESS;
}

//gcc test2.c -o test_egads -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legadslite -lm