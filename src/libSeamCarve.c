/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014 -- v3, major refactoring
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

#define SEAM_TRACE_INCREMENT 16
#define THRESHHOLD_SOBEL 96
#define THRESHHOLD_USECOUNT 64
#define PI 3.14159265359

/*
 * Trace all the seams
 * The least signifigant pixels will be traced multiple times and have a higher value (whiter)
 * The most signifigant pixels will not be traced at all and have a value of zero (black)
 */
static void findSeams(struct pixel *imageVector, struct window *imageWindow, int direction)
{
	// TODO: create macro definition
	int directionVertical = 0;
	int directionHorizontal = 1;
	if ((direction != directionVertical) && (direction != directionHorizontal)) {
		return;
	}

	int loopBeg = 0; // where the outer loop begins
	int loopEnd = 0; // where the outer loop ends
	int loopInc = 0; // the increment of the outer loop
	int loopInBeg = 0;
	int loopInEnd = 0;
	int loopInInc = 0;

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
	if (direction == directionVertical) {
		loopBeg = imageWindow->lastPixel - 1 - imageWindow->xLength;
		loopEnd = imageWindow->lastPixel - 1;
		loopInc = imageWindow->xStep;
		printf("%d, %d, %d \n", loopBeg, loopEnd, loopInc);

		// also set the next pixel distances
		nextPixelDistC = imageWindow->fullWidth;
		nextPixelDistR = nextPixelDistC - 1;
		nextPixelDistL = nextPixelDistC + 1;

		loopInBeg = imageWindow->yTerminus - 1;
		loopInEnd = imageWindow->yOrigin;
		loopInInc = imageWindow->xStep;
		printf("%d, %d, %d \n", loopInBeg, loopInEnd, loopInInc);
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
	}

	int minValueLocation = 0;
	// for every pixel in the right-most or bottom-most column of the image
	for (int k = loopBeg; k < loopEnd; k += loopInc) {
		// process seams with the lowest weights
		// start from the left-most column
		minValueLocation = k;
		countGoR = 0;
		countGoL = 0;

		// move right-to-left ot bottom-to-top across/up the image
		for (int j = loopInBeg; j > loopInEnd; j -= loopInInc) {
			// THIS IS THE CRUCIAL PART
			if (direction == directionVertical) {
				if (imageVector[minValueLocation].usecountV < (255-SEAM_TRACE_INCREMENT)) {
					imageVector[minValueLocation].usecountV += SEAM_TRACE_INCREMENT;
				}
			} else {
				if (imageVector[minValueLocation].usecountH < (255-SEAM_TRACE_INCREMENT)) {
					imageVector[minValueLocation].usecountH += SEAM_TRACE_INCREMENT;
				}
			}

			// get the possible next pixles
            if ((minValueLocation - nextPixelDistR) > 0) {
            	if (direction == directionVertical) {
                	nextPixelR = imageVector[minValueLocation - nextPixelDistR].seamvalV;
                } else {
                	nextPixelR = imageVector[minValueLocation - nextPixelDistR].seamvalH;
                }
            } else {
                nextPixelR = INT_MAX;
            }

            if (direction == directionVertical) {
	            nextPixelC = imageVector[minValueLocation - nextPixelDistC].seamvalV;
	        } else {
	        	nextPixelC = imageVector[minValueLocation - nextPixelDistC].seamvalH;
	        }
            
            if ((minValueLocation - nextPixelDistL) < loopEnd) {
                if (direction == directionVertical) {
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
				} else if (currentMin == nextPixelL) {
					minValueLocation -= nextPixelDistL;
					++countGoL;
				}
			} else if (countGoR > countGoL) {
				if (currentMin == nextPixelL) {
					minValueLocation -= nextPixelDistL;
					++countGoL;
				} else if (currentMin == nextPixelC) {
					minValueLocation -= nextPixelDistC;
				} else if (currentMin == nextPixelR) {
					minValueLocation -= nextPixelDistR;
					++countGoR;
				}
			} else if (countGoR < countGoL) {
				if (currentMin == nextPixelR) {
					minValueLocation -= nextPixelDistR;
					++countGoR;
				} else if (currentMin == nextPixelC) {
					minValueLocation -= nextPixelDistC;
				} else if (currentMin == nextPixelL) {
					minValueLocation -= nextPixelDistL;
					++countGoL;
				}
			}
		}
	}
}

