/* =============================================================================
** main2.c — Rapport exhaustif d'un maillage MMG2D
** Usage : ./mon_test2 fichier.mesh [options] [> rapport.txt]
**
** Options :
**   --all            Tout afficher (défaut)
**   --global         Champs globaux MMG5_Mesh
**   --info           Paramètres MMG5_Info
**   --stats          Statistiques globales
**   --points         MMG5_Point
**   --xpoints        MMG5_xPoint
**   --triangles      MMG5_Tria
**   --edges          MMG5_Edge
**   --htab           Hash table MMG5_HGeom
**   --adjacency      Tables d'adjacence
**   --sol            MMG5_Sol (métrique)
**   --max N          Limite à N éléments par tableau (défaut : tout)
**   --no-color       Désactive les couleurs ANSI
** =============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>   /* isatty */

#include "libmmg2d.h"
#include "libmmg2d_private.h"

/* ===================================================================
 * COULEURS ANSI — pointeurs globaux (pas de macros ternaires)
 * =================================================================== */
static int USE_COLOR = 1;

static const char *COL_RESET   = "";
static const char *COL_BOLD    = "";
static const char *COL_CYAN    = "";
static const char *COL_YELLOW  = "";
static const char *COL_GREEN   = "";
static const char *COL_RED     = "";
static const char *COL_MAGENTA = "";
static const char *COL_BLUE    = "";
static const char *COL_GREY    = "";
static const char *COL_WHITE   = "";

static void init_colors(void) {
    if (!USE_COLOR) return;
    COL_RESET   = "\033[0m";
    COL_BOLD    = "\033[1m";
    COL_CYAN    = "\033[1;36m";
    COL_YELLOW  = "\033[1;33m";
    COL_GREEN   = "\033[0;32m";
    COL_RED     = "\033[0;31m";
    COL_MAGENTA = "\033[0;35m";
    COL_BLUE    = "\033[0;34m";
    COL_GREY    = "\033[0;90m";
    COL_WHITE   = "\033[1;37m";
}

/* ===================================================================
 * OPTIONS
 * =================================================================== */
typedef struct {
    int show_global;
    int show_info;
    int show_stats;
    int show_points;
    int show_xpoints;
    int show_triangles;
    int show_edges;
    int show_htab;
    int show_adjacency;
    int show_sol;
    int max_rows;       /* -1 = tout afficher */
} Options;

static void options_default(Options *o) {
    o->show_global    = 1;
    o->show_info      = 1;
    o->show_stats     = 1;
    o->show_points    = 1;
    o->show_xpoints   = 1;
    o->show_triangles = 1;
    o->show_edges     = 1;
    o->show_htab      = 1;
    o->show_adjacency = 1;
    o->show_sol       = 1;
    o->max_rows       = -1;
}

static void options_clear(Options *o) {
    o->show_global    = 0;
    o->show_info      = 0;
    o->show_stats     = 0;
    o->show_points    = 0;
    o->show_xpoints   = 0;
    o->show_triangles = 0;
    o->show_edges     = 0;
    o->show_htab      = 0;
    o->show_adjacency = 0;
    o->show_sol       = 0;
}

/* ===================================================================
 * UTILITAIRES D'AFFICHAGE
 * =================================================================== */

/* Ligne de séparation principale */
static void box_title(FILE *out, const char *title, const char *color) {
    int width = 70;
    int tlen  = (int)strlen(title);
    int pad   = (width - 2 - tlen) / 2;

    fprintf(out, "\n%s", color);
    for (int i = 0; i < width; i++) fprintf(out, "═");
    fprintf(out, "\n");

    fprintf(out, "║");
    for (int i = 0; i < pad; i++) fprintf(out, " ");
    fprintf(out, "%s%s%s%s", COL_WHITE, title, COL_RESET, color);
    int right = width - 2 - pad - tlen;
    for (int i = 0; i < right; i++) fprintf(out, " ");
    fprintf(out, "║\n");

    for (int i = 0; i < width; i++) fprintf(out, "═");
    fprintf(out, "%s\n", COL_RESET);
}

/* Sous-titre de section */
static void section_title(FILE *out, const char *title) {
    fprintf(out, "\n%s┌─ %s%s%s ─%s\n",
            COL_YELLOW, COL_BOLD, title, COL_RESET, COL_YELLOW);
    fprintf(out, "%s", COL_RESET);
}

/* Ligne clé/valeur */
static void kv(FILE *out, const char *key, const char *fmt, ...) {
    fprintf(out, "  %s%-30s%s ", COL_CYAN, key, COL_RESET);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    fprintf(out, "\n");
}

