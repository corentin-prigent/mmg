#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <unistd.h>

#include "libmmg2d.h"
#include "libmmg2d_private.h"

/* =============================================================================
** main5.c — Lance MMG2D avec les options standard et affiche les structures
**
** Usage :
**   ./mon_test5 -in cercle.mesh -hsiz 0.05
**   ./mon_test5 -in carre.mesh  -sol carre.sol -ls 0.0
**   ./mon_test5 -in cercle.mesh -hmin 0.01 -hmax 0.2 -hgrad 1.3 -hausd 0.005
**   ./mon_test5 -in cercle.mesh -hsiz 0.1 -noinsert -optim
**   ./mon_test5 -in cercle.mesh -hsiz 0.05 -out result.mesh --max 20
**
** Options MMG supportées :
**   -in   <f>    fichier maillage entrée
**   -sol  <f>    fichier solution / level-set
**   -out  <f>    fichier maillage sortie  (défaut: /tmp/result.mesh)
**   -hsiz <v>    taille uniforme cible
**   -hmin <v>    taille minimale des arêtes
**   -hmax <v>    taille maximale des arêtes
**   -hgrad <v>   taux de gradation
**   -hausd <v>   distance de Hausdorff
**   -ls  [v]     mode level-set  (isovaleur, défaut 0.0)
**   -noinsert    interdit l'insertion/suppression de points
**   -noswap      interdit le basculement d'arêtes
**   -nomove      interdit le déplacement des nœuds
**   -nosurf      protège la frontière
**   -optim       optimisation géométrique pure
**   -nofem       lève les contraintes éléments finis
**   -ar  <v>     seuil de détection des angles vifs (degrés)
**   -A           active l'anisotropie automatique
**   -nsd <n>     sauvegarde uniquement le sous-domaine n
**
** Options d'affichage :
**   --max <N>    limite N lignes par tableau
**   --before     affiche uniquement avant
**   --after      affiche uniquement après
**   --both       affiche avant ET après + diff (défaut)
**   --no-color   désactive les couleurs ANSI
** =============================================================================
*/

/* ===================================================================
 * COULEURS
 * =================================================================== */
static int USE_COLOR = 1;
static const char *CR = "";   /* reset  */
static const char *CB = "";   /* bold   */
static const char *CC = "";   /* cyan   */
static const char *CY = "";   /* yellow */
static const char *CG = "";   /* green  */
static const char *CRD= "";   /* red    */
static const char *CM = "";   /* magenta*/
static const char *CBL= "";   /* blue   */
static const char *CGR= "";   /* grey   */
static const char *CW = "";   /* white  */

static void init_colors(void) {
    if (!USE_COLOR) return;
    CR  = "\033[0m";
    CB  = "\033[1m";
    CC  = "\033[1;36m";
    CY  = "\033[1;33m";
    CG  = "\033[0;32m";
    CRD = "\033[0;31m";
    CM  = "\033[0;35m";
    CBL = "\033[0;34m";
    CGR = "\033[0;90m";
    CW  = "\033[1;37m";
}

/* ===================================================================
 * CONFIGURATION RUNTIME
 * =================================================================== */
typedef struct {
    /* Fichiers */
    char in_file[512];
    char sol_file[512];
    char out_file[512];

    /* Paramètres MMG (double) */
    double hsiz;
    double hmin;
    double hmax;
    double hgrad;
    double hausd;
    double ls_val;
    double ar;

    /* Flags MMG (int) */
    int do_ls;       /* -ls activé         */
    int noinsert;
    int noswap;
    int nomove;
    int nosurf;
    int optim;
    int nofem;
    int aniso;       /* -A                 */
    int nsd;         /* -nsd n             */

    /* Options affichage */
    int max_rows;
    int show_before;
    int show_after;
} Config;

static void config_init(Config *c) {
    memset(c, 0, sizeof(*c));
    strcpy(c->out_file, "/tmp/result.mesh");
    c->hsiz     = -1.0;
    c->hmin     = -1.0;
    c->hmax     = -1.0;
    c->hgrad    = -1.0;
    c->hausd    = -1.0;
    c->ls_val   =  0.0;
    c->ar       = -1.0;
    c->max_rows = -1;
    c->show_before = 1;
    c->show_after  = 1;
}

/* ===================================================================
 * PARSING DES ARGUMENTS
 * =================================================================== */
