#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main() {
    ego context, model;
    EG_open(&context);

    // Chargement direct du fichier original (B-Rep)
    int status = EG_loadModel(context, 0, "/home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads", &model);
    
    if (status == EGADS_SUCCESS) {
        printf("Succès : Le fichier B-Rep est chargé via libegads.\n");
    } else {
        printf("Erreur de chargement (Code %d).\n", status);
    }

    EG_close(context);
    return 0;
}

// gcc test_full.c -o test_full -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legads -lm

//