/* Décode les flags binaires tag en chaîne lisible */
static void decode_tag(uint16_t tag, char *buf, int bufsize) {
    buf[0] = '\0';
    if (tag & (1 << 0)) strncat(buf, "BDY ",    bufsize - strlen(buf) - 1);
    if (tag & (1 << 1)) strncat(buf, "REF ",    bufsize - strlen(buf) - 1);
    if (tag & (1 << 2)) strncat(buf, "GEO ",    bufsize - strlen(buf) - 1);
    if (tag & (1 << 3)) strncat(buf, "NOM ",    bufsize - strlen(buf) - 1);
    if (tag & (1 << 4)) strncat(buf, "REQ ",    bufsize - strlen(buf) - 1);
    if (tag & (1 << 5)) strncat(buf, "NOSURF ", bufsize - strlen(buf) - 1);
    if (tag & (1 << 6)) strncat(buf, "OPNBDY ", bufsize - strlen(buf) - 1);
    if (tag == 0)       strncat(buf, "-",        bufsize - strlen(buf) - 1);
}

/* Colorie un tag décodé selon son contenu */
static void print_tag_colored(FILE *out, const char *tagstr) {
    if (strstr(tagstr, "BDY") || strstr(tagstr, "GEO"))
        fprintf(out, "%s%-18s%s", COL_GREEN,  tagstr, COL_RESET);
    else if (strstr(tagstr, "REQ"))
        fprintf(out, "%s%-18s%s", COL_RED,    tagstr, COL_RESET);
    else if (strstr(tagstr, "REF"))
        fprintf(out, "%s%-18s%s", COL_MAGENTA,tagstr, COL_RESET);
    else
        fprintf(out, "%s%-18s%s", COL_GREY,   tagstr, COL_RESET);
}

/* En-tête de tableau générique */
static void table_header(FILE *out, const char **cols, const int *widths, int ncols) {
    fprintf(out, "\n  %s", COL_BOLD);
    for (int i = 0; i < ncols; i++)
        fprintf(out, "%-*s  ", widths[i], cols[i]);
    fprintf(out, "%s\n  %s", COL_RESET, COL_GREY);
    for (int i = 0; i < ncols; i++) {
        for (int j = 0; j < widths[i]; j++) fprintf(out, "─");
        fprintf(out, "  ");
    }
    fprintf(out, "%s\n", COL_RESET);
}

/* Affiche "N affichés / M total" si troncature */
static void print_truncation_notice(FILE *out, int shown, int total) {
    if (shown < total)
        fprintf(out, "\n  %s[... %d éléments affichés sur %d — utilisez --max N pour ajuster ...]%s\n",
                COL_GREY, shown, total, COL_RESET);
}

/* Calcule le nombre de lignes à afficher */
static int display_count(int total, int max_rows) {
    if (max_rows < 0 || max_rows >= total) return total;
    return max_rows;
}

/* ===================================================================
 * MMG5_Mesh — champs globaux
 * =================================================================== */
void print_mesh_global(MMG5_pMesh m, FILE *out) {
    box_title(out, "MMG5_Mesh — champs globaux", COL_CYAN);

    section_title(out, "Topologie");
    kv(out, "Dimension",         "%d", m->dim);
    kv(out, "Version fichier",   "%d", m->ver);
    kv(out, "Type de maillage",  "%d", m->type);

    section_title(out, "Compteurs actuels");
    kv(out, "np  (points)",      "%s%d%s", COL_WHITE, m->np,     COL_RESET);
    kv(out, "nt  (triangles)",   "%s%d%s", COL_WHITE, m->nt,     COL_RESET);
    kv(out, "na  (arêtes)",      "%s%d%s", COL_WHITE, m->na,     COL_RESET);
    kv(out, "ne  (tétraèdres)",  "%d", m->ne);
    kv(out, "nquad",             "%d", m->nquad);
    kv(out, "nprism",            "%d", m->nprism);
    kv(out, "xp  (pts surface)", "%d", m->xp);
    kv(out, "xt  (surf elem)",   "%d", m->xt);

    section_title(out, "Capacités allouées");
    kv(out, "npmax",  "%d", m->npmax);
    kv(out, "ntmax",  "%d", m->ntmax);
    kv(out, "namax",  "%d", m->namax);
    kv(out, "nemax",  "%d", m->nemax);
    kv(out, "xpmax",  "%d", m->xpmax);
    kv(out, "xtmax",  "%d", m->xtmax);

    section_title(out, "Premiers éléments libres");
    kv(out, "npnil", "%d", m->npnil);
    kv(out, "nenil", "%d", m->nenil);
    kv(out, "nanil", "%d", m->nanil);

    section_title(out, "Mémoire");
    kv(out, "memCur", "%zu octets  %s(%zu Ko)%s",
       m->memCur, COL_GREY, m->memCur / 1024, COL_RESET);
    kv(out, "memMax", "%zu octets  %s(%zu Mo)%s",
       m->memMax, COL_GREY, m->memMax / (1024*1024), COL_RESET);
    kv(out, "gap",    "%.4f", m->gap);

    section_title(out, "Fichiers");
    kv(out, "namein",  "%s%s%s", COL_GREEN,
       m->namein  ? m->namein  : "(non défini)", COL_RESET);
    kv(out, "nameout", "%s%s%s", COL_GREEN,
       m->nameout ? m->nameout : "(non défini)", COL_RESET);

    section_title(out, "Drapeaux internes");
    kv(out, "base",  "%d", m->base);
    kv(out, "mark",  "%d", m->mark);
    kv(out, "nsols", "%d", m->nsols);
    kv(out, "nc1",   "%d", m->nc1);
}