static void print_usage(const char *prog) {
    printf("Usage : %s -in fichier.mesh [options]\n\n", prog);
    printf("Fichiers :\n");
    printf("  -in  <f>      maillage entree\n");
    printf("  -sol <f>      solution / level-set\n");
    printf("  -out <f>      maillage sortie (defaut: /tmp/result.mesh)\n");
    printf("\nTailles :\n");
    printf("  -hsiz  <v>    taille uniforme cible\n");
    printf("  -hmin  <v>    taille minimale\n");
    printf("  -hmax  <v>    taille maximale\n");
    printf("  -hgrad <v>    taux de gradation\n");
    printf("  -hausd <v>    distance de Hausdorff\n");
    printf("\nModes :\n");
    printf("  -ls [v]       mode level-set (isovaleur, defaut 0.0)\n");
    printf("  -A            anisotropie automatique\n");
    printf("  -nsd <n>      sauvegarde sous-domaine n uniquement\n");
    printf("\nOperateurs :\n");
    printf("  -noinsert     interdit insertion/suppression de points\n");
    printf("  -noswap       interdit le basculement d'aretes\n");
    printf("  -nomove       interdit le deplacement des noeuds\n");
    printf("  -nosurf       protege la frontiere\n");
    printf("  -optim        optimisation geometrique pure\n");
    printf("  -nofem        leve les contraintes EF\n");
    printf("  -ar <v>       seuil detection angles vifs (degres)\n");
    printf("\nAffichage :\n");
    printf("  --max <N>     limite N lignes par tableau\n");
    printf("  --before      affiche uniquement avant\n");
    printf("  --after       affiche uniquement apres\n");
    printf("  --both        avant + apres + diff (defaut)\n");
    printf("  --no-color    desactive les couleurs ANSI\n");
    printf("\nExemples :\n");
    printf("  %s -in cercle.mesh -hsiz 0.05\n", prog);
    printf("  %s -in carre.mesh -sol carre.sol -ls 0.0\n", prog);
    printf("  %s -in cercle.mesh -hmin 0.01 -hmax 0.2 -noinsert --max 15\n", prog);
}

static int parse_args(int argc, char *argv[], Config *c) {
    if (argc < 2) { print_usage(argv[0]); return 0; }

    /* Premier argument sans tiret = fichier d'entrée */
    if (argv[1][0] != '-') {
        strncpy(c->in_file, argv[1], sizeof(c->in_file)-1);
    }

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-in")  && i+1<argc)
            strncpy(c->in_file,  argv[++i], sizeof(c->in_file)-1);
        else if (!strcmp(argv[i], "-sol") && i+1<argc)
            strncpy(c->sol_file, argv[++i], sizeof(c->sol_file)-1);
        else if (!strcmp(argv[i], "-out") && i+1<argc)
            strncpy(c->out_file, argv[++i], sizeof(c->out_file)-1);

        else if (!strcmp(argv[i], "-hsiz")  && i+1<argc) c->hsiz  = atof(argv[++i]);
        else if (!strcmp(argv[i], "-hmin")  && i+1<argc) c->hmin  = atof(argv[++i]);
        else if (!strcmp(argv[i], "-hmax")  && i+1<argc) c->hmax  = atof(argv[++i]);
        else if (!strcmp(argv[i], "-hgrad") && i+1<argc) c->hgrad = atof(argv[++i]);
        else if (!strcmp(argv[i], "-hausd") && i+1<argc) c->hausd = atof(argv[++i]);
        else if (!strcmp(argv[i], "-ar")    && i+1<argc) c->ar    = atof(argv[++i]);
        else if (!strcmp(argv[i], "-nsd")   && i+1<argc) c->nsd   = atoi(argv[++i]);

        else if (!strcmp(argv[i], "-ls")) {
            c->do_ls = 1;
            if (i+1 < argc && argv[i+1][0] != '-')
                c->ls_val = atof(argv[++i]);
        }

        else if (!strcmp(argv[i], "-noinsert")) c->noinsert = 1;
        else if (!strcmp(argv[i], "-noswap"))   c->noswap   = 1;
        else if (!strcmp(argv[i], "-nomove"))   c->nomove   = 1;
        else if (!strcmp(argv[i], "-nosurf"))   c->nosurf   = 1;
        else if (!strcmp(argv[i], "-optim"))    c->optim    = 1;
        else if (!strcmp(argv[i], "-nofem"))    c->nofem    = 1;
        else if (!strcmp(argv[i], "-A"))        c->aniso    = 1;

        else if (!strcmp(argv[i], "--max") && i+1<argc)
            c->max_rows = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--before"))
            { c->show_before=1; c->show_after=0; }
        else if (!strcmp(argv[i], "--after"))
            { c->show_before=0; c->show_after=1; }
        else if (!strcmp(argv[i], "--both"))
            { c->show_before=1; c->show_after=1; }
        else if (!strcmp(argv[i], "--no-color"))
            USE_COLOR = 0;
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
            { print_usage(argv[0]); return 0; }
        /* ignore les arguments inconnus silencieusement */
    }

    if (!c->in_file[0]) {
        fprintf(stderr, "Erreur : fichier d'entree manquant (-in)\n");
        return 0;
    }
    return 1;
}

