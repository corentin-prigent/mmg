#include <stdio.h>
#include <stdlib.h>
#include "egads.h"

int main() {
    ego context, model, geom, *bodies, *children;
    int status, oclass, mtype, nbody, nchild, *senses;
    double range[4];
    int periodic[2];

    EG_open(&context);
    if (EG_loadModel(context, 0, "/home/eelleb/ESP/EngSketchPad/data/basic/demo0c.egads", &model) != EGADS_SUCCESS) return 1;

    /* 1. Analyse du Modèle -> Corps */
    EG_getTopology(model, &geom, &oclass, &mtype, NULL, &nbody, &bodies, &senses);
    printf("Modèle chargé : %d corps trouvé(s).\n", nbody);

    /* 2. Analyse du Corps -> Enfants (Faces ou Shells ?) */
    status = EG_getTopology(bodies[0], &geom, &oclass, &mtype, NULL, &nchild, &children, &senses);
    printf("Corps 0 : %d enfants trouvés. Classe de l'enfant : %d (14=FACE, 16=SHELL)\n", nchild, oclass);

    if (nchild > 0) {
        /* Si c'est un SHELL (16), il faut descendre d'un niveau pour avoir les FACES */
        if (oclass == 16) {
            printf("Objet de type SHELL détecté. Descente vers les faces...\n");
            ego *faces;
            status = EG_getTopology(children[0], &geom, &oclass, &mtype, NULL, &nchild, &faces, &senses);
            children = faces; // On cible maintenant les faces du premier shell
        }

        /* 3. Test de Range sur la première FACE trouvée */
        status = EG_getRange(children[0], range, periodic);
        if (status == EGADS_SUCCESS) {
            printf("Succès ! Limites UV : U=[%.2f, %.2f] V=[%.2f, %.2f]\n", range[0], range[1], range[2], range[3]);
        } else {
            printf("Erreur persistante sur getRange (Code %d). Classe de l'objet : %d\n", status, oclass);
        }
    }

    EG_close(context);
    return 0;
}

//gcc test_debug.c -o test_debug -I/home/eelleb/ESP/EngSketchPad/include -L/home/eelleb/ESP/EngSketchPad/lib -legads -lm
./test_debug