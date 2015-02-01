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
#include "libBinarization.c"
#include "libEnergies.c"
#include "libMinMax.c"

#define SEAM_TRACE_INCREMENT 16
#define THRESHHOLD_SOBEL 96
#define THRESHHOLD_USECOUNT 64

/*
 * Trace all the seams
 * The least signifigant pixels will be traced multiple times and have a higher value (whiter)
 * The most signifigant pixels will not be traced at all and have a value of zero (black)
 */
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
			for (int j = (imageSize - 1); j > 0; --j) {

				// THIS IS THE CRUCIAL PART
				if (imageVector[minValueLocation].usecount < (255-SEAM_TRACE_INCREMENT)) {
					imageVector[minValueLocation].usecount += SEAM_TRACE_INCREMENT;
					imageVector[minValueLocation].usecountR += SEAM_TRACE_INCREMENT;
					imageVector[minValueLocation].usecountG += SEAM_TRACE_INCREMENT;
					imageVector[minValueLocation].usecountB += SEAM_TRACE_INCREMENT;
				}

				// get the possible next pixles
				/*
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
				*/
	            if ((minValueLocation - nextPixelDistR) > 0) {
	                nextPixelR = imageVector[minValueLocation - nextPixelDistR].seamval;
	            } else {
	                nextPixelR = INT_MAX;
	            }
	            nextPixelC = imageVector[minValueLocation - nextPixelDistC].seamval;
	            if ((minValueLocation - nextPixelDistL) < loopEnd) {
	                nextPixelL = imageVector[minValueLocation - nextPixelDistL].seamval;
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
	//
	// This (below) is kinda a big deal
	// 
	if (imageVector[currentPixel].seamval > 0) {
		imageVector[currentPixel].seamval -= 1;
	}
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
		//
		// This (below) is kinda a big deal
		// 
		if (imageVector[currentPixel].seamval > 0) {
			imageVector[currentPixel].seamval -= 1;
		}
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

/*
 * The main function
 */
static int *seamCarve(int *imageVector, int imageWidth, int imageHeight, int imageDepth, int brightnessMode, int contrastMode, int forceDirection, int forceEdge, int preGauss)
{
	struct pixel *workingImageH = (struct pixel*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(struct pixel));
	struct pixel *workingImageV = (struct pixel*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * sizeof(struct pixel));
	int *resultImage = (int*)xmalloc((unsigned long)imageWidth * (unsigned long)imageHeight * (unsigned long)imageDepth * sizeof(int));
	
	int inputPixel = 0;
	int outputPixel = 0;
	int currentPixel = 0;
	// fill initial data structures
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			inputPixel = currentPixel * imageDepth;
			
			struct pixel newPixelH;
			newPixelH.r = imageVector[inputPixel];
			newPixelH.g = imageVector[inputPixel+1];
			newPixelH.b = imageVector[inputPixel+2];
			newPixelH.a = 0;//imageVector[inputPixel];
			newPixelH.bright = imageVector[inputPixel+3];
			newPixelH.gaussA = 0;
			newPixelH.gaussB = 0;
			newPixelH.energy = 0;
			newPixelH.seamval = 0;
			newPixelH.usecount = 0;
			newPixelH.usecountR = 0;
			newPixelH.usecountG = 0;
			newPixelH.usecountB = 0;
			workingImageH[currentPixel] = newPixelH;
			
			struct pixel newPixelV;
			newPixelV.r = imageVector[inputPixel];
			newPixelV.g = imageVector[inputPixel+1];
			newPixelV.b = imageVector[inputPixel+2];
			newPixelV.a = 0;//imageVector[inputPixel];
			newPixelV.bright = imageVector[inputPixel+3];
			newPixelV.gaussA = 0;
			newPixelV.gaussB = 0;
			newPixelV.energy = 0;
			newPixelV.seamval = 0;
			newPixelV.usecount = 0;
			newPixelV.usecountR = 0;
			newPixelV.usecountG = 0;
			newPixelV.usecountB = 0;
			workingImageV[currentPixel] = newPixelV;

			if (brightnessMode == 0) {
				// Intensity / Brightness
				newPixel.bright = ((imageVector[inputPixel] + imageVector[inputPixel+1] + imageVector[inputPixel+2]) / 3);
			} else if (brightnessMode == 1) {
				// HSV hexcone
				newPixel.bright = max3(imageVector[inputPixel], imageVector[inputPixel+1], imageVector[inputPixel+2]);
			} else if (brightnessMode == 2) {
				// Luma luminance -- sRGB / BT.709
				newPixel.bright = (imageVector[inputPixel] * 0.21) + (imageVector[inputPixel+1] * 0.72) + (imageVector[inputPixel+2] * 0.07);
			} else if (brightnessMode == 3) {
				// Luma luminance -- NTSC / BT.601
				//newPixel.bright = (imageVector[inputPixel] * 0.3) + (imageVector[inputPixel+1] * 0.59) + (imageVector[inputPixel+2] * 0.11);
				newPixel.bright = (imageVector[inputPixel] * 0.299) + (imageVector[inputPixel+1] * 0.587) + (imageVector[inputPixel+2] * 0.114);
			} else if (brightnessMode == 4) {
				// Relative luminance
				newPixel.bright = (imageVector[inputPixel] * 0.2126) + (imageVector[inputPixel+1] * 0.7152) + (imageVector[inputPixel+2] * 0.0722);
			} else if (brightnessMode == 5) {
				// HSP?
				newPixel.bright = sqrt((imageVector[inputPixel] * imageVector[inputPixel] * 0.299) + (imageVector[inputPixel+1] * imageVector[inputPixel+1] * 0.587) + (imageVector[inputPixel+2] * imageVector[inputPixel+2] * 0.114));
			} else if (brightnessMode == 6) {
				// Euclidian distance
				newPixel.bright = pow((imageVector[inputPixel] * imageVector[inputPixel]) + (imageVector[inputPixel+1] * imageVector[inputPixel+1]) + (imageVector[inputPixel+2] * imageVector[inputPixel+2]), 0.33333);
			}

			resultImage[inputPixel] = 0;
		}
	}

	
	// binarize the image as/if requested
	if (forceBinarization == 1) {
		// not really binarized, but brightness passed though a cosine function
		// this increases differentiation between light and dark pixels
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImageH[currentPixel].bright;
				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				workingImageH[currentPixel].bright = currentBrightness;
				workingImageV[currentPixel].bright = currentBrightness;
			}
		}

	} else if (forceBinarization == 2) {
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImageH[currentPixel].bright;
				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				workingImageH[currentPixel].bright = currentBrightness;
				workingImageV[currentPixel].bright = currentBrightness;
			}
		}

	} else if (forceBinarization == 3) {
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImageH[currentPixel].bright;
				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				workingImageH[currentPixel].bright = currentBrightness;
				workingImageV[currentPixel].bright = currentBrightness;
			}
		}

	} else if (forceBinarization == 4) {
		int currentBrightness = 0;
		double currentRadians = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;

				currentBrightness = workingImageH[currentPixel].bright;
				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				currentRadians = ((double)currentBrightness / 255.0) * 3.14159265359;
				currentBrightness = (int)(((1.0 - cos(currentRadians)) / 2.0) * 255.0);

				workingImageH[currentPixel].bright = currentBrightness;
				workingImageV[currentPixel].bright = currentBrightness;
			}
		}

	} else if (forceBinarization == 5) {
		// TODO: merge with above loops and below loops (split)
		
		int bins[256];
		int currentBrightness = 0;

		// get historgram
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				currentBrightness = workingImageH[currentPixel].bright;
				bins[currentBrightness] += 1;
			}
		}

		int threshold = otsuBinarization(bins, (imageWidth * imageHeight));

		// apply threshold
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				currentBrightness = workingImageH[currentPixel].bright;
				if (currentBrightness > threshold) {
					workingImageH[currentPixel].bright = 255;
				} else {
					workingImageH[currentPixel].bright = 0;
				}
			}
		}
	}

	int tmpDoG = 0;
	int tmpSobel = 0;

	// get energy values using the prescribed method
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;

			if (preGauss == 1) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 20);
				workingImageV[currentPixel].bright = workingImageH[currentPixel].bright;
			} else if (preGauss == 2) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 40);
				workingImageV[currentPixel].bright = workingImageH[currentPixel].bright;
			} else if (preGauss == 3) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 80);
				workingImageV[currentPixel].bright = workingImageH[currentPixel].bright;
			}

			if (forceEdge == 1) {
				// workingImageH[currentPixel].gaussA = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 12);
				// workingImageH[currentPixel].gaussB = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 13);
				workingImageH[currentPixel].gaussA = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 14);
				workingImageH[currentPixel].gaussB = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 16);
				
				workingImageV[currentPixel].gaussA = workingImageH[currentPixel].gaussA;
				workingImageV[currentPixel].gaussB = workingImageH[currentPixel].gaussB;

				workingImageH[currentPixel].energy = getPixelEnergyDoG(workingImageH, currentPixel);
			} else if (forceEdge == 2) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 80);
				workingImageH[currentPixel].energy = sqrt(getPixelEnergyLaplacian(workingImageH, imageWidth, imageHeight, currentPixel));
			} else if (forceEdge == 3) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 40);
				workingImageH[currentPixel].energy = sqrt(getPixelEnergyLaplacian(workingImageH, imageWidth, imageHeight, currentPixel));
			} else if (forceEdge == 4) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 20);
				workingImageH[currentPixel].energy = sqrt(getPixelEnergyLaplacian(workingImageH, imageWidth, imageHeight, currentPixel));
			} else if (forceEdge == 5) {
				workingImageH[currentPixel].energy = getPixelEnergySobel(workingImageH, imageWidth, imageHeight, currentPixel);
			} else if (forceEdge == 6) {
				workingImageH[currentPixel].energy = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 9999);
			} else if (forceEdge == 7) {
				workingImageH[currentPixel].energy = getPixelEnergySimple(workingImageH, imageWidth, imageHeight, currentPixel, 1);
			} else if (forceEdge == 8) {
				workingImageH[currentPixel].gaussA = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 14);
				workingImageH[currentPixel].gaussB = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 16);
				
				workingImageV[currentPixel].gaussA = workingImageH[currentPixel].gaussA;
				workingImageV[currentPixel].gaussB = workingImageH[currentPixel].gaussB;

				tmpDoG = getPixelEnergyDoG(workingImageH, currentPixel);
				tmpSobel = getPixelEnergySobel(workingImageH, imageWidth, imageHeight, currentPixel);

				//workingImageH[currentPixel].energy = (tmpDoG + ((tmpSobel / 100) * 20));
				workingImageH[currentPixel].energy = (tmpDoG + ((tmpSobel / 80) * 20)) / 2;
			} else if (forceEdge == 9) {
				workingImageH[currentPixel].bright = getPixelGaussian(workingImageH, imageWidth, imageHeight, 1, currentPixel, 80);
				tmpDoG = sqrt(getPixelEnergyLaplacian(workingImageH, imageWidth, imageHeight, currentPixel));
				tmpSobel = getPixelEnergySobel(workingImageH, imageWidth, imageHeight, currentPixel);

				if (tmpDoG && tmpSobel) {
					workingImageH[currentPixel].energy = (tmpDoG + (tmpSobel / 24));
					//printf("%d + (%d / 20) = %d \n", tmpDoG, tmpSobel, workingImageH[currentPixel].energy);
				} else {
					workingImageH[currentPixel].energy = 0;
				}
			}

			workingImageH[currentPixel].seamval = workingImageH[currentPixel].energy;
			workingImageV[currentPixel].energy = workingImageH[currentPixel].energy;
			workingImageV[currentPixel].seamval = workingImageV[currentPixel].energy;
			
			// also grab sobel energy
			/*
			workingImageH[currentPixel].sobelA = getPixelEnergySobel(workingImageH, imageWidth, imageHeight, currentPixel);
			workingImageV[currentPixel].sobelA = workingImageH[currentPixel].sobelA;

			if (workingImageH[currentPixel].sobelA > THRESHHOLD_SOBEL) {
				workingImageH[currentPixel].energy = workingImageH[currentPixel].sobelA / 4;
				workingImageH[currentPixel].seamval = workingImageH[currentPixel].energy;
				workingImageV[currentPixel].energy = workingImageH[currentPixel].sobelA / 4;
				workingImageV[currentPixel].seamval = workingImageV[currentPixel].energy;
			}
			*/
		}
	}

	/*
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			workingImageH[currentPixel].energy = getPixelEnergySobel(workingImageH, imageWidth, imageHeight, currentPixel);
		}
	}
	*/

	// find seams in the prescribed direction
	int resultDirection = 0;
	if (forceDirection == 1) {
		fillSeamMatrixHorizontal(workingImageH, imageWidth, imageHeight);
		findSeamsHorizontal(workingImageH, imageWidth, imageHeight);
		resultDirection = 1;
	} else if (forceDirection == 2) {
		fillSeamMatrixVertical(workingImageV, imageWidth, imageHeight);
		findSeamsVertical(workingImageV, imageWidth, imageHeight);
		resultDirection = 2;
	} else if (forceDirection >= 3) {
		fillSeamMatrixHorizontal(workingImageH, imageWidth, imageHeight);
		fillSeamMatrixVertical(workingImageV, imageWidth, imageHeight);

		findSeamsHorizontal(workingImageH, imageWidth, imageHeight);
		findSeamsVertical(workingImageV, imageWidth, imageHeight);

		resultDirection = forceDirection;
	} else {
		int horizontalSeamCost = fillSeamMatrixHorizontal(workingImageH, imageWidth, imageHeight);
		int verticalSeamCost = fillSeamMatrixVertical(workingImageV, imageWidth, imageHeight);
		printf("Sum traversal cost of all seams: horizontal = %d, vertical = %d \n", verticalSeamCost, horizontalSeamCost);

		findSeamsHorizontal(workingImageH, imageWidth, imageHeight);
		findSeamsVertical(workingImageV, imageWidth, imageHeight);
		
		if (horizontalSeamCost < verticalSeamCost) {
			resultDirection = 1;
		} else {
			resultDirection = 2;
		}
	}

	// prepare results for output
	if (resultDirection == 1) {
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImageH[currentPixel].usecount > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = workingImageH[currentPixel].r;
					resultImage[outputPixel+1] = workingImageH[currentPixel].g;
					resultImage[outputPixel+2] = workingImageH[currentPixel].b;
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 2) {
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;

				if (workingImageV[currentPixel].usecount > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = workingImageV[currentPixel].r;
					resultImage[outputPixel+1] = workingImageV[currentPixel].g;
					resultImage[outputPixel+2] = workingImageV[currentPixel].b;
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
				
				currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				if (currentUseCount > THRESHHOLD_USECOUNT) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = workingImageH[currentPixel].r /1;
					resultImage[outputPixel+1] = workingImageH[currentPixel].g /1;
					resultImage[outputPixel+2] = workingImageH[currentPixel].b /1;
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
				
				currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				if (currentUseCount > 256) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = min(max(workingImageH[currentPixel].bright, 0), 255);
					resultImage[outputPixel+1] = min(max(workingImageH[currentPixel].bright, 0), 255);
					resultImage[outputPixel+2] = min(max(workingImageH[currentPixel].bright, 0), 255);
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 5) {
		int energyScale = 16;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				if (currentUseCount > 256) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = min(max((workingImageH[currentPixel].energy * energyScale), 0), 255);
					resultImage[outputPixel+1] = min(max((workingImageH[currentPixel].energy * energyScale), 0), 255);
					resultImage[outputPixel+2] = min(max((workingImageH[currentPixel].energy * energyScale), 0), 255);
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 6) {
		int seamValueScale = 16;
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				if (currentUseCount > 256) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = min(max((workingImageH[currentPixel].seamval * seamValueScale), 0), 255);
					resultImage[outputPixel+1] = min(max((workingImageH[currentPixel].seamval * seamValueScale), 0), 255);
					resultImage[outputPixel+2] = min(max((workingImageH[currentPixel].seamval * seamValueScale), 0), 255);
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 7) {
		int seamValueScale = 4;
		int currentUseCount = 0;
		
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				if (currentUseCount > 256) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = 255-min(max((workingImageH[currentPixel].usecountR), 0), 255);
					resultImage[outputPixel+1] = 255-min(max((workingImageH[currentPixel].usecountG), 0), 255);
					resultImage[outputPixel+2] = 255-min(max((workingImageH[currentPixel].usecountB), 0), 255);
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
				
				currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				if (currentUseCount > 256) {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = 255-min(max((workingImageV[currentPixel].usecountR), 0), 255);
					resultImage[outputPixel+1] = 255-min(max((workingImageV[currentPixel].usecountG), 0), 255);
					resultImage[outputPixel+2] = 255-min(max((workingImageV[currentPixel].usecountB), 0), 255);
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	} else if (resultDirection == 99) {
		int seamValueScale = 4;
		
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				if ((workingImageH[currentPixel].usecountR == 0) && (workingImageH[currentPixel].usecountG == 0) && (workingImageH[currentPixel].usecountB == 0)) {
					resultImage[outputPixel] = min(max((workingImageH[currentPixel].seamval * 4), 0), 255);
					resultImage[outputPixel+1] = min(max((workingImageH[currentPixel].seamval * 4), 0), 255);
					resultImage[outputPixel+2] = min(max((workingImageH[currentPixel].seamval * 4), 0), 255);
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = min(max((workingImageH[currentPixel].usecountR), 0), 255);
					resultImage[outputPixel+1] = 0;//min(max((workingImageH[currentPixel].usecountG), 0), 255);
					resultImage[outputPixel+2] = 0;//min(max((workingImageH[currentPixel].usecountB), 0), 255);
					resultImage[outputPixel+3] = 255;
				}
			}
		}

	} else if (resultDirection == 99) {
		int seamValueScale = 4;
		int currentUseCount = 0;
		
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				// currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				currentUseCount = workingImageH[currentPixel].usecount;
				if (currentUseCount == 0) {
					resultImage[outputPixel] = 0;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 255;
					resultImage[outputPixel+2] = 255;
					resultImage[outputPixel+3] = 0;
				}
			}
		}

	} else if (resultDirection == 99) {
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				//currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				currentUseCount = min(max((workingImageH[currentPixel].usecount + workingImageV[currentPixel].usecount), 0), 255);
				resultImage[outputPixel] = 255-currentUseCount;
				resultImage[outputPixel+1] = 255-currentUseCount;
				resultImage[outputPixel+2] = 255-currentUseCount;
				resultImage[outputPixel+3] = 255;
			}
		}
	} else if (resultDirection == 9) {
		int currentUseCount = 0;
		for (int j = 0; j < imageHeight; ++j) {
			for (int i = 0; i < imageWidth; ++i) {
				currentPixel = (j * imageWidth) + i;
				outputPixel = currentPixel * imageDepth;
				
				//currentUseCount = max(workingImageH[currentPixel].usecount, workingImageV[currentPixel].usecount);
				currentUseCount = min(max((workingImageH[currentPixel].usecount + workingImageV[currentPixel].usecount), 0), 255);
				if (!currentUseCount) {
					resultImage[outputPixel] = 0;
					resultImage[outputPixel+1] = 0;
					resultImage[outputPixel+2] = 0;
					resultImage[outputPixel+3] = 255;
				} else {
					resultImage[outputPixel] = 255;
					resultImage[outputPixel+1] = 255;
					resultImage[outputPixel+2] = 255;
					resultImage[outputPixel+3] = 255;
				}
			}
		}
	}

	return resultImage;
}

#endif