/* ===================================================================
 * APPLICATION DES PARAMÈTRES → structures MMG
 *
 * Chaque Set_dparameter/Set_iparameter écrit dans mesh->info.*
 * C'est l'équivalent de MMG2D_parsar() dans mmg2d.c
 * =================================================================== */
static void apply_config(MMG5_pMesh mesh, MMG5_pSol met, const Config *c) {
    /* Tailles */
    if (c->hsiz  > 0) MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_hsiz,   c->hsiz);
    if (c->hmin  > 0) MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_hmin,   c->hmin);
    if (c->hmax  > 0) MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_hmax,   c->hmax);
    if (c->hgrad > 0) MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_hgrad,  c->hgrad);
    if (c->hausd > 0) MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_hausd,  c->hausd);
    if (c->ar    > 0) MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_angleDetection, c->ar);

    /* Modes */
    if (c->do_ls) {
        MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_iso, 1);
        MMG2D_Set_dparameter(mesh,met,MMG2D_DPARAM_ls,  c->ls_val);
    }
    if (c->aniso)    mesh->info.ani = 1;
    if (c->nsd > 0)  MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_numsubdomain, c->nsd);

    /* Opérateurs */
    if (c->noinsert) MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_noinsert, 1);
    if (c->noswap)   MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_noswap,   1);
    if (c->nomove)   MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_nomove,   1);
    if (c->nosurf)   MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_nosurf,   1);
    if (c->optim)    MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_optim,    1);
    if (c->nofem)    MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_nofem,    1);

    /* Verbosité : on laisse MMG muet pour ne pas polluer notre affichage */
    MMG2D_Set_iparameter(mesh,met,MMG2D_IPARAM_verbose, -1);
}

/* ===================================================================
 * FONCTIONS D'AFFICHAGE (reprises de main2.c)
 * =================================================================== */
static void decode_tag(uint16_t tag, char *buf, int bufsize) {
    buf[0] = '\0';
    if (tag & (1<<0)) strncat(buf,"BDY ",   bufsize-strlen(buf)-1);
    if (tag & (1<<1)) strncat(buf,"REF ",   bufsize-strlen(buf)-1);
    if (tag & (1<<2)) strncat(buf,"GEO ",   bufsize-strlen(buf)-1);
    if (tag & (1<<3)) strncat(buf,"NOM ",   bufsize-strlen(buf)-1);
    if (tag & (1<<4)) strncat(buf,"REQ ",   bufsize-strlen(buf)-1);
    if (tag & (1<<5)) strncat(buf,"NOSURF ",bufsize-strlen(buf)-1);
    if (tag & (1<<6)) strncat(buf,"OPNBDY ",bufsize-strlen(buf)-1);
    if (tag == 0)     strncat(buf,"-",      bufsize-strlen(buf)-1);
}

static void box(FILE *out, const char *title, const char *col) {
    int w=70, tlen=(int)strlen(title);
    int pad=(w-2-tlen)/2, right=w-2-pad-tlen;
    fprintf(out,"\n%s",col);
    for(int i=0;i<w;i++) fprintf(out,"=");
    fprintf(out,"\n|");
    for(int i=0;i<pad;i++) fprintf(out," ");
    fprintf(out,"%s%s%s%s",CW,title,CR,col);
    for(int i=0;i<right;i++) fprintf(out," ");
    fprintf(out,"|\n");
    for(int i=0;i<w;i++) fprintf(out,"=");
    fprintf(out,"%s\n",CR);
}

