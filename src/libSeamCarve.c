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
#include "libWrappers.c"
#include "libEnergies.c"
#include "libMinMax.c"

#define SEAM_TRACE_INCREMENT 16

static void findSeams(struct pixel *imageVector, int imageWidth, int imageHeight, int direction)
{
	// TODO: create macro definition
	int directionVertical = 0;
	int directionHorizontal = 1;
	if ((direction != directionVertical) && (direction != directionHorizontal)) {
		return;
	}

	int imageSize = 0; // width when going horizontal, height when going vertical
	int loopBeg = 0; // where the outer loop begins
	int loopEnd = 0; // where the outer loop ends
	int loopInc = 0; // the increment of the outer loop

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
		loopBeg = (imageWidth * imageHeight) - 1 - imageWidth;
		loopEnd = (imageWidth * imageHeight) - 1;
		loopInc = 1;

		// also set the next pixel distances
		nextPixelDistR = imageWidth - 1;
		nextPixelDistC = imageWidth;
		nextPixelDistL = imageWidth + 1;

		imageSize = imageHeight;
	} else {
		loopBeg = imageWidth - 1;
		loopEnd = (imageWidth * imageHeight) - 0;//1;
		loopInc = imageWidth;

		// also set the next pixel distances
		nextPixelDistR = imageWidth + 1;
		nextPixelDistC = 1;
		nextPixelDistL = (imageWidth - 1) * -1;

		imageSize = imageWidth;
	}

	int minValueLocation = 0;
	// for every pixel in the right-most or bottom-most column of the image
	for (int k = loopBeg; k < loopEnd; k += loopInc) {
		// process seams with the lowest weights
		//if (imageVector[k].seamval <= minValue) {
			// start from the left-most column
			minValueLocation = k;
			countGoR = 0;
			countGoL = 0;

			// move right-to-left ot bottom-to-top across/up the image
			//printf("%d\n", (imageSize - 1));
			for (int j = (imageSize - 1); j >= 0; --j) {

				if (imageVector[minValueLocation].usecount < (255-SEAM_TRACE_INCREMENT)) {
					imageVector[minValueLocation].usecount += SEAM_TRACE_INCREMENT;
				}
				// printf("%d\n", imageVector[minValueLocation].usecount);

				// get the possible next pixles
				if (direction == directionVertical) {
					if (((minValueLocation - imageWidth + 1) % imageWidth) != 0) {
						nextPixelR = imageVector[minValueLocation - imageWidth + 1].seamval;
					} else {
						nextPixelR = INT_MAX;
					}
					nextPixelC = imageVector[minValueLocation - imageWidth].seamval;
					nextPixelL = imageVector[minValueLocation - imageWidth - 1].seamval;
				} else {
					nextPixelR = imageVector[minValueLocation - 1 - imageWidth].seamval;
					nextPixelC = imageVector[minValueLocation - 1].seamval;
					if ((k + 1) != loopEnd) {
						nextPixelL = imageVector[minValueLocation - 1 + imageWidth].seamval;
					} else {
						nextPixelL = INT_MAX;
					}
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
		//}
	}
}

static void setPixelPathVertical(struct pixel *imageVector, int imageWidth, int currentPixel, int currentCol)
{
	int pixelAbove = 0;
	int aboveL = 0;
	int aboveC = 0;
	int aboveR = 0;
	int newValue = 0;

	pixelAbove = currentPixel - imageWidth;
	// avoid falling off the left end
	if (currentCol > 0) {
		// avoid falling off the right end
		if (currentCol < imageWidth) {
			aboveL = imageVector[pixelAbove - 1].seamval;
			aboveC = imageVector[pixelAbove].seamval;
			aboveR = imageVector[pixelAbove + 1].seamval;
			newValue = min3(aboveL, aboveC, aboveR);
		} else {
			aboveL = imageVector[pixelAbove - 1].seamval;
			aboveC = imageVector[pixelAbove].seamval;
			aboveR = INT_MAX;
			newValue = min(aboveL, aboveC);
		}
	} else {
		aboveL = INT_MAX;
		aboveC = imageVector[pixelAbove].seamval;
		aboveR = imageVector[pixelAbove + 1].seamval;
		newValue = min(aboveC, aboveR);
	}
	imageVector[currentPixel].seamval += newValue;
}

static int fillSeamMatrixVertical(struct pixel *imageVector, int imageWidth, int imageHeight)
{
	int result = 0;
	int currentPixel = 0;
	// do not process the first row, start with j=1
	for (int j = 1; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			setPixelPathVertical(imageVector, imageWidth, currentPixel, i);

			if (imageVector[currentPixel].seamval != 0) {
				++result;
			}
		}
	}
	return result;
}