/* ===================================================================
 * MMG5_Info
 * =================================================================== */
void print_info(MMG5_pMesh m, FILE *out) {
    box_title(out, "MMG5_Info — paramètres du job", COL_BLUE);
    MMG5_Info *info = &m->info;

    section_title(out, "Tailles d'arêtes");
    kv(out, "hmin",     "%g", info->hmin);
    kv(out, "hmax",     "%g", info->hmax);
    kv(out, "hsiz",     "%g", info->hsiz);
    kv(out, "hgrad",    "%g", info->hgrad);
    kv(out, "hgradreq", "%g", info->hgradreq);
    kv(out, "hausd",    "%g", info->hausd);
    kv(out, "dhd",      "%g", info->dhd);

    section_title(out, "Boîte englobante (info)");
    kv(out, "min", "[%.6f, %.6f, %.6f]", info->min[0], info->min[1], info->min[2]);
    kv(out, "max", "[%.6f, %.6f, %.6f]", info->max[0], info->max[1], info->max[2]);
    kv(out, "delta", "%g", info->delta);

    section_title(out, "Modes");
    kv(out, "iso (level-set)", "%d", info->iso);
    kv(out, "isosurf",         "%d", info->isosurf);
    kv(out, "isoref",          "%d", info->isoref);
    kv(out, "ls (isovalue)",   "%g", info->ls);
    kv(out, "lag (lagrangien)","% d", info->lag);
    kv(out, "setfem",          "%d", info->setfem);
    kv(out, "opnbdy",          "%d", info->opnbdy);

    section_title(out, "Opérations");
    kv(out, "noinsert", info->noinsert ? "%sOFF%s" : "%sON%s",
       info->noinsert ? COL_RED : COL_GREEN, COL_RESET);
    kv(out, "noswap",   info->noswap   ? "%sOFF%s" : "%sON%s",
       info->noswap   ? COL_RED : COL_GREEN, COL_RESET);
    kv(out, "nomove",   info->nomove   ? "%sOFF%s" : "%sON%s",
       info->nomove   ? COL_RED : COL_GREEN, COL_RESET);
    kv(out, "nosurf",   info->nosurf   ? "%sOFF%s" : "%sON%s",
       info->nosurf   ? COL_RED : COL_GREEN, COL_RESET);
    kv(out, "nosizreq", info->nosizreq ? "%sOFF%s" : "%sON%s",
       info->nosizreq ? COL_RED : COL_GREEN, COL_RESET);

    section_title(out, "Optimisation");
    kv(out, "optim",     "%d", info->optim);
    kv(out, "optimLES",  "%d", info->optimLES);
    kv(out, "ani",       "%d", info->ani);
    kv(out, "metRidTyp", "%d", info->metRidTyp);

    section_title(out, "Divers");
    kv(out, "imprim (verbosité)", "%d", info->imprim);
    kv(out, "ddebug",             "%d", info->ddebug);
    kv(out, "nreg",               "%d", info->nreg);
    kv(out, "xreg",               "%d", info->xreg);
    kv(out, "renum (scotch)",     "%d", info->renum);
    kv(out, "mem (Mo limite)",    "%d", info->mem);
    kv(out, "npar / npari",       "%d / %d", info->npar, info->npari);
    kv(out, "nmat / nmati",       "%d / %d", info->nmat, info->nmati);
    kv(out, "nsd",                "%d", info->nsd);
    kv(out, "sethmin / sethmax",  "%d / %d", info->sethmin, info->sethmax);
    kv(out, "badkal",             info->badkal ? "%s1 (⚠ mauvais élément)%s" : "0",
       COL_RED, COL_RESET);
    kv(out, "fparam", "%s", info->fparam ? info->fparam : "(non défini)");

    if (info->npar > 0 && info->par) {
        section_title(out, "MMG5_Par — paramètres locaux");
        const char *cols[] = {"idx","elt","hmin","hmax","hausd","ref"};
        const int   wids[] = {4,    5,    12,    12,    12,     5};
        table_header(out, cols, wids, 6);
        for (int i = 0; i < info->npar; i++) {
            fprintf(out, "  %-4d  %-5d  %-12g %-12g %-12g %-5d\n",
                    i, info->par[i].elt,
                    info->par[i].hmin, info->par[i].hmax,
                    info->par[i].hausd, info->par[i].ref);
        }
    } else {
        fprintf(out, "  %s(aucun paramètre local)%s\n", COL_GREY, COL_RESET);
    }
}

/* ===================================================================
 * STATISTIQUES
 * =================================================================== */