static void sec(FILE *out, const char *t) {
    fprintf(out,"\n%s--- %s%s%s ---\n",CY,CB,t,CR);
}

static void kv(FILE *out, const char *k, const char *fmt, ...) {
    fprintf(out,"  %s%-28s%s ",CC,k,CR);
    va_list ap; va_start(ap,fmt); vfprintf(out,fmt,ap); va_end(ap);
    fprintf(out,"\n");
}

static void thdr(FILE *out, const char **cols, const int *w, int n) {
    fprintf(out,"\n  %s",CB);
    for(int i=0;i<n;i++) fprintf(out,"%-*s  ",w[i],cols[i]);
    fprintf(out,"%s\n  %s",CR,CGR);
    for(int i=0;i<n;i++){for(int j=0;j<w[i];j++) fprintf(out,"-"); fprintf(out,"  ");}
    fprintf(out,"%s\n",CR);
}

static void print_tag_col(FILE *out, const char *t) {
    if      (strstr(t,"BDY")||strstr(t,"GEO")) fprintf(out,"%s%-16s%s",CG, t,CR);
    else if (strstr(t,"REQ"))                  fprintf(out,"%s%-16s%s",CRD,t,CR);
    else if (strstr(t,"REF"))                  fprintf(out,"%s%-16s%s",CM, t,CR);
    else                                       fprintf(out,"%s%-16s%s",CGR,t,CR);
}

static int dcnt(int total, int max) {
    return (max<0||max>=total) ? total : max;
}

/* --- Résumé compact --- */
static void print_summary(MMG5_pMesh m, FILE *out) {
    fprintf(out,"  %snp%s=%-6d  %snt%s=%-6d  %sna%s=%-6d  "
                "memCur=%zu Ko\n",
            CC,CR, m->np,
            CC,CR, m->nt,
            CC,CR, m->na,
            m->memCur/1024);
}

/* --- Paramètres info --- */
static void print_info(MMG5_pMesh m, FILE *out) {
    box(out,"MMG5_Info — parametres appliques",CBL);
    sec(out,"Tailles");
    kv(out,"hmin",   "%g", m->info.hmin);
    kv(out,"hmax",   "%g", m->info.hmax);
    kv(out,"hsiz",   "%g", m->info.hsiz);
    kv(out,"hgrad",  "%g", m->info.hgrad);
    kv(out,"hausd",  "%g", m->info.hausd);
    kv(out,"dhd",    "%g", m->info.dhd);
    sec(out,"Modes actifs");
    kv(out,"iso (level-set)",  "%d", m->info.iso);
    kv(out,"ls (isovaleur)",   "%g", m->info.ls);
    kv(out,"lag",              "%d", m->info.lag);
    kv(out,"optim",            "%d", m->info.optim);
    kv(out,"ani",              "%d", m->info.ani);
    sec(out,"Operateurs");
    kv(out,"noinsert",  "%d  %s", m->info.noinsert,
       m->info.noinsert ? "(insertion OFF)" : "(insertion ON)");
    kv(out,"noswap",    "%d  %s", m->info.noswap,
       m->info.noswap   ? "(swap OFF)"      : "(swap ON)");
    kv(out,"nomove",    "%d  %s", m->info.nomove,
       m->info.nomove   ? "(move OFF)"      : "(move ON)");
    kv(out,"nosurf",    "%d  %s", m->info.nosurf,
       m->info.nosurf   ? "(surface OFF)"   : "(surface ON)");
}

/* --- Points --- */
static void print_points(MMG5_pMesh m, FILE *out, int max) {
    box(out,"MMG5_Point — sommets",CG);
    fprintf(out,"  np = %s%d%s\n",CW,m->np,CR);
    const char *c[]={"idx","x","y","ref","xp","tag"};
    const int   w[]={6,    14,  14,  5,   4,   16};
    thdr(out,c,w,6);
    int n=dcnt(m->np,max);
    for(int i=1;i<=n;i++){
        MMG5_pPoint p=&m->point[i];
        char tb[128]; decode_tag(p->tag,tb,sizeof(tb));
        fprintf(out,"  %-6d  %-14.7f %-14.7f  %-5d %-4d  ",
                i,p->c[0],p->c[1],p->ref,p->xp);
        print_tag_col(out,tb);
        fprintf(out,"\n");
    }
    if(n<m->np)
        fprintf(out,"  %s[... %d/%d affiches]%s\n",CGR,n,m->np,CR);
}

