/**
 * \file anasurf.c
 * \brief Projection analytique de surface pour le mode -anasurf de MMG3D.
 */

#include <math.h>
#include <stdio.h>
#include "mmgcommon_private.h"
#include "libmmg3d_private.h"
#include "anasurf.h"
#include "egads.h"

/* ------------------------------------------------------------------ */
/* Etat interne EGADS (initialise une seule fois)                      */
/* ------------------------------------------------------------------ */
static ego  g_context = NULL;
static ego *g_faces   = NULL;
static int  g_nfaces  = 0;
static int  g_ready   = 0;

/* Chemin du fichier .egadslite — sera parametre plus tard */
#define EGADS_FILE "/home/eelleb/Desktop/test_egads/cube_sphere.egadslite"

/**
 * \brief Initialise le contexte EGADSlite (appel unique au debut).
 * \return 1 si succes, 0 si echec.
 */
int MMG3D_anasurf_initEGADS(void) {
  if ( g_ready ) return 1;

  ego model, geom, *bodies;
  int oclass, mtype, nbodies, *senses;

  if ( EG_open(&g_context) != EGADS_SUCCESS ) {
    fprintf(stderr, "[EGADS] EG_open failed\n");
    return 0;
  }

  if ( EG_loadModel(g_context, 0, EGADS_FILE, &model) != EGADS_SUCCESS ) {
    fprintf(stderr, "[EGADS] EG_loadModel failed : %s\n", EGADS_FILE);
    return 0;
  }

  EG_getTopology(model, &geom, &oclass, &mtype,
                 NULL, &nbodies, &bodies, &senses);
  if (nbodies < mtype) nbodies = mtype;

  if ( nbodies < 1 ) {
    fprintf(stderr, "[EGADS] Pas de body dans le modele\n");
    return 0;
  }

  EG_getBodyTopos(bodies[0], NULL, FACE, &g_nfaces, &g_faces);
  printf("[EGADS] Modele charge : %d faces\n", g_nfaces);

  g_ready = 1;
  return 1;
}

/**
 * \brief Libere les ressources EGADSlite.
 */
void MMG3D_anasurf_closeEGADS(void) {
  if ( g_faces )   { EG_free(g_faces); g_faces = NULL; }
  if ( g_context ) { EG_close(g_context); g_context = NULL; }
  g_ready  = 0;
  g_nfaces = 0;
  printf("[EGADS] Close Egads\n");
}

/**
 * \brief Projette xyz sur la face la plus proche du modele EGADS.
 *
 * \param[in]  xyz  point a projeter
 * \param[out] o    point projete sur la surface
 * \param[out] no   normale unitaire entrante en o
 * \return index de la face la plus proche (0-base), -1 si echec
 */
int MMG3D_anasurf_projectEGADS(double xyz[3], double o[3], double no[3]) {
  if ( !g_ready ) {
    fprintf(stderr, "[EGADS] Non initialise — appeler MMG3D_anasurf_initEGADS\n");
    return -1;
  }

  double best_dist   = 1e30;
  int    best_index  = -1;
  double best_result[3], best_param[2];

  /* 1. Trouver la face la plus proche */
  for (int i = 0; i < g_nfaces; i++) {
    double param[2], result[3];
    int stat = EG_invEvaluate(g_faces[i], xyz, param, result);
    if (stat != EGADS_SUCCESS && stat != EGADS_OUTSIDE) continue;

    double dx   = xyz[0] - result[0];
    double dy   = xyz[1] - result[1];
    double dz   = xyz[2] - result[2];
    double dist = sqrt(dx*dx + dy*dy + dz*dz);

    if (dist < best_dist) {
      best_dist      = dist;
      best_index     = i;
      best_result[0] = result[0];
      best_result[1] = result[1];
      best_result[2] = result[2];
      best_param[0]  = param[0];
      best_param[1]  = param[1];
    }
  }

  if (best_index < 0) return -1;

  /* 2. Point projete */
  o[0] = best_result[0];
  o[1] = best_result[1];
  o[2] = best_result[2];

  /* 3. Normale = Su x Sv */
  double data[18];
  EG_evaluate(g_faces[best_index], best_param, data);

  no[0] = data[4]*data[8] - data[5]*data[7];
  no[1] = data[5]*data[6] - data[3]*data[8];
  no[2] = data[3]*data[7] - data[4]*data[6];

  double norm = sqrt(no[0]*no[0] + no[1]*no[1] + no[2]*no[2]);
  if (norm < 1e-14) {
    no[0] = 0.0; no[1] = 0.0; no[2] = 1.0;
    return best_index;
  }

  no[0] /= norm;
  no[1] /= norm;
  no[2] /= norm;

  /* 4. Normale entrante : si dot > 0 la normale pointe vers xyz, on inverse */
  double dot = (xyz[0]-o[0])*no[0]
             + (xyz[1]-o[1])*no[1]
             + (xyz[2]-o[2])*no[2];
  if (dot > 0.0) {
    no[0] = -no[0];
    no[1] = -no[1];
    no[2] = -no[2];
  }

  return best_index;
}

/* ------------------------------------------------------------------ */
/* Fonctions analytiques existantes                                    */
/* ------------------------------------------------------------------ */

void MMG3D_anasurf_projectOnSphere(double xyz[3], double o[3], double no[3]) {
  double dx = xyz[0] - SPHERE_CX;
  double dy = xyz[1] - SPHERE_CY;
  double dz = xyz[2] - SPHERE_CZ;
  double dist = sqrt(dx*dx + dy*dy + dz*dz);
  double inv = 1.0 / dist;
  o[0]  = SPHERE_CX + SPHERE_R * dx * inv;
  o[1]  = SPHERE_CY + SPHERE_R * dy * inv;
  o[2]  = SPHERE_CZ + SPHERE_R * dz * inv;
  no[0] = -dx * inv;
  no[1] = -dy * inv;
  no[2] = -dz * inv;
}

int MMG3D_anasurf_isOnSphere(double c[3]) {
  double dx   = c[0] - SPHERE_CX;
  double dy   = c[1] - SPHERE_CY;
  double dz   = c[2] - SPHERE_CZ;
  double dist = sqrt(dx*dx + dy*dy + dz*dz);
  return ( fabs(dist - SPHERE_R) < 1.0e-4 );
}