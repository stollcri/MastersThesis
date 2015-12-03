/**
 * libSeabCarve.c
 *
 * Masters Thesis Work
 * Christopher Stoll, 2015 -- v3, major refactoring
 */

#ifndef LIBSEAMCARVE_C
#define LIBSEAMCARVE_C

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "pixel.h"
#include "window.h"
#include "libWrappers.c"
#include "libBinarization.c"
#include "libEnergies.c"
#include "libMinMax.c"
#include "libColorConv.c"

#define SEAM_TRACE_INCREMENT 16
#define THRESHHOLD_SOBEL 96
#define THRESHHOLD_USECOUNT 64
#define PI 3.14159265359

#define DIRECTION_VERTICAL 0
#define DIRECTION_HORIZONTAL 1

#ifdef PNG16BIT
#define PNG_MAX INT_MAX
#else
#define PNG_MAX 255
#endif

#define DEFAULT_CLIP_AREA_BOUND 1

/*
 * Trace all the seams
 * The least signifigant pixels will be traced multiple times and have a higher value (whiter)
 * The most signifigant pixels will not be traced at all and have a value of zero (black)
 */
static void findSeams(struct pixel *imageVector, struct window *imageWindow, int direction, int findAreas)
{
	if ((direction != DIRECTION_VERTICAL) && (direction != DIRECTION_HORIZONTAL)) {
		return;
	}

	int loopBeg = 0; // where the outer loop begins
	int loopEnd = 0; // where the outer loop ends
	int loopInc = 0; // the increment of the outer loop
	int loopInBeg = 0;
	int loopInEnd = 0;
	int loopInInc = 0;
	int seamLength = 0;

	int nextPixelR = 0; // next pixel to the right
	int nextPixelC = 0; // next pixel to the center
	int nextPixelL = 0; // next pixel to the left
	int currentMin = 0; // the minimum of nextPixelR, nextPixelC, and nextPixelL
	int countGoR = 0; // how many times the seam diverged upward
	int countGoL = 0; // how many times the seam diverged downward

	int nextPixelDistR = 0; // memory distance to the next pixel to the right
	int nextPixelDistC = 0; // memory distance to the next pixel to the center
	int nextPixelDistL = 0; // memory distance to the next pixel to the left

	// loop conditions depend upon the direction
	if (direction == DIRECTION_VERTICAL) {
		loopBeg = imageWindow->lastPixel - 1;
		loopEnd = imageWindow->lastPixel - 1 - imageWindow->xLength;
		loopInc = imageWindow->xStep * -1;

		// also set the next pixel distances
		nextPixelDistC = imageWindow->fullWidth;
		nextPixelDistR = nextPixelDistC - 1;
		nextPixelDistL = nextPixelDistC + 1;

		loopInBeg = imageWindow->yTerminus - 1;
		loopInEnd = imageWindow->yOrigin;
		loopInInc = imageWindow->xStep;

		seamLength = imageWindow->yLength;
	} else {
		loopBeg = imageWindow->firstPixel + imageWindow->xLength - 1;
		loopEnd = imageWindow->lastPixel;
		loopInc = imageWindow->yStep;

		// also set the next pixel distances
		nextPixelDistC = imageWindow->xStep;
		nextPixelDistR = imageWindow->fullWidth + nextPixelDistC;
		nextPixelDistL = (imageWindow->fullWidth - nextPixelDistC) * -1;

		loopInBeg = imageWindow->xTerminus;
		loopInEnd = imageWindow->xOrigin;
		loopInInc = imageWindow->xStep;

		seamLength = imageWindow->xLength;
	}

	// v5 experiemnts (based upon v2)
	int totalDeviation = 0;
	int totalDeviationL = 0;
	int totalDeviationR = 0;
	int lastTotalDeviationL = 0;
	int lastTotalDeviationR = 0;
	int seamPointer = 0;
	int seamBegan = 0;
	int *lastSeam = (int*)xmalloc((unsigned long)seamLength * sizeof(int));
	int *currentSeam = (int*)xmalloc((unsigned long)seamLength * sizeof(int));
	int deviationMin = imageWindow->fullWidth / 25;
	int deviationTol = imageWindow->fullWidth / 200;
	int clipAreaBound = DEFAULT_CLIP_AREA_BOUND;
	int straightDone = 0;
	int straightStart = 0;

	int k = loopBeg;
	int loopFinished = 0;
	int minValueLocation = 0;
	// for every pixel in the right-most or bottom-most column of the image
	while(!loopFinished) {
		// process seams with the lowest weights
		// start from the left-most column
		minValueLocation = k;
		countGoR = 0;
		countGoL = 0;

		// v5 experiemnts (based upon v2)
		if (findAreas) {
			totalDeviation = 0;
			totalDeviationL = 0;
			totalDeviationR = 0;
			seamPointer = 0;
			currentSeam[seamPointer] = minValueLocation;
		}

		// move right-to-left ot bottom-to-top across/up the image
		for (int j = loopInBeg; j > loopInEnd; j -= loopInInc) {
			// // v5 experiemnts (based upon v2)
			// if (findAreas) {
			// 	currentSeam[seamPointer] = minValueLocation;
			// 	++seamPointer;
			// }

			// THIS IS THE CRUCIAL PART
			if (direction == DIRECTION_VERTICAL) {
				if (imageVector[minValueLocation].usecountV < (PNG_MAX-SEAM_TRACE_INCREMENT)) {
					imageVector[minValueLocation].usecountV += SEAM_TRACE_INCREMENT;
				}
			} else {
				if (imageVector[minValueLocation].usecountH < (PNG_MAX-SEAM_TRACE_INCREMENT)) {
					imageVector[minValueLocation].usecountH += SEAM_TRACE_INCREMENT;
				}
			}

			// get the possible next pixles
            if (((minValueLocation - nextPixelDistR) > 0) && ((minValueLocation - nextPixelDistR) < loopEnd) ) {
            	if (direction == DIRECTION_VERTICAL) {
                	nextPixelR = imageVector[minValueLocation - nextPixelDistR].seamvalV;
                } else {
                	nextPixelR = imageVector[minValueLocation - nextPixelDistR].seamvalH;
                }
            } else {
                nextPixelR = INT_MAX;
            }

            if (((minValueLocation - nextPixelDistC) > 0) && ((minValueLocation - nextPixelDistC) < loopEnd) ) {
	            if (direction == DIRECTION_VERTICAL) {
		            nextPixelC = imageVector[minValueLocation - nextPixelDistC].seamvalV;
		        } else {
		        	nextPixelC = imageVector[minValueLocation - nextPixelDistC].seamvalH;
		        }
		    } else {
                nextPixelC = INT_MAX;
            }

            if (((minValueLocation - nextPixelDistL) > 0) && ((minValueLocation - nextPixelDistL) < loopEnd) ) {
                if (direction == DIRECTION_VERTICAL) {
	                nextPixelL = imageVector[minValueLocation - nextPixelDistL].seamvalV;
	            } else {
	            	nextPixelL = imageVector[minValueLocation - nextPixelDistL].seamvalH;
	            }
            } else {
                nextPixelL = INT_MAX;
            }

			// use the minimum of the possible pixels
			currentMin = min3(nextPixelR, nextPixelC, nextPixelL);

			// attempt to make the seam go back down if it was forced up and ice versa
			// the goal is to end on the same line which the seam started on, this
			// minimizes crazy diagonal seams which cut out important information
			if (countGoR == countGoL) {
				if (currentMin == nextPixelC) {
					minValueLocation -= nextPixelDistC;
				} else if (currentMin == nextPixelR) {
					minValueLocation -= nextPixelDistR;
					++countGoR;
					++totalDeviation;
				} else if (currentMin == nextPixelL) {
					minValueLocation -= nextPixelDistL;
					++countGoL;
					--totalDeviation;
				}
			} else if (countGoR > countGoL) {
				if (currentMin == nextPixelL) {
					minValueLocation -= nextPixelDistL;
					++countGoL;
					--totalDeviation;
				} else if (currentMin == nextPixelC) {
					minValueLocation -= nextPixelDistC;
				} else if (currentMin == nextPixelR) {
					minValueLocation -= nextPixelDistR;
					++countGoR;
					++totalDeviation;
				}
			} else if (countGoR < countGoL) {
				if (currentMin == nextPixelR) {
					minValueLocation -= nextPixelDistR;
					++countGoR;
					++totalDeviation;
				} else if (currentMin == nextPixelC) {
					minValueLocation -= nextPixelDistC;
				} else if (currentMin == nextPixelL) {
					minValueLocation -= nextPixelDistL;
					++countGoL;
					--totalDeviation;
				}
			}

			// v5 experiemnts (based upon v2)
			if (findAreas) {
				if (totalDeviation > 0) {
					++totalDeviationR;
				} else if (totalDeviation < 0) {
					++totalDeviationL;
				}

				++seamPointer;
				currentSeam[seamPointer] = minValueLocation;
			}
		}

		// v5 experiemnts (based upon v2)
		if (findAreas) {
			// only consider seams with persistent deviations
			if (totalDeviationL || totalDeviationR) {
				// if (direction == DIRECTION_VERTICAL) {
				// 	printf("%d\tdeviation: %d, L: %d, R: %d ", ((k + 1) % imageWindow->fullHeight), totalDeviation, totalDeviationL, totalDeviationR);
				// } else {
				// 	printf("%d\tdeviation: %d, L: %d, R: %d ", (k / imageWindow->fullWidth), totalDeviation, totalDeviationL, totalDeviationR);
				// }

				// persistently going left (bottom of an area)
				if (totalDeviationL > totalDeviationR) {
					// we already have the top of an area
					if (seamBegan) {
						// present deviation (plus tolerance) is less than last deviation amount
						// and the last deviation is greater than the minimum required deviation
						if (((totalDeviationL + deviationTol) < lastTotalDeviationL) && (lastTotalDeviationL > deviationMin)) {
							seamBegan = 0;
							// printf(" <<< L");

							straightDone = 0;
							for (int i = 0; i < seamLength; ++i) {
								if (direction == DIRECTION_VERTICAL) {
									if (clipAreaBound) {
										if ((i > 0) && ((currentSeam[i-1] - currentSeam[i]) != imageWindow->yStep)) {
											straightDone = 1;
											straightStart = 0;
										} else {
											if (straightDone && !straightStart) {
												straightStart = i;
											}
										}
									} else {
										straightDone = 1;
									}

									if (straightDone) {
										imageVector[currentSeam[i]].areaBoundaryV = 3;
									}
								} else {
									if (clipAreaBound) {
										if ((i > 0) && ((currentSeam[i-1] - currentSeam[i]) != imageWindow->xStep)) {
											straightDone = 1;
											straightStart = 0;
										} else {
											if (straightDone && !straightStart) {
												straightStart = i;
											}
										}
									} else {
										straightDone = 1;
									}

									if (straightDone) {
										imageVector[currentSeam[i]].areaBoundaryH = 3;
									}
								}
							}

							// remove final straight edge
							if (clipAreaBound && straightStart) {
								for (int i = straightStart; i < seamLength; ++i) {
									if (direction == DIRECTION_VERTICAL) {
										imageVector[currentSeam[i]].areaBoundaryV = 0;
									} else {
										imageVector[currentSeam[i]].areaBoundaryH = 0;
									}
								}
							}
						}

					// we don't have the top of an area yet
					} else {
						// present deviation (plus tolerance) is less than last deviation amount
						// and the last deviation is greater than the minimum required deviation
						if (((totalDeviationR + deviationTol) < lastTotalDeviationR) && (lastTotalDeviationR > deviationMin)) {
							seamBegan = 1;
							// printf(" <<< R1");

							straightDone = 0;
							for (int i = 0; i < seamLength; ++i) {
								if (direction == DIRECTION_VERTICAL) {
									if (clipAreaBound) {
										if ((i > 0) && ((currentSeam[i-1] - currentSeam[i]) != imageWindow->yStep)) {
											straightDone = 1;
											straightStart = 0;
										} else {
											if (straightDone && !straightStart) {
												straightStart = i;
											}
										}
									} else {
										straightDone = 1;
									}

									if (straightDone) {
										imageVector[lastSeam[i]].areaBoundaryV = 1;
									}
								} else {
									if (clipAreaBound) {
										if ((i > 0) && ((currentSeam[i-1] - currentSeam[i]) != imageWindow->xStep)) {
											straightDone = 1;
											straightStart = 0;
										} else {
											if (straightDone && !straightStart) {
												straightStart = i;
											}
										}
									} else {
										straightDone = 1;
									}

									if (straightDone) {
										imageVector[lastSeam[i]].areaBoundaryH = 1;
									}
								}
							}

							// remove final straight edge
							if (clipAreaBound && straightStart) {
								for (int i = straightStart; i < seamLength; ++i) {
									if (direction == DIRECTION_VERTICAL) {
										imageVector[lastSeam[i]].areaBoundaryV = 0;
									} else {
										imageVector[lastSeam[i]].areaBoundaryH = 0;
									}
								}
							}
						}
					}

				// persistently going right (top of an area)
				// totalDeviationL <= totalDeviationR
				} else {
					// only if a top has not yet been found (without a mathing bottom)
					if (!seamBegan) {
						// present deviation (plus tolerance) is less than last deviation amount
						// and the last deviation is greater than the minimum required deviation
						if (((totalDeviationR + deviationTol) < lastTotalDeviationR) && (lastTotalDeviationR > deviationMin)) {
							seamBegan = 1;
							// printf(" <<< R2");

							straightDone = 0;
							for (int i = 0; i < seamLength; ++i) {
								if (direction == DIRECTION_VERTICAL) {
									if (clipAreaBound) {
										if ((i > 0) && ((currentSeam[i-1] - currentSeam[i]) != imageWindow->yStep)) {
											straightDone = 1;
											straightStart = 0;
										} else {
											if (straightDone && !straightStart) {
												straightStart = i;
											}
										}
									} else {
										straightDone = 1;
									}

									if (straightDone) {
										imageVector[lastSeam[i]].areaBoundaryV = 1;
									}
								} else {
									if (clipAreaBound) {
										if ((i > 0) && ((currentSeam[i-1] - currentSeam[i]) != imageWindow->xStep)) {
											straightDone = 1;
											straightStart = 0;
										} else {
											if (straightDone && !straightStart) {
												straightStart = i;
											}
										}
									} else {
										straightDone = 1;
									}

									if (straightDone) {
										imageVector[lastSeam[i]].areaBoundaryH = 1;
									}
								}
							}

							// remove final straight edge
							if (clipAreaBound && straightStart) {
								for (int i = straightStart; i < seamLength; ++i) {
									if (direction == DIRECTION_VERTICAL) {
										imageVector[lastSeam[i]].areaBoundaryV = 0;
									} else {
										imageVector[lastSeam[i]].areaBoundaryH = 0;
									}
								}
							}
						}
					}
				}
				// printf("\n");
			}

			lastTotalDeviationL = totalDeviationL;
			lastTotalDeviationR = totalDeviationR;
			for (int i = 0; i < seamLength; ++i) {
				lastSeam[i] = currentSeam[i];
			}
		}

		//for (int k = loopBeg; k < loopEnd; k += loopInc) {
		k += loopInc;
		if (direction == DIRECTION_VERTICAL) {
			if (k <= loopEnd) {
				loopFinished = 1;
			}
		} else {
			if (k >= loopEnd) {
				loopFinished = 1;
			}
		}
	}

	free(lastSeam);
	free(currentSeam);
}

