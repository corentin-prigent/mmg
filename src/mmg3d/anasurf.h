/**
 * \file anasurf.h
 * \brief Déclarations publiques du module anasurf.
 */

#ifndef ANASURF_H
#define ANASURF_H

#include "libmmgtypes.h"
#include <math.h>

#define SPHERE_CX  0.5
#define SPHERE_CY  0.5
#define SPHERE_CZ  0.5
#define SPHERE_R   0.25
#define REF_SPHERE 7
#define SPHERE_TOL 0.05

void   MMG3D_anasurf_projectOnSphere(double xyz[3], double o[3], double no[3]);
int MMG3D_anasurf_isOnSphere(double c[3]);

#endif /* ANASURF_H */