static void setPixelPathVertical(struct pixel *imageVector, struct window *imageWindow, int currentPixel, int currentCol)
{
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

static void findSeamsVertical(struct pixel *imageVector, struct window *imageWindow)
{
	findSeams(imageVector, imageWindow, 0);
}

static void setPixelPathHorizontal(struct pixel *imageVector, struct window *imageWindow, int currentPixel, int currentCol)
{
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

static void findSeamsHorizontal(struct pixel *imageVector, struct window *imageWindow)
{
	findSeams(imageVector, imageWindow, 1);
}

/*
 * The main function
 */
static int *seamCarve(int *imageVector, int imageWidth, int imageHeight, int imageDepth, int brightnessMode, int contrastMode, int forceDirection, int forceEdge, int preGauss)
{
	struct pixel *workingImage = (struct pixel*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(struct pixel));
	int *resultImage = (int*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * (unsigned long)imageDepth * sizeof(int));
	
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
			}

			newPixel.energy = 0;
			newPixel.seamvalH = 0;
			newPixel.seamvalV = 0;
			newPixel.usecountH = 0;
			newPixel.usecountV = 0;
			workingImage[currentPixel] = newPixel;

			resultImage[inputPixel] = 0;
		}
	}

	
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
				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);
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
				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

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
				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

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
				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * PI;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

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
					workingImage[currentPixel].bright = 255;
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
				workingImage[currentPixel].energy = getPixelEnergySobel(workingImage, imageWidth, imageHeight, currentPixel);
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
		printf("Sum traversal cost of all seams: horizontal = %d, vertical = %d \n", verticalSeamCost, horizontalSeamCost);

		findSeamsHorizontal(workingImage, currentWindow);
		findSeamsVertical(workingImage, currentWindow);
		
		if (horizontalSeamCost < verticalSeamCost) {
			resultDirection = 1;
		} else {
			resultDirection = 2;
		}
	} else if ((forceDirection == 1) || (forceDirection == 6) || (forceDirection == 8)) {
		fillSeamMatrixHorizontal(workingImage, currentWindow);
		findSeamsHorizontal(workingImage, currentWindow);
	} else if ((forceDirection == 2) || (forceDirection == 7) || (forceDirection == 9)) {
		fillSeamMatrixVertical(workingImage, currentWindow);
		findSeamsVertical(workingImage, currentWindow);
	} else if ((forceDirection == 4) || (forceDirection == 5)) {
		// pass
	} else {
		fillSeamMatrixHorizontal(workingImage, currentWindow);
		fillSeamMatrixVertical(workingImage, currentWindow);

		findSeamsHorizontal(workingImage, currentWindow);
		findSeamsVertical(workingImage, currentWindow);
	}

	// prepare results for output
	if (resultDirection == 1) {
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImage[currentPixel].usecountH > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r / 2;
					resultImage[outputPixel+1] = workingImage[currentPixel].g / 2;
					resultImage[outputPixel+2] = workingImage[currentPixel].b / 2;
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 2) {
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImage[currentPixel].usecountV > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r / 2;
					resultImage[outputPixel+1] = workingImage[currentPixel].g / 2;
					resultImage[outputPixel+2] = workingImage[currentPixel].b / 2;
					resultImage[outputPixel+3] = 255;
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
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = workingImage[currentPixel].r / 2;
					resultImage[outputPixel+1] = workingImage[currentPixel].g / 2;
					resultImage[outputPixel+2] = workingImage[currentPixel].b / 2;
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 4) {
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				resultImage[outputPixel] = min(max(workingImage[currentPixel].bright, 0), 255);
				resultImage[outputPixel+1] = min(max(workingImage[currentPixel].bright, 0), 255);
				resultImage[outputPixel+2] = min(max(workingImage[currentPixel].bright, 0), 255);
				resultImage[outputPixel+3] = 255;
			}
		}
	} else if (resultDirection == 5) {
		int energyScale = 16;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				resultImage[outputPixel] = min(max((workingImage[currentPixel].energy * energyScale), 0), 255);
				resultImage[outputPixel+1] = min(max((workingImage[currentPixel].energy * energyScale), 0), 255);
				resultImage[outputPixel+2] = min(max((workingImage[currentPixel].energy * energyScale), 0), 255);
				resultImage[outputPixel+3] = 255;
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
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = min(max((workingImage[currentPixel].seamvalH * seamValueScale), 0), 255);
					resultImage[outputPixel+1] = min(max((workingImage[currentPixel].seamvalH * seamValueScale), 0), 255);
					resultImage[outputPixel+2] = min(max((workingImage[currentPixel].seamvalH * seamValueScale), 0), 255);
					resultImage[outputPixel+3] = 255;
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
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = min(max((workingImage[currentPixel].seamvalV * seamValueScale), 0), 255);
					resultImage[outputPixel+1] = min(max((workingImage[currentPixel].seamvalV * seamValueScale), 0), 255);
					resultImage[outputPixel+2] = min(max((workingImage[currentPixel].seamvalV * seamValueScale), 0), 255);
					resultImage[outputPixel+3] = 255;
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
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = 255-min(max((workingImage[currentPixel].usecountH), 0), 255);
					resultImage[outputPixel+1] = 255-min(max((workingImage[currentPixel].usecountH), 0), 255);
					resultImage[outputPixel+2] = 255-min(max((workingImage[currentPixel].usecountH), 0), 255);
					resultImage[outputPixel+3] = 255;
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
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = 255-min(max((workingImage[currentPixel].usecountV), 0), 255);
					resultImage[outputPixel+1] = 255-min(max((workingImage[currentPixel].usecountV), 0), 255);
					resultImage[outputPixel+2] = 255-min(max((workingImage[currentPixel].usecountV), 0), 255);
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	}

	return resultImage;
}

#endif