static void setPixelPathVertical(struct pixel *imageVector, struct window *imageWindow, int currentPixel, int currentCol)
{
	//return;

	int pixelAbove = 0;
	int aboveL = 0;
	int aboveC = 0;
	int aboveR = 0;
	int newValue = 0;

	pixelAbove = currentPixel - imageWindow->yStep;
	// avoid falling off the left end
	if (currentCol > 0) {
		// avoid falling off the right end
		if (currentCol < imageWindow->xLength) {
			aboveL = imageVector[pixelAbove - imageWindow->xStep].seamvalV;
			aboveC = imageVector[pixelAbove].seamvalV;
			aboveR = imageVector[pixelAbove + imageWindow->xStep].seamvalV;
			newValue = min3(aboveL, aboveC, aboveR);
		} else {
			aboveL = imageVector[pixelAbove - imageWindow->xStep].seamvalV;
			aboveC = imageVector[pixelAbove].seamvalV;
			aboveR = INT_MAX;
			newValue = min(aboveL, aboveC);
		}
	} else {
		aboveL = INT_MAX;
		aboveC = imageVector[pixelAbove].seamvalV;
		aboveR = imageVector[pixelAbove + imageWindow->xStep].seamvalV;
		newValue = min(aboveC, aboveR);
	}
	imageVector[currentPixel].seamvalV += newValue;
	//
	// This (below) is kinda a big deal
	//
	if (imageVector[currentPixel].seamvalV > 0) {
		imageVector[currentPixel].seamvalV -= 1;
	}
}