void print_stats(MMG5_pMesh m, FILE *out) {
    box_title(out, "Statistiques globales", COL_MAGENTA);

    if (m->nt > 0 && m->tria) {
        double qmin = 1e30, qmax = 0.0, qmoy = 0.0;
        int ndeg = 0, ngood = 0;
        for (int i = 1; i <= m->nt; i++) {
            double q = m->tria[i].qual;
            if (q < qmin) qmin = q;
            if (q > qmax) qmax = q;
            qmoy += q;
            if (q < 0.01) ndeg++;
            if (q > 0.5)  ngood++;
        }
        qmoy /= m->nt;

        section_title(out, "Qualité des triangles");
        kv(out, "Minimum", "%s%.6f%s", qmin < 0.1 ? COL_RED : COL_GREEN, qmin, COL_RESET);
        kv(out, "Maximum", "%.6f", qmax);
        kv(out, "Moyenne", "%s%.6f%s",
           qmoy > 0.5 ? COL_GREEN : qmoy > 0.2 ? COL_YELLOW : COL_RED, qmoy, COL_RESET);
        kv(out, "Bons (q > 0.5)",       "%s%d%s / %d  %s(%.1f%%)%s",
           COL_GREEN,  ngood, COL_RESET, m->nt,
           COL_GREY, 100.0*ngood/m->nt, COL_RESET);
        kv(out, "Dégénérés (q < 0.01)", "%s%d%s / %d",
           ndeg > 0 ? COL_RED : COL_GREY, ndeg, COL_RESET, m->nt);

        /* Histogramme ascii */
        section_title(out, "Histogramme qualité (par tranches de 0.1)");
        int bins[10] = {0};
        for (int i = 1; i <= m->nt; i++) {
            int b = (int)(m->tria[i].qual * 10.0);
            if (b > 9) b = 9;
            bins[b]++;
        }
        for (int b = 9; b >= 0; b--) {
            int bar = bins[b] * 40 / m->nt;
            const char *col = (b >= 7) ? COL_GREEN :
                              (b >= 4) ? COL_YELLOW : COL_RED;
            fprintf(out, "  %s[%.1f-%.1f[%s  %s",
                    COL_GREY, b*0.1, (b+1)*0.1, COL_RESET, col);
            for (int j = 0; j < bar; j++) fprintf(out, "█");
            fprintf(out, "%s  %s%d%s\n", COL_RESET, COL_WHITE, bins[b], COL_RESET);
        }
    }

    if (m->np > 0 && m->point) {
        double xmin=1e30, xmax=-1e30, ymin=1e30, ymax=-1e30;
        for (int i = 1; i <= m->np; i++) {
            double x = m->point[i].c[0], y = m->point[i].c[1];
            if (x < xmin) xmin = x;  if (x > xmax) xmax = x;
            if (y < ymin) ymin = y;  if (y > ymax) ymax = y;
        }
        section_title(out, "Bounding box réelle");
        kv(out, "X", "[%s%.7f%s, %s%.7f%s]  %s(Δ = %.7f)%s",
           COL_WHITE, xmin, COL_RESET, COL_WHITE, xmax, COL_RESET,
           COL_GREY,  xmax - xmin, COL_RESET);
        kv(out, "Y", "[%s%.7f%s, %s%.7f%s]  %s(Δ = %.7f)%s",
           COL_WHITE, ymin, COL_RESET, COL_WHITE, ymax, COL_RESET,
           COL_GREY,  ymax - ymin, COL_RESET);
    }

    /* Références uniques */
    section_title(out, "Références utilisées");
    /* Points */
    {
        int refs[256] = {0}; int nrefs = 0;
        for (int i = 1; i <= m->np; i++) {
            int r = (int)m->point[i].ref;
            if (r >= 0 && r < 256 && !refs[r]) { refs[r] = 1; nrefs++; }
        }
        fprintf(out, "  %sPoints    (%d réfs):%s", COL_CYAN, nrefs, COL_RESET);
        for (int r = 0; r < 256; r++) if (refs[r]) fprintf(out, " %d", r);
        fprintf(out, "\n");
    }
    /* Triangles */
    {
        int refs[256] = {0}; int nrefs = 0;
        for (int i = 1; i <= m->nt; i++) {
            int r = (int)m->tria[i].ref;
            if (r >= 0 && r < 256 && !refs[r]) { refs[r] = 1; nrefs++; }
        }
        fprintf(out, "  %sTriangles (%d réfs):%s", COL_CYAN, nrefs, COL_RESET);
        for (int r = 0; r < 256; r++) if (refs[r]) fprintf(out, " %d", r);
        fprintf(out, "\n");
    }
    /* Arêtes */
    if (m->na > 0 && m->edge) {
        int refs[256] = {0}; int nrefs = 0;
        for (int i = 1; i <= m->na; i++) {
            int r = (int)m->edge[i].ref;
            if (r >= 0 && r < 256 && !refs[r]) { refs[r] = 1; nrefs++; }
        }
        fprintf(out, "  %sArêtes    (%d réfs):%s", COL_CYAN, nrefs, COL_RESET);
        for (int r = 0; r < 256; r++) if (refs[r]) fprintf(out, " %d", r);
        fprintf(out, "\n");
    }
}