static void findSeamsVertical(struct pixel *imageVector, int imageWidth, int imageHeight)
{
	findSeams(imageVector, imageWidth, imageHeight, 0);
}

static void setPixelPathHorizontal(struct pixel *imageVector, int imageWidth, int imageHeight, int currentPixel, int currentCol)
{
	// avoid falling off the right
	if (currentCol < imageWidth) {
		int pixelLeft = 0;
		int leftT = 0;
		int leftM = 0;
		int leftB = 0;
		int newValue = 0;

		pixelLeft = currentPixel - 1;
		// avoid falling off the top
		if (currentPixel > imageWidth) {
			// avoid falling off the bottom
			if (currentPixel < ((imageWidth * imageHeight) - imageWidth)) {
				leftT = imageVector[pixelLeft - imageWidth].seamval;
				leftM = imageVector[pixelLeft].seamval;
				leftB = imageVector[pixelLeft + imageWidth].seamval;
				newValue = min3(leftT, leftM, leftB);
			} else {
				leftT = imageVector[pixelLeft - imageWidth].seamval;
				leftM = imageVector[pixelLeft].seamval;
				leftB = INT_MAX;
				newValue = min(leftT, leftM);
			}
		} else {
			leftT = INT_MAX;
			leftM = imageVector[pixelLeft].seamval;
			leftB = imageVector[pixelLeft + imageWidth].seamval;
			newValue = min(leftM, leftB);
		}
		imageVector[currentPixel].seamval += newValue;
	}
}

static int fillSeamMatrixHorizontal(struct pixel *imageVector, int imageWidth, int imageHeight)
{
	int result = 0;
	int currentPixel = 0;
	// do not process the first row, start with j=1
	// must be in reverse order from verticle seam, calulate colums as we move across (top down, left to right)
	for (int i = 0; i < imageWidth; ++i) {
		for (int j = 1; j < imageHeight; ++j) {
			currentPixel = (j * imageWidth) + i;
			setPixelPathHorizontal(imageVector, imageWidth, imageHeight, currentPixel, i);

			if (imageVector[currentPixel].seamval != 0) {
				++result;
			}
		}
	}
	return result;
}