/* --- Triangles --- */
static void print_triangles(MMG5_pMesh m, FILE *out, int max) {
    box(out,"MMG5_Tria — triangles",CY);
    fprintf(out,"  nt = %s%d%s\n",CW,m->nt,CR);
    const char *c[]={"idx","v0","v1","v2","ref","qual","tag[0]","tag[1]","tag[2]"};
    const int   w[]={6,     6,   6,   6,   4,   10,    14,     14,     14};
    thdr(out,c,w,9);
    int n=dcnt(m->nt,max);
    for(int i=1;i<=n;i++){
        MMG5_pTria t=&m->tria[i];
        char t0[64],t1[64],t2[64];
        decode_tag(t->tag[0],t0,sizeof(t0));
        decode_tag(t->tag[1],t1,sizeof(t1));
        decode_tag(t->tag[2],t2,sizeof(t2));
        const char *qc=(t->qual>0.5)?CG:(t->qual>0.1)?CY:CRD;
        fprintf(out,"  %-6d  %-6d %-6d %-6d  %-4d  %s%-10.6f%s  ",
                i,t->v[0],t->v[1],t->v[2],t->ref,qc,t->qual,CR);
        print_tag_col(out,t0); fprintf(out,"  ");
        print_tag_col(out,t1); fprintf(out,"  ");
        print_tag_col(out,t2);
        fprintf(out,"\n");
    }
    if(n<m->nt)
        fprintf(out,"  %s[... %d/%d affiches]%s\n",CGR,n,m->nt,CR);
}

/* --- Arêtes --- */
static void print_edges(MMG5_pMesh m, FILE *out, int max) {
    box(out,"MMG5_Edge — aretes",CY);
    fprintf(out,"  na = %s%d%s\n",CW,m->na,CR);
    if(!m->edge||m->na==0){fprintf(out,"  (aucune)\n");return;}
    const char *c[]={"idx","a","b","ref","tag"};
    const int   w[]={6,    7,  7,  5,    16};
    thdr(out,c,w,5);
    int n=dcnt(m->na,max);
    for(int i=1;i<=n;i++){
        MMG5_pEdge e=&m->edge[i];
        char tb[128]; decode_tag(e->tag,tb,sizeof(tb));
        fprintf(out,"  %-6d  %-7d %-7d  %-5d  ",i,e->a,e->b,e->ref);
        print_tag_col(out,tb);
        fprintf(out,"\n");
    }
    if(n<m->na)
        fprintf(out,"  %s[... %d/%d affiches]%s\n",CGR,n,m->na,CR);
}

/* --- Statistiques qualité --- */
static void print_stats(MMG5_pMesh m, FILE *out) {
    box(out,"Statistiques",CM);
    if(m->nt>0&&m->tria){
        double qmin=1e30,qmax=0,qmoy=0;
        int ndeg=0,ngood=0;
        for(int i=1;i<=m->nt;i++){
            double q=m->tria[i].qual;
            if(q<qmin)qmin=q; if(q>qmax)qmax=q; qmoy+=q;
            if(q<0.01)ndeg++; if(q>0.5)ngood++;
        }
        qmoy/=m->nt;
        sec(out,"Qualite triangles");
        kv(out,"min",   "%.6f",qmin);
        kv(out,"max",   "%.6f",qmax);
        kv(out,"moy",   "%.6f",qmoy);
        kv(out,"bons q>0.5",    "%d/%d (%.1f%%)",ngood,m->nt,100.0*ngood/m->nt);
        kv(out,"deg.  q<0.01",  "%d/%d",         ndeg, m->nt);
        /* Histogramme */
        sec(out,"Histogramme qualite");
        int bins[10]={0};
        for(int i=1;i<=m->nt;i++){
            int b=(int)(m->tria[i].qual*10); if(b<0)b=0; if(b>9)b=9; bins[b]++;
        }
        for(int b=9;b>=0;b--){
            int bar=bins[b]*40/m->nt;
            const char *bc=(b>=7)?CG:(b>=4)?CY:CRD;
            fprintf(out,"  %s[%.1f-%.1f[%s  %s",CGR,b*.1,(b+1)*.1,CR,bc);
            for(int j=0;j<bar;j++) fprintf(out,"#");
            fprintf(out,"%s  %d\n",CR,bins[b]);
        }
    }
    if(m->np>0&&m->point){
        double xmin=1e30,xmax=-1e30,ymin=1e30,ymax=-1e30;
        for(int i=1;i<=m->np;i++){
            double x=m->point[i].c[0],y=m->point[i].c[1];
            if(x<xmin)xmin=x; if(x>xmax)xmax=x;
            if(y<ymin)ymin=y; if(y>ymax)ymax=y;
        }
        sec(out,"Bounding box");
        kv(out,"X","[%.7f, %.7f]  delta=%.7f",xmin,xmax,xmax-xmin);
        kv(out,"Y","[%.7f, %.7f]  delta=%.7f",ymin,ymax,ymax-ymin);
    }
}