/* ===================================================================
 * MMG5_Point
 * =================================================================== */
void print_points(MMG5_pMesh m, FILE *out, int max_rows) {
    box_title(out, "MMG5_Point — sommets", COL_GREEN);
    fprintf(out, "  %sNombre de points : %s%d%s\n",
            COL_GREY, COL_WHITE, m->np, COL_RESET);

    const char *cols[] = {"idx","x","y","ref","xp","flag","s","del","tag"};
    const int   wids[] = {6,    14,  14,  5,   5,   5,    4,  4,   20};
    table_header(out, cols, wids, 9);

    int n = display_count(m->np, max_rows);
    for (int i = 1; i <= n; i++) {
        MMG5_pPoint p = &m->point[i];
        char tagbuf[128];
        decode_tag(p->tag, tagbuf, sizeof(tagbuf));

        /* Colorier la ligne selon le type de point */
        const char *row_col = COL_RESET;
        if (p->tag & (1 << 0)) row_col = COL_GREEN;   /* BDY */
        if (p->tag & (1 << 4)) row_col = COL_RED;     /* REQ */

        fprintf(out, "  %s%-6d%s  %-14.7f %-14.7f  %-5d %-5d  %-5d %-4d %-4d  ",
                row_col, i, COL_RESET,
                p->c[0], p->c[1],
                p->ref, p->xp, p->flag, p->s, p->tagdel);
        print_tag_colored(out, tagbuf);
        fprintf(out, "\n");
    }
    print_truncation_notice(out, n, m->np);

    /* Normales / tangentes */
    section_title(out, "Normales / tangentes (valeurs non nulles uniquement)");
    int found = 0;
    for (int i = 1; i <= m->np; i++) {
        MMG5_pPoint p = &m->point[i];
        if (p->n[0] != 0.0 || p->n[1] != 0.0 || p->n[2] != 0.0) {
            fprintf(out, "  %spoint[%d]%s  n = [%.6f, %.6f, %.6f]\n",
                    COL_WHITE, i, COL_RESET, p->n[0], p->n[1], p->n[2]);
            found = 1;
        }
    }
    if (!found)
        fprintf(out, "  %s(toutes nulles — attendu en 2D)%s\n", COL_GREY, COL_RESET);
}

/* ===================================================================
 * MMG5_xPoint
 * =================================================================== */
void print_xpoints(MMG5_pMesh m, FILE *out) {
    box_title(out, "MMG5_xPoint — points de surface", COL_GREEN);
    kv(out, "Nombre de xpoints", "%d", m->xp);

    if (!m->xpoint || m->xp == 0) {
        fprintf(out, "  %s(aucun xpoint)%s\n", COL_GREY, COL_RESET);
        return;
    }
    for (int i = 1; i <= m->xp; i++) {
        MMG5_pxPoint xp = &m->xpoint[i];
        fprintf(out, "\n  %sxpoint[%d]%s\n", COL_WHITE, i, COL_RESET);
        fprintf(out, "    %sn1%s = [%12.6f, %12.6f, %12.6f]\n",
                COL_CYAN, COL_RESET, xp->n1[0], xp->n1[1], xp->n1[2]);
        fprintf(out, "    %sn2%s = [%12.6f, %12.6f, %12.6f]\n",
                COL_CYAN, COL_RESET, xp->n2[0], xp->n2[1], xp->n2[2]);
        fprintf(out, "    %snnor%s = %d  %s%s%s\n",
                COL_CYAN, COL_RESET, xp->nnor,
                COL_GREY, xp->nnor ? "(pas de normale)" : "", COL_RESET);
    }
}

/* ===================================================================
 * MMG5_Tria
 * =================================================================== */
void print_triangles(MMG5_pMesh m, FILE *out, int max_rows) {
    box_title(out, "MMG5_Tria — triangles", COL_YELLOW);
    fprintf(out, "  %sNombre de triangles : %s%d%s\n",
            COL_GREY, COL_WHITE, m->nt, COL_RESET);

    const char *cols[] = {"idx","v0","v1","v2","ref","qual","base","flag",
                          "edg0","edg1","edg2","tag[0]","tag[1]","tag[2]"};
    const int   wids[] = {6,     6,   6,   6,   4,   10,     5,    5,
                          5,    5,    5,    18,    18,    18};
    table_header(out, cols, wids, 14);

    int n = display_count(m->nt, max_rows);
    for (int i = 1; i <= n; i++) {
        MMG5_pTria t = &m->tria[i];
        char t0[64], t1[64], t2[64];
        decode_tag(t->tag[0], t0, sizeof(t0));
        decode_tag(t->tag[1], t1, sizeof(t1));
        decode_tag(t->tag[2], t2, sizeof(t2));

        /* Colorier selon la qualité */
        const char *qcol = (t->qual > 0.5) ? COL_GREEN :
                           (t->qual > 0.1) ? COL_YELLOW : COL_RED;

        fprintf(out, "  %-6d  %-6d %-6d %-6d  %-4d  %s%-10.6f%s  %-5d %-5d  "
                     "%-5d %-5d %-5d  ",
                i,
                t->v[0], t->v[1], t->v[2],
                t->ref,
                qcol, t->qual, COL_RESET,
                t->base, t->flag,
                t->edg[0], t->edg[1], t->edg[2]);
        print_tag_colored(out, t0);
        fprintf(out, "  ");
        print_tag_colored(out, t1);
        fprintf(out, "  ");
        print_tag_colored(out, t2);
        fprintf(out, "\n");
    }
    print_truncation_notice(out, n, m->nt);
}

