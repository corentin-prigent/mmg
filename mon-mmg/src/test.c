/* ============================================================
 *  mesh_reader.c  –  Lecteur de maillage 2D au format MEDIT
 *
 *  Format supporté : MeshVersionFormatted 2, Dimension 2
 *  Sections lues   : Vertices, Triangles, Edges (optionnelle)
 *
 *  Compilation :  gcc -Wall -Wextra -O2 -o mesh_reader mesh_reader.c
 *  Usage       :  ./mesh_reader fichier.mesh
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 *  1. Structures de données
 * ============================================================ */

/** Sommet du maillage (coordonnées réelles + étiquette) */
typedef struct {
    double x, y;  /**< Coordonnées en double précision         */
    int    ref;   /**< Référence (ex. : numéro de bord physique) */
} Point;

/**
 * Triangle défini par trois indices de sommets (toujours 1-based,
 * conformément au format MEDIT) et une référence entière.
 */
typedef struct {
    int v0, v1, v2;  /**< Indices 1-based dans le tableau de sommets */
    int ref;          /**< Référence du triangle (sous-domaine, etc.)  */
} Triangle;

/**
 * Arête unique, normalisée p1 < p2 (indices 1-based).
 * Accumule dynamiquement les références de tous les triangles
 * qui partagent cette arête (1 → bord, 2 → arête interne).
 */
typedef struct {
    int  p1, p2;     /**< Extrémités, avec p1 < p2 (1-based)        */
    int *tri_refs;   /**< Tableau dynamique des refs des triangles   */
    int  n;          /**< Nombre d'entrées dans tri_refs             */
    int  cap;        /**< Capacité allouée (gestion interne)         */
} Arete;

/** Arête lue dans la section Edges du fichier (pour vérification) */
typedef struct {
    int p1, p2, ref;
} EdgeFile;

/** Maillage complet : regroupe toutes les données extraites */
typedef struct {
    Point    *vertices;     int n_vertices;
    Triangle *triangles;    int n_triangles;
    EdgeFile *file_edges;   int n_file_edges;
    Arete    *aretes;       int n_aretes;
} Mesh;


/* ============================================================
 *  2. Table de hachage (chaînage) pour dédupliquer les arêtes
 * ============================================================ */

typedef struct HNode {
    int           p1, p2;
    int           arete_idx;  /**< Index dans Mesh::aretes */
    struct HNode *next;
} HNode;

#define HT_SZ (1u << 17)     /* 131 072 buckets – puissance de 2 */

typedef struct { HNode **b; } HTable;

/** Fonction de hachage par multiplication de Knuth (deux clés) */
static unsigned ht_hash(int p1, int p2)
{
    unsigned u = (unsigned)p1 * 2654435761u
               ^ (unsigned)p2 * 2246822519u;
    return (u >> 15) & (HT_SZ - 1);
}

static HTable *ht_new(void)
{
    HTable *h = malloc(sizeof *h);
    if (!h) return NULL;
    h->b = calloc(HT_SZ, sizeof *h->b);
    if (!h->b) { free(h); return NULL; }
    return h;
}

static void ht_free(HTable *h)
{
    for (unsigned i = 0; i < HT_SZ; i++) {
        HNode *n = h->b[i];
        while (n) { HNode *t = n->next; free(n); n = t; }
    }
    free(h->b);
    free(h);
}

/** Retourne l'index de l'arête dans Mesh::aretes, ou -1 si absente. */
static int ht_get(const HTable *h, int p1, int p2)
{
    for (HNode *n = h->b[ht_hash(p1, p2)]; n; n = n->next)
        if (n->p1 == p1 && n->p2 == p2)
            return n->arete_idx;
    return -1;
}

/** Insère une nouvelle association (p1,p2) → arete_idx. */
static void ht_put(HTable *h, int p1, int p2, int idx)
{
    unsigned bucket = ht_hash(p1, p2);
    HNode *n = malloc(sizeof *n);
    if (!n) return;                        /* OOM silencieux */
    n->p1 = p1; n->p2 = p2; n->arete_idx = idx;
    n->next = h->b[bucket];
    h->b[bucket] = n;
}


/* ============================================================
 *  3. Gestion dynamique des références de triangles dans Arete
 * ============================================================ */

/** Ajoute tri_ref au tableau de tri_refs de l'arête a (doublage de capacité). */
static int arete_push(Arete *a, int tri_ref)
{
    if (a->n >= a->cap) {
        a->cap = (a->cap == 0) ? 2 : a->cap * 2;
        int *tmp = realloc(a->tri_refs, (size_t)a->cap * sizeof(int));
        if (!tmp) return -1;
        a->tri_refs = tmp;
    }
    a->tri_refs[a->n++] = tri_ref;
    return 0;
}


/* ============================================================
 *  4. Lecture du fichier .mesh  (parser orienté tokens)
 *
 *  Stratégie : fscanf("%s") lit les tokens un à un.
 *  Les keywords alphabétiques déclenchent la lecture de leur
 *  section ; les tokens inconnus sont ignorés (robustesse).
 * ============================================================ */