/* --- Diff avant/après --- */
static void print_diff(
    int np_b, int nt_b, int na_b, double qmoy_b,
    int np_a, int nt_a, int na_a, double qmoy_a,
    FILE *out)
{
    box(out,"DIFFERENCES avant → apres",CC);
    fprintf(out,"  %s%-12s%s  %s%-10s%s  %s%-10s%s  %s%-10s%s\n",
            CB,"champ",CR, CB,"avant",CR, CB,"apres",CR, CB,"delta",CR);
    fprintf(out,"  %s%-12s%s  %-10s  %-10s  %-10s\n",
            CGR,"------------",CR,
            "----------","----------","----------");

    int dnp=np_a-np_b, dnt=nt_a-nt_b, dna=na_a-na_b;
    const char *pc=dnp>0?CG:(dnp<0?CRD:CGR);
    const char *tc=dnt>0?CG:(dnt<0?CRD:CGR);
    const char *ac=dna>0?CG:(dna<0?CRD:CGR);

    fprintf(out,"  %-12s  %-10d  %-10d  %s%+d%s\n","np",np_b,np_a,pc,dnp,CR);
    fprintf(out,"  %-12s  %-10d  %-10d  %s%+d%s\n","nt",nt_b,nt_a,tc,dnt,CR);
    fprintf(out,"  %-12s  %-10d  %-10d  %s%+d%s\n","na",na_b,na_a,ac,dna,CR);

    double dq=qmoy_a-qmoy_b;
    const char *qc=dq>0?CG:(dq<0?CRD:CGR);
    fprintf(out,"  %-12s  %-10.4f  %-10.4f  %s%+.4f%s\n",
            "qual.moy",qmoy_b,qmoy_a,qc,dq,CR);
}

/* --- Affichage complet d'un snapshot --- */
static void print_snapshot(MMG5_pMesh mesh, const char *label,
                           int max, FILE *out) {
    fprintf(out,"\n%s%s╔══ %s ══╗%s\n",CB,CC,label,CR);
    print_summary(mesh, out);
    print_info    (mesh, out);
    print_stats   (mesh, out);
    print_points  (mesh, out, max);
    print_triangles(mesh,out, max);
    print_edges   (mesh, out, max);
}

/* ===================================================================
 * CALCUL QUALITÉ MOYENNE (pour le diff)
 * =================================================================== */
static double qual_moy(MMG5_pMesh m) {
    if (m->nt==0||!m->tria) return 0.0;
    double s=0;
    for(int i=1;i<=m->nt;i++) s+=m->tria[i].qual;
    return s/m->nt;
}

/* ===================================================================
 * MAIN
 * =================================================================== */