/* ===================================================================
 * MMG5_Edge
 * =================================================================== */
void print_edges(MMG5_pMesh m, FILE *out, int max_rows) {
    box_title(out, "MMG5_Edge — arêtes", COL_YELLOW);
    fprintf(out, "  %sNombre d'arêtes : %s%d%s\n",
            COL_GREY, COL_WHITE, m->na, COL_RESET);

    if (!m->edge || m->na == 0) {
        fprintf(out, "  %s(aucune arête explicite)%s\n", COL_GREY, COL_RESET);
        return;
    }

    const char *cols[] = {"idx","a","b","ref","base","tag"};
    const int   wids[] = {6,    7,  7,  5,    6,     20};
    table_header(out, cols, wids, 6);

    int n = display_count(m->na, max_rows);
    for (int i = 1; i <= n; i++) {
        MMG5_pEdge e = &m->edge[i];
        char tagbuf[128];
        decode_tag(e->tag, tagbuf, sizeof(tagbuf));
        fprintf(out, "  %-6d  %-7d %-7d  %-5d  %-6d  ",
                i, e->a, e->b, e->ref, e->base);
        print_tag_colored(out, tagbuf);
        fprintf(out, "\n");
    }
    print_truncation_notice(out, n, m->na);
}

/* ===================================================================
 * MMG5_HGeom — hash table
 * =================================================================== */
void print_htab(MMG5_pMesh m, FILE *out) {
    box_title(out, "MMG5_HGeom — hash table arêtes géométriques", COL_BLUE);
    MMG5_HGeom *h = &m->htab;
    kv(out, "siz (utilisée)", "%d", h->siz);
    kv(out, "max (capacité)", "%d", h->max);
    kv(out, "nxt (libre)",    "%d", h->nxt);

    if (!h->geom || h->siz == 0) {
        fprintf(out, "  %s(table vide)%s\n", COL_GREY, COL_RESET);
        return;
    }

    const char *cols[] = {"idx","a","b","ref","nxt","tag"};
    const int   wids[] = {6,    7,  7,  5,    5,    20};
    table_header(out, cols, wids, 6);

    for (int i = 0; i < h->siz; i++) {
        MMG5_hgeom *g = &h->geom[i];
        if (g->a == 0 && g->b == 0) continue;
        char tagbuf[128];
        decode_tag(g->tag, tagbuf, sizeof(tagbuf));
        fprintf(out, "  %-6d  %-7d %-7d  %-5d  %-5d  ",
                i, g->a, g->b, g->ref, g->nxt);
        print_tag_colored(out, tagbuf);
        fprintf(out, "\n");
    }
}

/* ===================================================================
 * Tables d'adjacence
 * =================================================================== */
void print_adjacency(MMG5_pMesh m, FILE *out, int max_rows) {
    box_title(out, "Tables d'adjacence", COL_BLUE);

    if (m->adjt && m->nt > 0) {
        section_title(out, "adjt — adjacence triangles");
        fprintf(out, "  %sadjt[3*(i-1)+1+j] = 3*k+l  "
                     "→ tria i, arête j ↔ tria k, arête l%s\n",
                COL_GREY, COL_RESET);

        const char *cols[] = {"tria","voisin j=0","arête","voisin j=1","arête","voisin j=2","arête"};
        const int   wids[] = {5,     9,            5,      9,           5,      9,           5};
        table_header(out, cols, wids, 7);

        int n = display_count(m->nt, max_rows);
        for (int i = 1; i <= n; i++) {
            int a0 = m->adjt[3*(i-1)+1+0];
            int a1 = m->adjt[3*(i-1)+1+1];
            int a2 = m->adjt[3*(i-1)+1+2];
            /* 0 = bord (pas de voisin) */
            const char *bc = COL_RED;
            fprintf(out, "  %-5d  %s%-9d%s  %-5d  %s%-9d%s  %-5d  %s%-9d%s  %-5d\n",
                    i,
                    a0 ? COL_RESET : bc, a0/3, COL_RESET, a0%3,
                    a1 ? COL_RESET : bc, a1/3, COL_RESET, a1%3,
                    a2 ? COL_RESET : bc, a2/3, COL_RESET, a2%3);
        }
        print_truncation_notice(out, n, m->nt);

        /* Compte les triangles de bord */
        int nbord = 0;
        for (int i = 1; i <= m->nt; i++) {
            for (int j = 0; j < 3; j++)
                if (m->adjt[3*(i-1)+1+j] == 0) nbord++;
        }
        fprintf(out, "\n  %sArêtes de bord (voisin = 0) : %s%d%s\n",
                COL_GREY, COL_RED, nbord, COL_RESET);
    } else {
        fprintf(out, "  %sadjt : non disponible (hashTria pas encore appelé)%s\n",
                COL_GREY, COL_RESET);
    }

    kv(out, "adja (tétraèdres)", "%s%s%s",
       m->adja ? COL_GREEN : COL_GREY,
       m->adja ? "présent" : "NULL (normal en 2D)", COL_RESET);
    kv(out, "adjq (quadrangles)", "%s%s%s",
       m->adjq ? COL_GREEN : COL_GREY,
       m->adjq ? "présent" : "NULL", COL_RESET);
}

