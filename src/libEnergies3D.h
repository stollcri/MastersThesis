#ifndef LIBENERGIES_C
#define LIBENERGIES_C

/**
 * libenergies -- Calculate image "energies"
 *
 * Copyright (c) 2015, Christopher Stoll
 * All rights reserved.
 */

#include "pixel.h"

extern int getPixelEnergySimple(struct pixel*, int, int, int, int);
extern int getPixelEnergySobel(int*, int, int, int, int);
extern int getPixelEnergyLaplacian(struct pixel*, int, int, int);
extern int getPixelGaussian(struct pixel*, int, int, int, int, int);
extern int getPixelEnergyDoG(int, int);
extern int getPixelEnergyStoll(struct pixel*, int, int, int, int);

#endif