int main(int argc, char *argv[]) {

    if (!isatty(STDOUT_FILENO)) USE_COLOR = 0;

    Config cfg;
    config_init(&cfg);

    /* --no-color peut être partout dans argv, on le détecte en premier */
    for (int i=1;i<argc;i++)
        if (!strcmp(argv[i],"--no-color")) USE_COLOR=0;

    init_colors();

    if (!parse_args(argc, argv, &cfg)) return EXIT_FAILURE;

    /* ----------------------------------------------------------------
     * 1. Initialisation MMG
     * ---------------------------------------------------------------- */
    MMG5_pMesh mesh = NULL;
    MMG5_pSol  met  = NULL;
    MMG5_pSol  ls   = NULL;

    if (MMG2D_Init_mesh(MMG5_ARG_start,
                        MMG5_ARG_ppMesh, &mesh,
                        MMG5_ARG_ppMet,  &met,
                        MMG5_ARG_ppLs,   &ls,
                        MMG5_ARG_end) != 1) {
        fprintf(stderr,"Erreur MMG2D_Init_mesh\n");
        return EXIT_FAILURE;
    }

    /* ----------------------------------------------------------------
     * 2. Chargement du maillage
     * ---------------------------------------------------------------- */
    if (MMG2D_loadMesh(mesh, cfg.in_file) != 1) {
        fprintf(stderr,"Erreur chargement '%s'\n", cfg.in_file);
        goto free_all;
    }

    /* Chargement solution/level-set si fournie */
    if (cfg.sol_file[0]) {
        MMG5_pSol target = cfg.do_ls ? ls : met;
        if (MMG2D_loadSol(mesh, target, cfg.sol_file) < 1) {
            fprintf(stderr,"Erreur chargement sol '%s'\n", cfg.sol_file);
            goto free_all;
        }
    }

    /* ----------------------------------------------------------------
     * 3. Application des paramètres → mesh->info.*
     * ---------------------------------------------------------------- */
    apply_config(mesh, met, &cfg);

    /* ----------------------------------------------------------------
     * 4. Snapshot AVANT
     * ---------------------------------------------------------------- */
    int    np_b = mesh->np, nt_b = mesh->nt, na_b = mesh->na;
    double qb   = qual_moy(mesh);

    if (cfg.show_before)
        print_snapshot(mesh, "ETAT INITIAL (apres chargement)", cfg.max_rows, stdout);

    /* ----------------------------------------------------------------
     * 5. Lancement MMG — même logique que mmg2d.c
     * ---------------------------------------------------------------- */
    fprintf(stdout,"\n%s+--- Lancement MMG2D ---%s\n",CC,CR);

    int ier;
    if (cfg.do_ls) {
        fprintf(stdout,"  Mode : %slevel-set%s  (isovaleur=%.4f)\n",
                CW, CR, cfg.ls_val);
        ier = MMG2D_mmg2dls(mesh, ls, met);
    }
    else if (!mesh->nt) {
        fprintf(stdout,"  Mode : %sgeneration%s  (nt=0, contour seulement)\n",
                CW, CR);
        ier = MMG2D_mmg2dmesh(mesh, met);
    }
    else {
        fprintf(stdout,"  Mode : %sremaillage adaptatif%s\n", CW, CR);
        ier = MMG2D_mmg2dlib(mesh, met);
    }

    /* Résultat */
    if (ier == MMG5_SUCCESS)
        fprintf(stdout,"  Resultat : %sSUCCESS%s\n", CG,  CR);
    else if (ier == MMG5_LOWFAILURE)
        fprintf(stdout,"  Resultat : %sLOWFAILURE%s (maillage conforme sauvegardable)\n", CY, CR);
    else
        fprintf(stdout,"  Resultat : %sSTRONGFAILURE%s\n", CRD, CR);

    if (ier == MMG5_STRONGFAILURE) goto free_all;

    /* ----------------------------------------------------------------
     * 6. Snapshot APRÈS
     * ---------------------------------------------------------------- */
    double qa = qual_moy(mesh);

    if (cfg.show_after)
        print_snapshot(mesh, "ETAT FINAL (apres remaillage)", cfg.max_rows, stdout);

    /* ----------------------------------------------------------------
     * 7. Diff
     * ---------------------------------------------------------------- */
    if (cfg.show_before && cfg.show_after)
        print_diff(np_b,nt_b,na_b,qb, mesh->np,mesh->nt,mesh->na,qa, stdout);

    /* ----------------------------------------------------------------
     * 8. Sauvegarde
     * ---------------------------------------------------------------- */
    MMG2D_saveMesh(mesh, cfg.out_file);
    if (met && met->np)
        MMG2D_saveSol(mesh, met, cfg.out_file);
    fprintf(stdout,"\n  Sauvegarde : %s%s%s\n", CW, cfg.out_file, CR);

free_all:
    MMG2D_Free_all(MMG5_ARG_start,
                   MMG5_ARG_ppMesh, &mesh,
                   MMG5_ARG_ppMet,  &met,
                   MMG5_ARG_ppLs,   &ls,
                   MMG5_ARG_end);
    return EXIT_SUCCESS;
}