/* ===================================================================
 * MMG5_Sol
 * =================================================================== */
void print_sol(MMG5_pSol sol, const char *label, FILE *out, int max_rows) {
    box_title(out, label, COL_MAGENTA);

    if (!sol) {
        fprintf(out, "  %s(pointeur NULL)%s\n", COL_GREY, COL_RESET);
        return;
    }

    kv(out, "np",    "%d", sol->np);
    kv(out, "npmax", "%d", sol->npmax);
    kv(out, "npi",   "%d", sol->npi);
    kv(out, "dim",   "%d", sol->dim);
    kv(out, "ver",   "%d", sol->ver);
    kv(out, "size",  "%d  %s(valeurs par entité)%s",
       sol->size, COL_GREY, COL_RESET);

    const char *type_str =
        (sol->type == MMG5_Scalar) ? "Scalar" :
        (sol->type == MMG5_Vector) ? "Vector" :
        (sol->type == MMG5_Tensor) ? "Tensor" : "inconnu";
    kv(out, "type",     "%s%s%s", COL_WHITE, type_str, COL_RESET);
    kv(out, "entities", "%d", sol->entities);
    kv(out, "umin",     "%g", sol->umin);
    kv(out, "umax",     "%g", sol->umax);
    kv(out, "namein",   "%s", sol->namein  ? sol->namein  : "(NULL)");
    kv(out, "nameout",  "%s", sol->nameout ? sol->nameout : "(NULL)");

    if (sol->m && sol->np > 0) {
        section_title(out, "Valeurs de la solution m[]");
        fprintf(out, "  %s%-6s", COL_BOLD, "idx");
        for (int s = 0; s < sol->size; s++)
            fprintf(out, "  %-15s", s == 0 ? "val[0]" : s == 1 ? "val[1]" : "val[2]");
        fprintf(out, "%s\n", COL_RESET);

        int n = display_count(sol->np, max_rows);
        for (int i = 1; i <= n; i++) {
            fprintf(out, "  %-6d", i);
            for (int s = 0; s < sol->size; s++)
                fprintf(out, "  %s%-15.7g%s",
                        COL_WHITE, sol->m[sol->size*i + s], COL_RESET);
            fprintf(out, "\n");
        }
        print_truncation_notice(out, n, sol->np);
    } else {
        fprintf(out, "  %s(pas de valeurs — métrique non initialisée)%s\n",
                COL_GREY, COL_RESET);
    }
}

/* ===================================================================
 * AIDE
 * =================================================================== */
static void print_help(const char *prog) {
    printf("Usage : %s fichier.mesh [options]\n\n", prog);
    printf("Options de sélection (sans option = tout afficher) :\n");
    printf("  --all          Tout afficher (défaut)\n");
    printf("  --global       Champs globaux MMG5_Mesh\n");
    printf("  --info         Paramètres MMG5_Info\n");
    printf("  --stats        Statistiques\n");
    printf("  --points       MMG5_Point\n");
    printf("  --xpoints      MMG5_xPoint\n");
    printf("  --triangles    MMG5_Tria\n");
    printf("  --edges        MMG5_Edge\n");
    printf("  --htab         Hash table\n");
    printf("  --adjacency    Tables d'adjacence\n");
    printf("  --sol          Métrique / solution\n");
    printf("\nOptions de formatage :\n");
    printf("  --max N        Limite N lignes par tableau\n");
    printf("  --no-color     Désactive les couleurs ANSI\n");
    printf("\nExemples :\n");
    printf("  %s mesh.mesh\n", prog);
    printf("  %s mesh.mesh --points --triangles --max 20\n", prog);
    printf("  %s mesh.mesh --no-color > rapport.txt\n", prog);
    printf("  %s mesh.mesh > rapport.txt  (couleurs auto-désactivées)\n", prog);
}

/* ===================================================================
 * MAIN
 * =================================================================== */