static void findSeamsHorizontal(struct pixel *imageVector, int imageWidth, int imageHeight)
{
	findSeams(imageVector, imageWidth, imageHeight, 1);
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight, int forceDirection)
{
	struct pixel *workingImageH = (struct pixel*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(struct pixel));
	struct pixel *workingImageV = (struct pixel*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(struct pixel));
	int *resultImage = (int*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(int));
	
	int currentPixel = 0;
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			
			struct pixel newPixelH;
			newPixelH.r = imageVector[currentPixel];
			newPixelH.g = imageVector[currentPixel];
			newPixelH.b = imageVector[currentPixel];
			newPixelH.a = imageVector[currentPixel];
			newPixelH.bright = imageVector[currentPixel];
			newPixelH.gaussA = 0;
			newPixelH.gaussB = 0;
			newPixelH.energy = 0;
			newPixelH.seamval = 0;
			newPixelH.usecount = 0;
			workingImageH[currentPixel] = newPixelH;
			
			struct pixel newPixelV;
			newPixelV.r = imageVector[currentPixel];
			newPixelV.g = imageVector[currentPixel];
			newPixelV.b = imageVector[currentPixel];
			newPixelV.a = imageVector[currentPixel];
			newPixelV.bright = imageVector[currentPixel];
			newPixelV.gaussA = 0;
			newPixelV.gaussB = 0;
			newPixelV.energy = 0;
			newPixelV.seamval = 0;
			newPixelV.usecount = 0;
			workingImageV[currentPixel] = newPixelV;

			resultImage[currentPixel] = 0;
		}
	}

	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			workingImageH[currentPixel].gaussA = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 10);
			workingImageH[currentPixel].gaussB = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 14);

			workingImageV[currentPixel].gaussA = workingImageH[currentPixel].gaussA;
			workingImageV[currentPixel].gaussB = workingImageH[currentPixel].gaussB;
		}
	}

	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			//workingImageH[currentPixel].energy = getPixelEnergySobel(workingImageH, imageWidth, imageHeight, currentPixel);
			workingImageH[currentPixel].energy = getPixelEnergyDoG(workingImageH, currentPixel);
			workingImageH[currentPixel].seamval = workingImageH[currentPixel].energy;

			workingImageV[currentPixel].energy = workingImageH[currentPixel].energy;
			workingImageV[currentPixel].seamval = workingImageV[currentPixel].energy;
		}
	}
	

	if (forceDirection == 1) {
		fillSeamMatrixHorizontal(workingImageH, imageWidth, imageHeight);
		findSeamsHorizontal(workingImageH, imageWidth, imageHeight);

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				resultImage[currentPixel] = workingImageH[currentPixel].usecount;
			}
		}

	} else if (forceDirection == 2) {
		fillSeamMatrixVertical(workingImageV, imageWidth, imageHeight);
		findSeamsVertical(workingImageV, imageWidth, imageHeight);

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				resultImage[currentPixel] = workingImageV[currentPixel].usecount;
			}
		}

	} else if (forceDirection == 3) {
		int horizontalSeamCost = fillSeamMatrixHorizontal(workingImageH, imageWidth, imageHeight);
		int verticalSeamCost = fillSeamMatrixVertical(workingImageV, imageWidth, imageHeight);

		findSeamsHorizontal(workingImageH, imageWidth, imageHeight);
		findSeamsVertical(workingImageV, imageWidth, imageHeight);

		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				resultImage[currentPixel] = workingImageH[currentPixel].usecount + workingImageV[currentPixel].usecount;
				//resultImage[currentPixel] = (workingImageH[currentPixel].usecount + workingImageV[currentPixel].usecount) / 2;
				/*
				if ((workingImageH[currentPixel].usecount >= 64) || (workingImageV[currentPixel].usecount >= 64)) {
					resultImage[currentPixel] = 255;
				} else {
					resultImage[currentPixel] = 0;
				}
				*/
			}
		}

	} else {
		int horizontalSeamCost = fillSeamMatrixHorizontal(workingImageH, imageWidth, imageHeight);
		int verticalSeamCost = fillSeamMatrixVertical(workingImageV, imageWidth, imageHeight);
		printf("Sum traversal cost of all seams: horizontal = %d, vertical = %d \n", verticalSeamCost, horizontalSeamCost);

		findSeamsHorizontal(workingImageH, imageWidth, imageHeight);
		findSeamsVertical(workingImageV, imageWidth, imageHeight);
		
		if (horizontalSeamCost < verticalSeamCost) {
			for (int j = 0; j < imageHeight; ++j) {
				for (int i = 0; i < imageWidth; ++i) {
					currentPixel = (j * imageWidth) + i;
					resultImage[currentPixel] = workingImageH[currentPixel].seamval;//usecount;
				}
			}
		} else {
			for (int j = 0; j < imageHeight; ++j) {
				for (int i = 0; i < imageWidth; ++i) {
					currentPixel = (j * imageWidth) + i;
					resultImage[currentPixel] = workingImageV[currentPixel].seamval;//usecount;
				}
			}
		}
	}
	return resultImage;
}

#endif