Mesh *mesh_read(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return NULL; }

    Mesh *m = calloc(1, sizeof *m);
    if (!m) { fclose(f); return NULL; }

    char tok[256];

    while (fscanf(f, "%255s", tok) == 1) {

        /* ---- En-tête ---------------------------------------- */
        if (strcmp(tok, "MeshVersionFormatted") == 0) {
            { int _v=0, _r; _r = fscanf(f, "%d", &_v); (void)_v; (void)_r; }

        /* ---- Dimension --------------------------------------- */
        } else if (strcmp(tok, "Dimension") == 0) {
            int d = 0;
            if (fscanf(f, "%d", &d) != 1 || d != 2) {
                fprintf(stderr,
                        "Erreur : seule la dimension 2 est supportée"
                        " (valeur lue : %d).\n", d);
                goto fail;
            }

        /* ---- Sommets ----------------------------------------- */
        } else if (strcmp(tok, "Vertices") == 0) {
            if (fscanf(f, "%d", &m->n_vertices) != 1) goto fail;
            m->vertices = malloc((size_t)m->n_vertices * sizeof(Point));
            if (!m->vertices) goto fail;
            for (int i = 0; i < m->n_vertices; i++) {
                Point *p = &m->vertices[i];
                if (fscanf(f, "%lf %lf %d",
                           &p->x, &p->y, &p->ref) != 3) goto fail;
            }

        /* ---- Triangles --------------------------------------- */
        } else if (strcmp(tok, "Triangles") == 0) {
            if (fscanf(f, "%d", &m->n_triangles) != 1) goto fail;
            m->triangles = malloc((size_t)m->n_triangles * sizeof(Triangle));
            if (!m->triangles) goto fail;
            for (int i = 0; i < m->n_triangles; i++) {
                Triangle *t = &m->triangles[i];
                if (fscanf(f, "%d %d %d %d",
                           &t->v0, &t->v1, &t->v2, &t->ref) != 4) goto fail;
            }

        /* ---- Arêtes de bord (optionnelle) -------------------- */
        } else if (strcmp(tok, "Edges") == 0) {
            if (fscanf(f, "%d", &m->n_file_edges) != 1) goto fail;
            m->file_edges = malloc((size_t)m->n_file_edges * sizeof(EdgeFile));
            if (!m->file_edges) goto fail;
            for (int i = 0; i < m->n_file_edges; i++) {
                EdgeFile *e = &m->file_edges[i];
                if (fscanf(f, "%d %d %d",
                           &e->p1, &e->p2, &e->ref) != 3) goto fail;
            }

        /* ---- Fin --------------------------------------------- */
        } else if (strcmp(tok, "End") == 0) {
            break;

        }
        /* Tout autre token (section inconnue) est silencieusement ignoré */
    }

    fclose(f);
    return m;

fail:
    fprintf(stderr, "Erreur de lecture dans \"%s\" (token courant : \"%s\").\n",
            path, tok);
    fclose(f);
    free(m->vertices);
    free(m->triangles);
    free(m->file_edges);
    free(m);
    return NULL;
}


/* ============================================================
 *  5. Construction des arêtes uniques à partir des triangles
 * ============================================================ */

/**
 * Pour chaque triangle (v0,v1,v2), génère les trois arêtes dans
 * l'ordre AB, BC, CA.  Chaque arête est normalisée (p1 < p2) et
 * indexée via la table de hachage pour détecter les doublons.
 * La référence du triangle courant est ajoutée à Arete::tri_refs.
 *
 * Complexité : O(3T) en moyenne avec la table de hachage.
 */
int mesh_build_aretes(Mesh *m)
{
    int cap = 0;
    m->n_aretes = 0;
    m->aretes   = NULL;

    HTable *ht = ht_new();
    if (!ht) return -1;

    for (int t = 0; t < m->n_triangles; t++) {
        const Triangle *tri = &m->triangles[t];

        /* Les trois paires dans l'ordre (v0,v1), (v1,v2), (v2,v0) */
        const int pairs[3][2] = {
            { tri->v0, tri->v1 },
            { tri->v1, tri->v2 },
            { tri->v2, tri->v0 }
        };

        for (int e = 0; e < 3; e++) {
            int a  = pairs[e][0], b = pairs[e][1];
            int p1 = (a < b) ? a : b;      /* normalisation : p1 < p2 */
            int p2 = (a < b) ? b : a;

            int idx = ht_get(ht, p1, p2);

            if (idx < 0) {
                /* Arête inédite : agrandir le tableau par doublement */
                if (m->n_aretes >= cap) {
                    cap = (cap == 0) ? 32 : cap * 2;
                    Arete *tmp = realloc(m->aretes,
                                         (size_t)cap * sizeof(Arete));
                    if (!tmp) { ht_free(ht); return -1; }
                    m->aretes = tmp;
                }
                idx = m->n_aretes++;
                m->aretes[idx] = (Arete){ p1, p2, NULL, 0, 0 };
                ht_put(ht, p1, p2, idx);
            }

            /* Enregistre la référence du triangle courant */
            if (arete_push(&m->aretes[idx], tri->ref) != 0) {
                ht_free(ht);
                return -1;
            }
        }
    }

    ht_free(ht);
    return 0;
}