static int fillSeamMatrixVertical(struct pixel *imageVector, struct window *imageWindow)
{
	int result = 0;
	int currentPixel = 0;
	// do not process the first row, start with j=1
	for (int y = (imageWindow->yOrigin + 1); y < imageWindow->yTerminus; y += imageWindow->xStep) {
		for (int x = imageWindow->xOrigin; x < imageWindow->xTerminus; x += imageWindow->xStep) {
			currentPixel = (y * imageWindow->fullWidth) + x;
			setPixelPathVertical(imageVector, imageWindow, currentPixel, x);

			if (imageVector[currentPixel].seamvalV != 0) {
				++result;
			}
		}
	}
	return result;
}

static void findSeamsVertical(struct pixel *imageVector, struct window *imageWindow, int findAreas)
{
	findSeams(imageVector, imageWindow, 0, findAreas);
}

static void setPixelPathHorizontal(struct pixel *imageVector, struct window *imageWindow, int currentPixel, int currentCol)
{
	//return;

	// avoid falling off the right
	if (currentCol < imageWindow->xLength) {
		int pixelLeft = 0;
		int leftT = 0;
		int leftM = 0;
		int leftB = 0;
		int newValue = 0;

		pixelLeft = currentPixel - imageWindow->xStep;
		// avoid falling off the top
		if (currentPixel > imageWindow->xLength) {
			// avoid falling off the bottom
			if (currentPixel < (imageWindow->pixelCount - imageWindow->xLength)) {
				leftT = imageVector[pixelLeft - imageWindow->yStep].seamvalH;
				leftM = imageVector[pixelLeft].seamvalH;
				leftB = imageVector[pixelLeft + imageWindow->yStep].seamvalH;
				newValue = min3(leftT, leftM, leftB);
			} else {
				leftT = imageVector[pixelLeft - imageWindow->yStep].seamvalH;
				leftM = imageVector[pixelLeft].seamvalH;
				leftB = INT_MAX;
				newValue = min(leftT, leftM);
			}
		} else {
			leftT = INT_MAX;
			leftM = imageVector[pixelLeft].seamvalH;
			leftB = imageVector[pixelLeft + imageWindow->yStep].seamvalH;
			newValue = min(leftM, leftB);
		}
		imageVector[currentPixel].seamvalH += newValue;
		//
		// This (below) is kinda a big deal
		//
		if (imageVector[currentPixel].seamvalH > 0) {
			imageVector[currentPixel].seamvalH -= 1;
		}
	}
}

