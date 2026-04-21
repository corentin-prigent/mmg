/**
 * \file anasurf.c
 * \brief Projection analytique de surface pour le mode -anasurf de MMG3D.
 *
 * Ce fichier regroupe les fonctions de projection sur des surfaces analytiques
 * (sphère pour l'instant, CAO à terme) utilisées en remplacement de bezierInt.
 */

#include <math.h>
#include "mmgcommon_private.h"
#include "libmmg3d_private.h"
#include "anasurf.h"


 #include "libmmg3d_private.h"

/**
 * \brief Projette un point sur la sphère analytique et calcule la normale.
 *
 * Prend un point xyz[] (le milieu d'une arête de surface),
 * le projette sur la sphère de centre (SPHERE_CX, SPHERE_CY, SPHERE_CZ)
 * et de rayon SPHERE_R. Retourne la position projetée dans o[] et la normale
 * unitaire rentrante dans no[].
 *
 *
 * \param[in]  xyz  coordonnées du point à projeter
 * \param[out] o    coordonnées du point projeté sur la sphère
 * \param[out] no   normale unitaire rentrante à la sphère en o
 */

void MMG3D_anasurf_projectOnSphere(double xyz[3], double o[3], double no[3]) {
  double dx = xyz[0] - SPHERE_CX;
  double dy = xyz[1] - SPHERE_CY;
  double dz = xyz[2] - SPHERE_CZ;
  double dist = sqrt(dx*dx + dy*dy + dz*dz);

  double inv = 1.0 / dist;
  o[0]  = SPHERE_CX + SPHERE_R * dx * inv;
  o[1]  = SPHERE_CY + SPHERE_R * dy * inv;
  o[2]  = SPHERE_CZ + SPHERE_R * dz * inv;

  /* Normale entrante (du centre vers la surface) */
  no[0] = -dx * inv;
  no[1] = -dy * inv;
  no[2] = -dz * inv;
}


/**
 * \brief Teste si un point est sur la sphère analytique.
 *
 * \param[in] c coordonnées du point
 *
 * \return 1 si le point est sur la sphère (à la tolérance près), 0 sinon.
 */
int MMG3D_anasurf_isOnSphere(double c[3]) {
  double dx   = c[0] - SPHERE_CX;
  double dy   = c[1] - SPHERE_CY;
  double dz   = c[2] - SPHERE_CZ;
  double dist = sqrt(dx*dx + dy*dy + dz*dz);

  if ( fabs(dist - SPHERE_R) < 1.0e-4 ) {
    return 1;
  }
  return 0;
}