/* ============================================================
 *  6. Vérification : arêtes calculées vs. section Edges
 * ============================================================ */

/**
 * Confronte les arêtes de bord calculées (n == 1) avec la section
 * Edges du fichier pour détecter d'éventuelles incohérences.
 */
static void mesh_verify_edges(const Mesh *m)
{
    printf("\n--- Vérification (arêtes calculées vs. fichier) ---\n");

    if (m->n_file_edges == 0) {
        printf("  (Section Edges absente du fichier – vérification ignorée.)\n");
        return;
    }

    int n_boundary = 0;
    for (int i = 0; i < m->n_aretes; i++)
        if (m->aretes[i].n == 1) n_boundary++;

    printf("  Arêtes de bord calculées (1 triangle seul) : %d\n", n_boundary);
    printf("  Arêtes déclarées dans Edges                : %d\n",
           m->n_file_edges);

    if (n_boundary == m->n_file_edges)
        printf("  => OK : les deux comptages coïncident.\n");
    else
        printf("  => AVERTISSEMENT : divergence de %d arête(s).\n",
               abs(n_boundary - m->n_file_edges));
}


/* ============================================================
 *  7. Affichage récapitulatif
 * ============================================================ */

void mesh_print(const Mesh *m)
{
    /* ----- En-tête ----- */
    printf("==========================================\n");
    printf("  Résumé du maillage\n");
    printf("==========================================\n");
    printf("  Sommets             : %d\n", m->n_vertices);
    printf("  Triangles           : %d\n", m->n_triangles);
    printf("  Arêtes (fichier)    : %d\n", m->n_file_edges);
    printf("  Arêtes (calculées)  : %d\n", m->n_aretes);
    printf("==========================================\n\n");

    /* ----- Sommets ----- */
    printf("--- Sommets ---\n");
    for (int i = 0; i < m->n_vertices; i++) {
        const Point *p = &m->vertices[i];
        printf("  [%3d]  (%-12.6g, %-12.6g)  ref=%d\n",
               i + 1, p->x, p->y, p->ref);
    }

    /* ----- Triangles ----- */
    printf("\n--- Triangles ---\n");
    for (int i = 0; i < m->n_triangles; i++) {
        const Triangle *t = &m->triangles[i];
        printf("  T%-4d  (%d, %d, %d)  ref=%d\n",
               i + 1, t->v0, t->v1, t->v2, t->ref);
    }

    /* ----- Arêtes calculées ----- */
    printf("\n--- Arêtes uniques calculées ---\n");
    printf("  %-12s  %-22s  %s\n", "Arête (p1,p2)", "Refs triangles", "Type");
    printf("  %-12s  %-22s  %s\n", "-------------", "--------------", "----");
    for (int i = 0; i < m->n_aretes; i++) {
        const Arete *a = &m->aretes[i];
        char refs[128] = "";
        int  pos = 0;
        for (int k = 0; k < a->n && pos < (int)sizeof refs - 8; k++)
            pos += snprintf(refs + pos, sizeof refs - (size_t)pos,
                            "%d%s", a->tri_refs[k],
                            k < a->n - 1 ? ", " : "");
        printf("  (%d, %d)%-7s  { %-20s}  %s\n",
               a->p1, a->p2, "",
               refs,
               (a->n == 1) ? "BORD" : "interne");
    }

    /* ----- Arêtes fichier ----- */
    if (m->n_file_edges > 0) {
        printf("\n--- Arêtes de bord (section Edges du fichier) ---\n");
        for (int i = 0; i < m->n_file_edges; i++) {
            const EdgeFile *e = &m->file_edges[i];
            printf("  (%d, %d)  ref=%d\n", e->p1, e->p2, e->ref);
        }
    }

    mesh_verify_edges(m);
}


/* ============================================================
 *  8. Libération mémoire
 * ============================================================ */

void mesh_free(Mesh *m)
{
    if (!m) return;
    free(m->vertices);
    free(m->triangles);
    free(m->file_edges);
    for (int i = 0; i < m->n_aretes; i++)
        free(m->aretes[i].tri_refs);
    free(m->aretes);
    free(m);
}


/* ============================================================
 *  9. Point d'entrée principal
 * ============================================================ */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <fichier.mesh>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* --- Lecture --- */
    Mesh *m = mesh_read(argv[1]);
    if (!m) return EXIT_FAILURE;

    /* --- Construction des arêtes uniques --- */
    if (mesh_build_aretes(m) != 0) {
        fprintf(stderr, "Erreur lors de la construction des arêtes.\n");
        mesh_free(m);
        return EXIT_FAILURE;
    }

    /* --- Affichage --- */
    mesh_print(m);

    /* --- Nettoyage --- */
    mesh_free(m);
    return EXIT_SUCCESS;
}