int main(int argc, char *argv[]) {

    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    /* Si la sortie est redirigée vers un fichier → pas de couleurs */
    if (!isatty(STDOUT_FILENO)) USE_COLOR = 0;

    Options opt;
    options_default(&opt);
    int explicit_section = 0;

    /* Parse des arguments (à partir du 2ème) */
    for (int i = 2; i < argc; i++) {
        if      (strcmp(argv[i], "--all")       == 0) { options_default(&opt); explicit_section = 0; }
        else if (strcmp(argv[i], "--no-color")  == 0) { USE_COLOR = 0; }
        else if (strcmp(argv[i], "--max")       == 0 && i+1 < argc) {
            opt.max_rows = atoi(argv[++i]);
        }
        else {
            /* Première section explicite → tout éteindre sauf ce qui est demandé */
            if (!explicit_section) { options_clear(&opt); explicit_section = 1; }
            if      (strcmp(argv[i], "--global")    == 0) opt.show_global    = 1;
            else if (strcmp(argv[i], "--info")      == 0) opt.show_info      = 1;
            else if (strcmp(argv[i], "--stats")     == 0) opt.show_stats     = 1;
            else if (strcmp(argv[i], "--points")    == 0) opt.show_points    = 1;
            else if (strcmp(argv[i], "--xpoints")   == 0) opt.show_xpoints   = 1;
            else if (strcmp(argv[i], "--triangles") == 0) opt.show_triangles = 1;
            else if (strcmp(argv[i], "--edges")     == 0) opt.show_edges     = 1;
            else if (strcmp(argv[i], "--htab")      == 0) opt.show_htab      = 1;
            else if (strcmp(argv[i], "--adjacency") == 0) opt.show_adjacency = 1;
            else if (strcmp(argv[i], "--sol")       == 0) opt.show_sol       = 1;
            else { fprintf(stderr, "Option inconnue : %s\n", argv[i]); return EXIT_FAILURE; }
        }
    }

    FILE *out = stdout;

    MMG5_pMesh mmgMesh = NULL;
    MMG5_pSol  mmgSol  = NULL;

    if (MMG2D_Init_mesh(MMG5_ARG_start,
                        MMG5_ARG_ppMesh, &mmgMesh,
                        MMG5_ARG_ppMet,  &mmgSol,
                        MMG5_ARG_end) != 1) {
        fprintf(stderr, "Erreur MMG2D_Init_mesh\n");
        return EXIT_FAILURE;
    }

    if (MMG2D_loadMesh(mmgMesh, argv[1]) != 1) {
        fprintf(stderr, "Erreur chargement '%s'\n", argv[1]);
        MMG2D_Free_all(MMG5_ARG_start,
                       MMG5_ARG_ppMesh, &mmgMesh,
                       MMG5_ARG_ppMet, &mmgSol,
                       MMG5_ARG_end);
        return EXIT_FAILURE;
    }

    if (opt.show_adjacency)
        if (MMG2D_hashTria(mmgMesh) != 1)
            fprintf(stderr, "[avertissement] hashTria a échoué\n");

    /* En-tête du rapport */
    fprintf(out, "%s", COL_CYAN);
    fprintf(out, "╔══════════════════════════════════════════════════════════════════════╗\n");
    fprintf(out, "║  RAPPORT MMG2D                                                       ║\n");
    fprintf(out, "║  Fichier : %-58s║\n", argv[1]);
    if (opt.max_rows > 0)
        fprintf(out, "║  Affichage limité à %d lignes par tableau%-29s║\n",
                opt.max_rows, "");
    fprintf(out, "╚══════════════════════════════════════════════════════════════════════╝\n");
    fprintf(out, "%s\n", COL_RESET);

    if (opt.show_global)    print_mesh_global(mmgMesh,                           out);
    if (opt.show_info)      print_info       (mmgMesh,                           out);
    if (opt.show_stats)     print_stats      (mmgMesh,                           out);
    if (opt.show_points)    print_points     (mmgMesh,                           out, opt.max_rows);
    if (opt.show_xpoints)   print_xpoints    (mmgMesh,                           out);
    if (opt.show_triangles) print_triangles  (mmgMesh,                           out, opt.max_rows);
    if (opt.show_edges)     print_edges      (mmgMesh,                           out, opt.max_rows);
    if (opt.show_htab)      print_htab       (mmgMesh,                           out);
    if (opt.show_adjacency) print_adjacency  (mmgMesh,                           out, opt.max_rows);
    if (opt.show_sol)       print_sol        (mmgSol,  "MMG5_Sol — métrique",    out, opt.max_rows);

    fprintf(out, "\n%s╔══ FIN DU RAPPORT ════════════════════════════════════════════════════╗%s\n\n",
            COL_CYAN, COL_RESET);

    MMG2D_Free_all(MMG5_ARG_start,
                   MMG5_ARG_ppMesh, &mmgMesh,
                   MMG5_ARG_ppMet,  &mmgSol,
                   MMG5_ARG_end);
    return EXIT_SUCCESS;
}