static int fillSeamMatrixHorizontal(struct pixel *imageVector, struct window *imageWindow)
{
	int result = 0;
	int currentPixel = 0;
	// do not process the first row, start with j=1
	// must be in reverse order from verticle seam, calulate colums as we move across (top down, left to right)
	for (int x = imageWindow->xOrigin; x < imageWindow->xTerminus; x += imageWindow->xStep) {
		for (int y = (imageWindow->yOrigin + 1); y < imageWindow->yTerminus; y += 1) {
			currentPixel = (y * imageWindow->fullWidth) + x;
			setPixelPathHorizontal(imageVector, imageWindow, currentPixel, x);

			if (imageVector[currentPixel].seamvalH != 0) {
				++result;
			}
		}
	}
	return result;
}

static void findSeamsHorizontal(struct pixel *imageVector, struct window *imageWindow, int findAreas)
{
	findSeams(imageVector, imageWindow, 1, findAreas);
}

/*
 * The main function
 */
static int *seamCarve(int *imageVector, int imageWidth, int imageHeight, int imageDepth, int brightnessMode, int contrastMode, int forceDirection, int forceEdge, int preGauss)
{
	struct pixel *workingImage = (struct pixel*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(struct pixel));
	int *resultImage = (int*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * (unsigned long)imageDepth * sizeof(int));

	int invertOutput = 1;

	double valcx = 0.0;
	double valcy = 0.0;
	double valcz = 0.0;
	int inputPixel = 0;
	int outputPixel = 0;
	int currentPixel = 0;
	int currentBrightness = 0;
	// fill initial data structures
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			inputPixel = currentPixel * imageDepth;

			struct pixel newPixel;
			newPixel.r = imageVector[inputPixel];
			newPixel.g = imageVector[inputPixel+1];
			newPixel.b = imageVector[inputPixel+2];
			newPixel.a = imageVector[inputPixel+3];

			if (brightnessMode == 0) {
				// Average Intensity / Brightness
				newPixel.bright = ((imageVector[inputPixel] + imageVector[inputPixel+1] + imageVector[inputPixel+2]) / 3);
			} else if (brightnessMode == 1) {
				// HSV hexcone
				newPixel.bright = max3(imageVector[inputPixel], imageVector[inputPixel+1], imageVector[inputPixel+2]);
			} else if (brightnessMode == 2) {
				// Luma luminance -- sRGB / BT.709
				newPixel.bright = (imageVector[inputPixel] * 0.21) + (imageVector[inputPixel+1] * 0.72) + (imageVector[inputPixel+2] * 0.07);
			} else if (brightnessMode == 3) {
				// Luma luminance -- NTSC / BT.601 (Digital CCIR601)
				//newPixel.bright = (imageVector[inputPixel] * 0.3) + (imageVector[inputPixel+1] * 0.59) + (imageVector[inputPixel+2] * 0.11);
				newPixel.bright = (imageVector[inputPixel] * 0.299) + (imageVector[inputPixel+1] * 0.587) + (imageVector[inputPixel+2] * 0.114);
			} else if (brightnessMode == 4) {
				// Relative luminance (Photometric/digital ITU-R)
				newPixel.bright = (imageVector[inputPixel] * 0.2126) + (imageVector[inputPixel+1] * 0.7152) + (imageVector[inputPixel+2] * 0.0722);
			} else if (brightnessMode == 5) {
				// HSP?
				newPixel.bright = sqrt((imageVector[inputPixel] * imageVector[inputPixel] * 0.299) + (imageVector[inputPixel+1] * imageVector[inputPixel+1] * 0.587) + (imageVector[inputPixel+2] * imageVector[inputPixel+2] * 0.114));
			} else if (brightnessMode == 6) {
				// Euclidian distance
				newPixel.bright = pow((imageVector[inputPixel] * imageVector[inputPixel]) + (imageVector[inputPixel+1] * imageVector[inputPixel+1]) + (imageVector[inputPixel+2] * imageVector[inputPixel+2]), 0.33333);
			} else if (brightnessMode == 7) {
				// Fast ITU-R
				newPixel.bright = (imageVector[inputPixel] * 0.33) + (imageVector[inputPixel+1] * 0.5) + (imageVector[inputPixel+2] * 0.16);
				// even faster (when not in an IF condition)
				//newPixel.bright = (imageVector[inputPixel] + imageVector[inputPixel] + imageVector[inputPixel+1] + imageVector[inputPixel+1] + imageVector[inputPixel+1] + imageVector[inputPixel+2]) / 6;
			} else if (brightnessMode == 8) {
				// Fast BT.601
				newPixel.bright = (imageVector[inputPixel] * 0.375) + (imageVector[inputPixel+1] * 0.5) + (imageVector[inputPixel+2] * 0.125);
				// even faster (when not in an IF condition)
				//newPixel.bright = (imageVector[inputPixel] + imageVector[inputPixel] + imageVector[inputPixel] + imageVector[inputPixel+1] + imageVector[inputPixel+1] + imageVector[inputPixel+1] + imageVector[inputPixel+1] + imageVector[inputPixel+2]) >> 3;
			} else if (brightnessMode == 9) {
				// Euclidean distance of Lab color over Gaussian distribution
				valcx = rgbToXyzX(imageVector[inputPixel], imageVector[inputPixel+1], imageVector[inputPixel+2]);
				valcy = rgbToXyzY(imageVector[inputPixel], imageVector[inputPixel+1], imageVector[inputPixel+2]);
				valcz = rgbToXyzZ(imageVector[inputPixel], imageVector[inputPixel+1], imageVector[inputPixel+2]);

				newPixel.L = xyzToLabL(valcx, valcy, valcz);
				newPixel.A = xyzToLabA(valcx, valcy, valcz);
				newPixel.B = xyzToLabB(valcx, valcy, valcz);

				newPixel.bright = 0;
			}

			newPixel.energy = 0;
			newPixel.seamvalH = 0;
			newPixel.seamvalV = 0;
			newPixel.usecountH = 0;
			newPixel.usecountV = 0;
			newPixel.areaBoundaryH = 0;
			newPixel.areaBoundaryV = 0;
			workingImage[currentPixel] = newPixel;

			resultImage[inputPixel] = 0;
		}
	}

	double tmp1 = 0.0;
	double tmp2 = 0.0;

	// tmp1 = rgbToXyzX(0, 0, 0); // 0
	// tmp2 = rgbToXyzX(255, 255, 255); // 0.950456
	// printf("rgbToXyzX: %f, %f\n", tmp1, tmp2);
	// tmp1 = rgbToXyzY(0, 0, 0); // 0
	// tmp2 = rgbToXyzY(255, 255, 255); // 1.000000
	// printf("rgbToXyzY: %f, %f\n", tmp1, tmp2);
	// tmp1 = rgbToXyzZ(0, 0, 0); // 0
	// tmp2 = rgbToXyzZ(255, 255, 255); // 1.088754
	// printf("rgbToXyzZ: %f, %f\n", tmp1, tmp2);

	// tmp1 = xyzToLabL(0, 0, 0); // 0
	// tmp2 = xyzToLabL(0.950456, 1.000000, 1.088754); // 100.000000 (10,000)
	// printf("xyzToLabL: %f, %f\n", tmp1, tmp2);
	// tmp1 = xyzToLabA(0, 0, 0); // 0
	// tmp2 = xyzToLabA(0.950456, 1.000000, 1.088754); // -8.397583 (70.519400242)
	// printf("xyzToLabA: %f, %f\n", tmp1, tmp2);
	// tmp1 = xyzToLabB(0, 0, 0); // 0
	// tmp2 = xyzToLabB(0.950456, 1.000000, 1.088754); // -5.750035 (33.062902501)
	// printf("xyzToLabB: %f, %f\n", tmp1, tmp2);

	// each difference has a range of 0 - 100.52

	// calculate the birghtness based upon the difference between pixels over a Guassian area
	// /* */
	if (brightnessMode == 9) {
		double weights[25];

		weights[0]  = 0.038764;
		weights[1]  = 0.039682;
		weights[2]  = 0.039993;
		weights[6]  = 0.040622;
		weights[7]  = 0.040940;
		weights[12] = 0.041261;

		// line 1 has 2 duplicated values
		weights[3] = weights[1];
		weights[4] = weights[0];
		// line 2 has 3 duplicated values
		weights[5] = weights[1];
		weights[8] = weights[6];
		weights[9] = weights[5];
		// line 3 has 4 duplicated values
		weights[10] = weights[2];
		weights[11] = weights[7];
		weights[13] = weights[11];
		weights[14] = weights[10];
		// line 4 is the same as line 2
		weights[15] = weights[5];
		weights[16] = weights[6];
		weights[17] = weights[7];
		weights[18] = weights[8];
		weights[19] = weights[9];
		// line 5 is the  same as line 1
		weights[20] = weights[0];
		weights[21] = weights[1];
		weights[22] = weights[2];
		weights[23] = weights[3];
		weights[24] = weights[4];

		int points[25];
		double pointValues[25];

		double valcx = 0.0;
		double valcy = 0.0;
		double valcz = 0.0;

		double valcl = 0.0;
		double valca = 0.0;
		double valcb = 0.0;

		double valnl = 0.0;
		double valna = 0.0;
		double valnb = 0.0;

		double gaussL1 = 0.0;
		double gaussL2 = 0.0;
		double gaussL3 = 0.0;
		double gaussL4 = 0.0;
		double gaussL5 = 0.0;
		double gaussAll = 0.0;

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				points[0] = currentPixel - imageWidth - imageWidth - 1 - 1;
				points[1] = currentPixel - imageWidth - imageWidth - 1;
				points[2] = currentPixel - imageWidth - imageWidth;
				points[3] = currentPixel - imageWidth - imageWidth + 1;
				points[4] = currentPixel - imageWidth - imageWidth + 1 + 1;

				points[5] = currentPixel - imageWidth - 1 - 1;
				points[6] = currentPixel - imageWidth - 1;
				points[7] = currentPixel - imageWidth;
				points[8] = currentPixel - imageWidth + 1;
				points[9] = currentPixel - imageWidth + 1 + 1;

				points[10] = currentPixel - 1 - 1;
				points[11] = currentPixel - 1;
				points[12] = currentPixel;
				points[13] = currentPixel + 1;
				points[14] = currentPixel + 1 + 1;

				points[15] = currentPixel + imageWidth - 1 - 1;
				points[16] = currentPixel + imageWidth - 1;
				points[17] = currentPixel + imageWidth;
				points[18] = currentPixel + imageWidth + 1;
				points[19] = currentPixel + imageWidth + 1 + 1;

				points[20] = currentPixel + imageWidth + imageWidth - 1 - 1;
				points[21] = currentPixel + imageWidth + imageWidth - 1;
				points[22] = currentPixel + imageWidth + imageWidth;
				points[23] = currentPixel + imageWidth + imageWidth + 1;
				points[24] = currentPixel + imageWidth + imageWidth + 1 + 1;

				// TODO: this is wrong, fix it
				for (int i = 0; i < 25; ++i) {
					if (points[i] < 0) {
						points[i] = 0;
					} else if (points[i] >= (imageHeight * imageWidth)) {
						points[i] = (imageHeight * imageWidth);
					}
				}

				valcx = rgbToXyzX(workingImage[currentPixel].r, workingImage[currentPixel].g, workingImage[currentPixel].b);
				valcy = rgbToXyzY(workingImage[currentPixel].r, workingImage[currentPixel].g, workingImage[currentPixel].b);
				valcz = rgbToXyzZ(workingImage[currentPixel].r, workingImage[currentPixel].g, workingImage[currentPixel].b);

				// What Bob Ross would call a happy accident
				// notice the bug, which is there in purpose now (all xyzToLabL)
				valcl = xyzToLabL(valcx, valcy, valcz);
				valca = xyzToLabL(valcx, valcy, valcz);
				valcb = xyzToLabL(valcx, valcy, valcz);
				// valcl = xyzToLabL(valcx, valcy, valcz);
				// valca = xyzToLabA(valcx, valcy, valcz);
				// valcb = xyzToLabB(valcx, valcy, valcz);

				valnl = valcl - workingImage[points[0]].L;
				valna = valca - workingImage[points[0]].A;
				valnb = valcb - workingImage[points[0]].B;
				pointValues[0] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[1]].L;
				valna = valca - workingImage[points[1]].A;
				valnb = valcb - workingImage[points[1]].B;
				pointValues[1] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[2]].L;
				valna = valca - workingImage[points[2]].A;
				valnb = valcb - workingImage[points[2]].B;
				pointValues[2] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[3]].L;
				valna = valca - workingImage[points[3]].A;
				valnb = valcb - workingImage[points[3]].B;
				pointValues[3] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[4]].L;
				valna = valca - workingImage[points[4]].A;
				valnb = valcb - workingImage[points[4]].B;
				pointValues[4] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[5]].L;
				valna = valca - workingImage[points[5]].A;
				valnb = valcb - workingImage[points[5]].B;
				pointValues[5] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[6]].L;
				valna = valca - workingImage[points[6]].A;
				valnb = valcb - workingImage[points[6]].B;
				pointValues[6] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[7]].L;
				valna = valca - workingImage[points[7]].A;
				valnb = valcb - workingImage[points[7]].B;
				pointValues[7] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[8]].L;
				valna = valca - workingImage[points[8]].A;
				valnb = valcb - workingImage[points[8]].B;
				pointValues[8] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[9]].L;
				valna = valca - workingImage[points[9]].A;
				valnb = valcb - workingImage[points[9]].B;
				pointValues[9] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[10]].L;
				valna = valca - workingImage[points[10]].A;
				valnb = valcb - workingImage[points[10]].B;
				pointValues[10] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[11]].L;
				valna = valca - workingImage[points[11]].A;
				valnb = valcb - workingImage[points[11]].B;
				pointValues[11] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				pointValues[12] = 0;

				valnl = valcl - workingImage[points[13]].L;
				valna = valca - workingImage[points[13]].A;
				valnb = valcb - workingImage[points[13]].B;
				pointValues[13] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[14]].L;
				valna = valca - workingImage[points[14]].A;
				valnb = valcb - workingImage[points[14]].B;
				pointValues[14] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[15]].L;
				valna = valca - workingImage[points[15]].A;
				valnb = valcb - workingImage[points[15]].B;
				pointValues[15] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[16]].L;
				valna = valca - workingImage[points[16]].A;
				valnb = valcb - workingImage[points[16]].B;
				pointValues[16] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[17]].L;
				valna = valca - workingImage[points[17]].A;
				valnb = valcb - workingImage[points[17]].B;
				pointValues[17] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[18]].L;
				valna = valca - workingImage[points[18]].A;
				valnb = valcb - workingImage[points[18]].B;
				pointValues[18] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[19]].L;
				valna = valca - workingImage[points[19]].A;
				valnb = valcb - workingImage[points[19]].B;
				pointValues[19] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[20]].L;
				valna = valca - workingImage[points[20]].A;
				valnb = valcb - workingImage[points[20]].B;
				pointValues[20] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[21]].L;
				valna = valca - workingImage[points[21]].A;
				valnb = valcb - workingImage[points[21]].B;
				pointValues[21] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[22]].L;
				valna = valca - workingImage[points[22]].A;
				valnb = valcb - workingImage[points[22]].B;
				pointValues[22] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[23]].L;
				valna = valca - workingImage[points[23]].A;
				valnb = valcb - workingImage[points[23]].B;
				pointValues[23] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				valnl = valcl - workingImage[points[24]].L;
				valna = valca - workingImage[points[24]].A;
				valnb = valcb - workingImage[points[24]].B;
				pointValues[24] = sqrt((valnl * valnl) + (valna * valna) + (valnb * valnb));

				gaussL1 = (weights[0]  * pointValues[0])  + (weights[1]  * pointValues[1])  + (weights[2]  * pointValues[2])  + (weights[3]  * pointValues[3])  + (weights[4]  * pointValues[4]);
				gaussL2 = (weights[5]  * pointValues[5])  + (weights[6]  * pointValues[6])  + (weights[7]  * pointValues[7])  + (weights[8]  * pointValues[8])  + (weights[9]  * pointValues[9]);
				gaussL3 = (weights[10] * pointValues[10]) + (weights[11] * pointValues[11]) + (weights[12] * pointValues[12]) + (weights[13] * pointValues[13]) + (weights[14] * pointValues[14]);
				gaussL4 = (weights[15] * pointValues[15]) + (weights[16] * pointValues[16]) + (weights[17] * pointValues[17]) + (weights[18] * pointValues[18]) + (weights[19] * pointValues[19]);
				gaussL5 = (weights[20] * pointValues[20]) + (weights[21] * pointValues[21]) + (weights[22] * pointValues[22]) + (weights[23] * pointValues[23]) + (weights[24] * pointValues[24]);

				gaussAll = (gaussL1 + gaussL2 + gaussL3 + gaussL4 + gaussL5);
				// if(gaussAll < 25) printf("%f\n", gaussAll);
				// if(gaussAll > 150) printf("%f\n", gaussAll);

				// gaussAll = gaussAll / 24;
				// if(gaussAll < 1.05) printf("%f\n", gaussAll);
				// if(gaussAll > 6.25) printf("%f\n", gaussAll);

				gaussAll = (gaussAll - 25) * 2;
				// if(gaussAll < 2) printf("%f\n", gaussAll);
				// if(gaussAll > 250) printf("%f\n", gaussAll);


				workingImage[currentPixel].bright = min(max((int)gaussAll, 0), PNG_MAX);
				//printf("%d\n", workingImage[currentPixel].bright );
			}
		}
	}
	/* */

	// binarize the image as/if requested
	if (contrastMode == 1) {
		// not really binarized, but brightness passed though a cosine function
		// this increases differentiation between light and dark pixels
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImage[currentPixel].bright;
				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);
				workingImage[currentPixel].bright = currentBrightness;
			}
		}

	} else if (contrastMode == 2) {
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImage[currentPixel].bright;
				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				workingImage[currentPixel].bright = currentBrightness;
			}
		}

	} else if (contrastMode == 3) {
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImage[currentPixel].bright;
				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				workingImage[currentPixel].bright = currentBrightness;
			}
		}

	} else if (contrastMode == 4) {
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImage[currentPixel].bright;
				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				currentRadians = ((double)currentBrightness / (double)PNG_MAX) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * (double)PNG_MAX);

				workingImage[currentPixel].bright = currentBrightness;
			}
		}

	} else if (contrastMode == 5) {
		// TODO: merge with above loops and below loops (split)

		int bins[256];
		int currentBrightness = 0;

		// get historgram
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				currentBrightness = workingImage[currentPixel].bright;
				bins[currentBrightness] += 1;
			}
		}

		int threshold = otsuBinarization(bins, (imageWidth * imageHeight));

		// apply threshold
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				currentBrightness = workingImage[currentPixel].bright;
				if (currentBrightness > threshold) {
					workingImage[currentPixel].bright = PNG_MAX;
				} else {
					workingImage[currentPixel].bright = 0;
				}
			}
		}
	}

	int gaussA = 0;
    int gaussB = 0;
	int tmpDoG = 0;
	int tmpSobel = 0;

	// get energy values using the prescribed method
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;

			if (preGauss == 1) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 20);
			} else if (preGauss == 2) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 40);
			} else if (preGauss == 3) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 80);
			}

			if (forceEdge == 0) {
				gaussA = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 14);
				gaussB = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 16);
				workingImage[currentPixel].energy = getPixelEnergyDoG(gaussA, gaussB);
			} else if (forceEdge == 1) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 80);
				workingImage[currentPixel].energy = sqrt(getPixelEnergyLaplacian(workingImage, imageWidth, imageHeight, currentPixel));
			} else if (forceEdge == 2) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 40);
				workingImage[currentPixel].energy = sqrt(getPixelEnergyLaplacian(workingImage, imageWidth, imageHeight, currentPixel));
			} else if (forceEdge == 3) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 20);
				workingImage[currentPixel].energy = sqrt(getPixelEnergyLaplacian(workingImage, imageWidth, imageHeight, currentPixel));
			} else if (forceEdge == 4) {
				workingImage[currentPixel].energy = sqrt(getPixelEnergySobel(workingImage, imageWidth, imageHeight, currentPixel) / 2.4);
				//if(workingImage[currentPixel].energy > 20) printf("%d\n", workingImage[currentPixel].energy);
			} else if (forceEdge == 5) {
				workingImage[currentPixel].energy = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 9999);
			} else if (forceEdge == 6) {
				workingImage[currentPixel].energy = getPixelEnergySimple(workingImage, imageWidth, imageHeight, currentPixel, 1);
			} else if (forceEdge == 7) {
				gaussA = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 14);
				gaussB = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 16);

				tmpDoG = getPixelEnergyDoG(gaussA, gaussB);
				tmpSobel = getPixelEnergySobel(workingImage, imageWidth, imageHeight, currentPixel);

				workingImage[currentPixel].energy = (tmpDoG + ((tmpSobel / 80) * 20)) / 2;
			} else if (forceEdge == 8) {
				workingImage[currentPixel].bright = getPixelGaussian(workingImage, imageWidth, imageHeight, 1, currentPixel, 80);
				tmpDoG = sqrt(getPixelEnergyLaplacian(workingImage, imageWidth, imageHeight, currentPixel));
				tmpSobel = getPixelEnergySobel(workingImage, imageWidth, imageHeight, currentPixel);

				if (tmpDoG && tmpSobel) {
					workingImage[currentPixel].energy = (tmpDoG + (tmpSobel / 24));
				} else {
					workingImage[currentPixel].energy = 0;
				}
			} else if (forceEdge == 9) {
				workingImage[currentPixel].energy = getPixelEnergyStoll(workingImage, imageWidth, imageHeight, 1, currentPixel);
			}
			workingImage[currentPixel].seamvalH = workingImage[currentPixel].energy;
			workingImage[currentPixel].seamvalV = workingImage[currentPixel].energy;
		}
	}

	struct window *currentWindow = newWindow(0, 0, imageWidth, imageHeight, imageWidth, imageHeight);

	// find seams in the prescribed direction
	int resultDirection = forceDirection;
	if (forceDirection == 0) {
		int horizontalSeamCost = fillSeamMatrixHorizontal(workingImage, currentWindow);
		int verticalSeamCost = fillSeamMatrixVertical(workingImage, currentWindow);
		//printf("Sum traversal cost of all seams: horizontal = %d, vertical = %d \n", verticalSeamCost, horizontalSeamCost);

		findSeamsHorizontal(workingImage, currentWindow, 0);
		findSeamsVertical(workingImage, currentWindow, 0);

		if (horizontalSeamCost < verticalSeamCost) {
			printf("Horizontal \n");
			resultDirection = 1;
		} else {
			printf("Vertical \n");
			resultDirection = 2;
		}
	} else if ((forceDirection == 1) || (forceDirection == 6) || (forceDirection == 8) || (forceDirection == 49)) {
		fillSeamMatrixHorizontal(workingImage, currentWindow);
		if (forceDirection == 49) {
			findSeamsHorizontal(workingImage, currentWindow, 1);
		} else {
			findSeamsHorizontal(workingImage, currentWindow, 0);
		}
	} else if ((forceDirection == 2) || (forceDirection == 7) || (forceDirection == 9) || (forceDirection == 50)) {
		fillSeamMatrixVertical(workingImage, currentWindow);
		if (forceDirection == 50) {
			findSeamsVertical(workingImage, currentWindow, 1);
		} else {
			findSeamsVertical(workingImage, currentWindow, 0);
		}
	} else if ((forceDirection == 4) || (forceDirection == 5)) {
		// pass
	} else {
		fillSeamMatrixHorizontal(workingImage, currentWindow);
		fillSeamMatrixVertical(workingImage, currentWindow);

		findSeamsHorizontal(workingImage, currentWindow, 0);
		findSeamsVertical(workingImage, currentWindow, 0);
	}

	// prepare results for output
	if (resultDirection == 1) {
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImage[currentPixel].usecountH > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r / 2;
					resultImage[outputPixel+1] = workingImage[currentPixel].g / 2;
					resultImage[outputPixel+2] = workingImage[currentPixel].b / 2;
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 2) {
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImage[currentPixel].usecountV > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r / 2;
					resultImage[outputPixel+1] = workingImage[currentPixel].g / 2;
					resultImage[outputPixel+2] = workingImage[currentPixel].b / 2;
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 3) {
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				currentUseCount = workingImage[currentPixel].usecountH + workingImage[currentPixel].usecountV;
				if (currentUseCount > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r / 2;
					resultImage[outputPixel+1] = workingImage[currentPixel].g / 2;
					resultImage[outputPixel+2] = workingImage[currentPixel].b / 2;
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 4) {
		int scaleFactor = 32;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				resultImage[outputPixel] = min(max(workingImage[currentPixel].bright, 0), PNG_MAX) * scaleFactor;
				resultImage[outputPixel+1] = min(max(workingImage[currentPixel].bright, 0), PNG_MAX) * scaleFactor;
				resultImage[outputPixel+2] = min(max(workingImage[currentPixel].bright, 0), PNG_MAX) * scaleFactor;
				resultImage[outputPixel+3] = PNG_MAX;
			}
		}

	} else if (resultDirection == 5) {
		int energyScale = 16;
		int scaleFactor = 32;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				resultImage[outputPixel] = min(max((workingImage[currentPixel].energy * energyScale), 0), PNG_MAX) * scaleFactor;
				resultImage[outputPixel+1] = min(max((workingImage[currentPixel].energy * energyScale), 0), PNG_MAX) * scaleFactor;
				resultImage[outputPixel+2] = min(max((workingImage[currentPixel].energy * energyScale), 0), PNG_MAX) * scaleFactor;
				resultImage[outputPixel+3] = PNG_MAX;
			}
		}

	} else if (resultDirection == 6) {
		int seamValueScale = 16;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				currentUseCount = workingImage[currentPixel].usecountH;
				if (currentUseCount > 256) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = min(max((workingImage[currentPixel].seamvalH * seamValueScale), 0), PNG_MAX);
					resultImage[outputPixel+1] = min(max((workingImage[currentPixel].seamvalH * seamValueScale), 0), PNG_MAX);
					resultImage[outputPixel+2] = min(max((workingImage[currentPixel].seamvalH * seamValueScale), 0), PNG_MAX);
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 7) {
		int seamValueScale = 16;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				currentUseCount = workingImage[currentPixel].usecountV;
				if (currentUseCount > 256) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = min(max((workingImage[currentPixel].seamvalV * seamValueScale), 0), PNG_MAX);
					resultImage[outputPixel+1] = min(max((workingImage[currentPixel].seamvalV * seamValueScale), 0), PNG_MAX);
					resultImage[outputPixel+2] = min(max((workingImage[currentPixel].seamvalV * seamValueScale), 0), PNG_MAX);
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 8) {
		int seamValueScale = 4;
		int currentUseCount = 0;

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				currentUseCount = workingImage[currentPixel].usecountH;
				if (currentUseCount > 256) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					if (invertOutput) {
						resultImage[outputPixel] = PNG_MAX-min(max((workingImage[currentPixel].usecountH), 0), PNG_MAX);
						resultImage[outputPixel+1] = PNG_MAX-min(max((workingImage[currentPixel].usecountH), 0), PNG_MAX);
						resultImage[outputPixel+2] = PNG_MAX-min(max((workingImage[currentPixel].usecountH), 0), PNG_MAX);
						resultImage[outputPixel+3] = PNG_MAX;
					} else {
						resultImage[outputPixel] = min(max((workingImage[currentPixel].usecountH), 0), PNG_MAX);
						resultImage[outputPixel+1] = min(max((workingImage[currentPixel].usecountH), 0), PNG_MAX);
						resultImage[outputPixel+2] = min(max((workingImage[currentPixel].usecountH), 0), PNG_MAX);
						resultImage[outputPixel+3] = PNG_MAX;
					}
				}
			}
		}

	} else if (resultDirection == 9) {
		int seamValueScale = 4;
		int currentUseCount = 0;

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				currentUseCount = workingImage[currentPixel].usecountV;
				if (currentUseCount > 256) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					if (invertOutput) {
						resultImage[outputPixel] = PNG_MAX-min(max((workingImage[currentPixel].usecountV), 0), PNG_MAX);
						resultImage[outputPixel+1] = PNG_MAX-min(max((workingImage[currentPixel].usecountV), 0), PNG_MAX);
						resultImage[outputPixel+2] = PNG_MAX-min(max((workingImage[currentPixel].usecountV), 0), PNG_MAX);
						resultImage[outputPixel+3] = PNG_MAX;
					} else {
						resultImage[outputPixel] = min(max((workingImage[currentPixel].usecountV), 0), PNG_MAX);
						resultImage[outputPixel+1] = min(max((workingImage[currentPixel].usecountV), 0), PNG_MAX);
						resultImage[outputPixel+2] = min(max((workingImage[currentPixel].usecountV), 0), PNG_MAX);
						resultImage[outputPixel+3] = PNG_MAX;
					}
				}
			}
		}

	} else if (resultDirection == 49) {
		int seamValueScale = 4;
		int currentUseCount = 0;

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImage[currentPixel].areaBoundaryH == 1) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else if (workingImage[currentPixel].areaBoundaryH == 2) {
					resultImage[outputPixel] = 0;
					resultImage[outputPixel+1] = PNG_MAX;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else if (workingImage[currentPixel].areaBoundaryH == 3) {
					resultImage[outputPixel] = 0;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = PNG_MAX;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r;
					resultImage[outputPixel+1] = workingImage[currentPixel].g;
					resultImage[outputPixel+2] = workingImage[currentPixel].b;
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 50) {
		int seamValueScale = 4;
		int currentUseCount = 0;

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImage[currentPixel].areaBoundaryV == 1) {
					resultImage[outputPixel] = PNG_MAX;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else if (workingImage[currentPixel].areaBoundaryV == 2) {
					resultImage[outputPixel] = 0;
					resultImage[outputPixel+1] = PNG_MAX;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = PNG_MAX;
				} else if (workingImage[currentPixel].areaBoundaryV == 3) {
					resultImage[outputPixel] = 0;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = PNG_MAX;
					resultImage[outputPixel+3] = PNG_MAX;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r;
					resultImage[outputPixel+1] = workingImage[currentPixel].g;
					resultImage[outputPixel+2] = workingImage[currentPixel].b;
					resultImage[outputPixel+3] = PNG_MAX;
				}
			}
		}

	} else if (resultDirection == 51) {
		int seamValueScale = 4;
		int currentUseCountH = 0;
		int currentUseCountV = 0;
		int currentUseCount = 0;

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				currentUseCountH = workingImage[currentPixel].usecountH;
				currentUseCountV = workingImage[currentPixel].usecountV;
				currentUseCount = currentUseCountH + currentUseCountV;
				if ((currentUseCountH <= 0) || (currentUseCountV <= 0)) {
					resultImage[outputPixel] = workingImage[currentPixel].r;
					resultImage[outputPixel+1] = workingImage[currentPixel].g;
					resultImage[outputPixel+2] = workingImage[currentPixel].b;
					resultImage[outputPixel+3] = PNG_MAX;
				// } else if ((currentUseCountH <= 0) || (currentUseCountV <= 0)) {
				// 	resultImage[outputPixel] = workingImage[currentPixel].r;
				// 	resultImage[outputPixel+1] = workingImage[currentPixel].g;
				// 	resultImage[outputPixel+2] = workingImage[currentPixel].b;
				// 	resultImage[outputPixel+3] = 64;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r;
					resultImage[outputPixel+1] = workingImage[currentPixel].g;
					resultImage[outputPixel+2] = workingImage[currentPixel].b;
					resultImage[outputPixel+3] = 64;
					// if (invertOutput) {
					// 	resultImage[outputPixel] = PNG_MAX-min(max((currentUseCount), 0), PNG_MAX);
					// 	resultImage[outputPixel+1] = PNG_MAX-min(max((currentUseCount), 0), PNG_MAX);
					// 	resultImage[outputPixel+2] = PNG_MAX-min(max((currentUseCount), 0), PNG_MAX);
					// 	resultImage[outputPixel+3] = PNG_MAX;
					// } else {
					// 	resultImage[outputPixel] = min(max((currentUseCount), 0), PNG_MAX);
					// 	resultImage[outputPixel+1] = min(max((currentUseCount), 0), PNG_MAX);
					// 	resultImage[outputPixel+2] = min(max((currentUseCount), 0), PNG_MAX);
					// 	resultImage[outputPixel+3] = PNG_MAX;
					// }
				}
			}
		}
	}

	return resultImage;
}

#endif
