#include <stdio.h>
#include <stdlib.h>

#include "libmmg2d.h"
#include "libmmg2d_private.h"

/* ------------------------------------------------------------------ */
/* Affiche les MMG5_Point du maillage                                  */
/* ------------------------------------------------------------------ */
void print_points(MMG5_pMesh mesh) {
    printf("\n=== POINTS (np = %d) ===\n", mesh->np);
    printf("  %-6s  %-12s %-12s  %-6s  %-6s\n",
           "idx", "x", "y", "ref", "tag");
    printf("  %-6s  %-12s %-12s  %-6s  %-6s\n",
           "---", "-----------", "-----------", "---", "---");

    /* Les tableaux MMG sont 1-indexés : point[1] ... point[np] */
    for (int i = 1; i <= mesh->np; i++) {
        MMG5_pPoint p = &mesh->point[i];
        printf("  %-6d  %-12.6f %-12.6f  %-6d  %-6d\n",
               i, p->c[0], p->c[1], p->ref, (int)p->tag);
    }
}

/* ------------------------------------------------------------------ */
/* Affiche les MMG5_Tria du maillage                                   */
/* ------------------------------------------------------------------ */
void print_triangles(MMG5_pMesh mesh) {
    printf("\n=== TRIANGLES (nt = %d) ===\n", mesh->nt);
    printf("  %-6s  %-6s %-6s %-6s  %-6s  %-12s\n",
           "idx", "v[0]", "v[1]", "v[2]", "ref", "qual");
    printf("  %-6s  %-6s %-6s %-6s  %-6s  %-12s\n",
           "---", "----", "----", "----", "---", "----");

    for (int i = 1; i <= mesh->nt; i++) {
        MMG5_pTria t = &mesh->tria[i];
        printf("  %-6d  %-6d %-6d %-6d  %-6d  %-12.6f\n",
               i, t->v[0], t->v[1], t->v[2], t->ref, t->qual);
    }
}

/* ------------------------------------------------------------------ */
/* Affiche les MMG5_Edge du maillage                                   */
/* ------------------------------------------------------------------ */
void print_edges(MMG5_pMesh mesh) {
    printf("\n=== EDGES (na = %d) ===\n", mesh->na);
    printf("  %-6s  %-6s %-6s  %-6s  %-6s\n",
           "idx", "a", "b", "ref", "tag");
    printf("  %-6s  %-6s %-6s  %-6s  %-6s\n",
           "---", "---", "---", "---", "---");

    for (int i = 1; i <= mesh->na; i++) {
        MMG5_pEdge e = &mesh->edge[i];
        printf("  %-6d  %-6d %-6d  %-6d  %-6d\n",
               i, e->a, e->b, e->ref, (int)e->tag);
    }
}

/* ------------------------------------------------------------------ */
/* Affiche le résumé global de MMG5_Mesh                               */
/* ------------------------------------------------------------------ */
void print_mesh_info(MMG5_pMesh mesh) {
    printf("\n=== MMG5_Mesh — résumé ===\n");
    printf("  Dimension         : %d\n",   mesh->dim);
    printf("  Version fichier   : %d\n",   mesh->ver);
    printf("  np  (points)      : %d\n",   mesh->np);
    printf("  nt  (triangles)   : %d\n",   mesh->nt);
    printf("  na  (arêtes)      : %d\n",   mesh->na);
    printf("  nquad (quads)     : %d\n",   mesh->nquad);
    printf("  Mémoire utilisée  : %zu Mo\n", mesh->memCur / (1024*1024));
    printf("  Mémoire max       : %zu Mo\n", mesh->memMax / (1024*1024));
    printf("  Fichier entrée    : %s\n",
           mesh->namein  ? mesh->namein  : "(non défini)");
    printf("  Fichier sortie    : %s\n",
           mesh->nameout ? mesh->nameout : "(non défini)");
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage : %s fichier.mesh\n", argv[0]);
        return EXIT_FAILURE;
    }

    MMG5_pMesh mmgMesh = NULL;
    MMG5_pSol  mmgSol  = NULL;

    /* 1. Initialisation */
    if (MMG2D_Init_mesh(MMG5_ARG_start,
                        MMG5_ARG_ppMesh, &mmgMesh,
                        MMG5_ARG_ppMet,  &mmgSol,
                        MMG5_ARG_end) != 1) {
        fprintf(stderr, "Erreur : MMG2D_Init_mesh\n");
        return EXIT_FAILURE;
    }

    /* 2. Chargement du fichier .mesh */
    if (MMG2D_loadMesh(mmgMesh, argv[1]) != 1) {
        fprintf(stderr, "Erreur : impossible de charger '%s'\n", argv[1]);
        MMG2D_Free_all(MMG5_ARG_start,
                       MMG5_ARG_ppMesh, &mmgMesh,
                       MMG5_ARG_ppMet,  &mmgSol,
                       MMG5_ARG_end);
        return EXIT_FAILURE;
    }

    /* 3. Affichage */
    print_mesh_info(mmgMesh);
    print_points(mmgMesh);
    print_triangles(mmgMesh);
    print_edges(mmgMesh);

    /* 4. Nettoyage */
    MMG2D_Free_all(MMG5_ARG_start,
                   MMG5_ARG_ppMesh, &mmgMesh,
                   MMG5_ARG_ppMet,  &mmgSol,
                   MMG5_ARG_end);

    return EXIT_SUCCESS;
}