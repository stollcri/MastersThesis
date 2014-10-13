/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBSEAMCARVE_C
#define LIBSEAMCARVE_C

#include <stdio.h>
#include <limits.h>
#include "libWrappers.c"
#include "libResize.c"

static inline int max(int a, int b)
{
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

static inline int min(int a, int b)
{
	if (a < b) {
		return a;
	} else {
		return b;
	}
}

static inline int min3(int a, int b, int c)
{
	if (a < b) {
		if (a < c) {
			return a;
		} else {
			return c;
		}
	} else {
		if (b < c) {
			return b;
		} else {
			return c;
		}
	}
}

// Simple energy function, basically a gradient magnitude calculation
static int getPixelEnergySimple(int *imageVector, int imageWidth, int imageHeight, int currentPixel, int gradientSize)
{
	// We can pull from two pixels above instead of summing one above and one below
	int pixelAbove = 0;
	if (currentPixel > (imageWidth * gradientSize)) {
		pixelAbove = currentPixel - (imageWidth * gradientSize);
	}

	int yDif = 0;
	if (imageVector[pixelAbove] > imageVector[currentPixel]) {
		yDif = imageVector[pixelAbove] - imageVector[currentPixel];
	} else {
		yDif = imageVector[currentPixel] - imageVector[pixelAbove];
	}

	int pixelLeft = 0;
	// TODO: fix this from rolling back to the other side?
	pixelLeft = currentPixel - gradientSize;
	if (pixelLeft < 0) {
		pixelLeft = 0;
	}

	int xDif = 0;
	if (imageVector[pixelLeft] > imageVector[currentPixel]) {
		xDif = imageVector[pixelLeft] - imageVector[currentPixel];
	} else {
		xDif = imageVector[currentPixel] - imageVector[pixelLeft];
	}

	return min((yDif + xDif), 255);
}

static void printSeam(int *seamPath, int *image, int imageWidth, int imageHeight, int direction, int grayScale)
{
	// TODO: create macro definition
	int directionVertical = 0;
	int directionHorizontal = 1;
	if ((direction != directionVertical) && (direction != directionHorizontal)) {
		return;
	}

	int currentPixel = 0;
	if (direction == directionVertical) {
		for (int j = imageHeight; j > 0; --j) {
			currentPixel = seamPath[j];
			image[currentPixel] = grayScale;
		}
	} else {
		for (int j = (imageWidth - 1); j >= 0; --j) {
			currentPixel = seamPath[j];
			image[currentPixel] = grayScale;
		}
	}
}

//static int findSeamsHorizontal(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig)
static int findSeams(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig, int direction)
{
	// TODO: create macro definition
	int directionVertical = 0;
	int directionHorizontal = 1;
	if ((direction != directionVertical) && (direction != directionHorizontal)) {
		return -1;
	}
	int loopBeg = 0;
	int loopEnd = 0;
	int loopInc = 0;
	int innerLoopBeg = 0;
	int innerLoopEnd = 0;
	int innerLoopInc = 0;

	// the return value of the function, the sum traversal cost of all seams
	int totalSeamValue = 0;
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;

	// loop conditions depend upon the direction
	if (direction == directionVertical) {
		for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
			totalSeamValue += imageSeams[i]; 
			
			if (imageSeams[i] < minValue) {
				minValue = imageSeams[i];
				minValueLocation = i;
			}
			// TODO: break if min value is zero?
		}
	} else {
		for (int i = imageWidth; i < (imageWidth * imageHeight); i += imageWidth) {
			totalSeamValue += imageSeams[i]; 
			
			if (imageSeams[i] < minValue) {
				minValue = imageSeams[i];
				minValueLocation = i;
			}
			// TODO: break if min value is zero?
		}
	}

	int nextPixelR = 0; // next pixel to the right
	int nextPixelC = 0; // next pixel to the center
	int nextPixelL = 0; // next pixel to the left
	int currentMin = 0; // the minimum of nextPixelR, nextPixelC, and nextPixelL
	int countGoR = 0; // how many times the seam diverged upward
	int countGoL = 0; // how many times the seam diverged downward
	
	int nextPixelDistR = 0; // memory distance to the next pixel to the right
	int nextPixelDistC = 0; // memory distance to the next pixel to the center
	int nextPixelDistL = 0; // memory distance to the next pixel to the left

	// the deviation variables record a seam's net divergence from being straight
	//  if a seam's average y position is above its starting line, then the deviation is negative
	//  if a seam's average y position is below its starting line, then the deviation is positive
	//  if a seam's average y position is the same as its starting line, then the deviation is 0
	//  (the sign was arbitrarily choosen and does not matter latter, the ABS will be taken)
	//  ( ^^^ TODO: so, maybe we can simplify the deviation calculation ^^^ )
	int seamDeviation = 0; // current row's net deviation from being stright
	int seamDeviationABS = 0; // absolute value of the current row's deviation
	int lastSeamDeviation = 0; // last row's net deviation from being stright
	int lastSeamDeviationABS = 0; // absolute value of last row's deviation
	int negDeviationCount = 0;
	int posDeviationCount = 0;
	double tmp = 0.0;
	int *thisPath = (int*)xmalloc((unsigned long)imageWidth * sizeof(int));

	// a seam is considered to have zero weight when it is less than this value
	// we raise it based upon the size of the image to help ignore dust and speckles
	int imageSeamZeroValue = imageWidth / 50; // 2% of the image width (TODO: improve heuristic)
	// the minimum deviation to prevent a text line (seam gap) from ending
	int minLineContinue = imageSeamZeroValue * 1; //2; // (TODO: improve heuristic)

	int textLineDepth = 0; // how far we are into a recognized text line

	// loop conditions depend upon the direction
	if (direction == directionVertical) {
		loopBeg = (imageWidth * imageHeight) - 1 - imageWidth;
		loopEnd = (imageWidth * imageHeight) - 1;
		loopInc = 1;

		innerLoopBeg = imageHeight;
		innerLoopEnd = 1;
		innerLoopInc = -1;

		// also set the next pixel distances
		nextPixelDistR = imageWidth - 1;
		nextPixelDistC = imageWidth;
		nextPixelDistL = imageWidth + 1;
	} else {
		loopBeg = imageWidth - 1;
		loopEnd = (imageWidth * imageHeight) - 1;
		loopInc = imageWidth;

		innerLoopBeg = (imageWidth - 1);
		innerLoopEnd = 0;
		innerLoopInc = -1;

		// also set the next pixel distances
		nextPixelDistR = imageWidth + 1;
		nextPixelDistC = 1;
		nextPixelDistL = (imageWidth - 1) * -1;
	}

	// for every pixel in the right-most or bottom-most column of the image
	for (int k = loopBeg; k < loopEnd; k += loopInc) {
		// process seams with the lowest weights
		if (imageSeams[k] <= minValue) {
			// start from the left-most column
			minValueLocation = k;

			countGoR = 0;
			countGoL = 0;
			seamDeviation = 0;

			// move right-to-left across the image
			for (int j = innerLoopBeg; j >= innerLoopEnd; j += innerLoopInc) {
				// add pixel to the current path
				thisPath[j] = minValueLocation;

				// get the possible next pixles
				if (direction == directionVertical) {
					nextPixelR = imageSeams[minValueLocation - imageWidth + 1];
					nextPixelC = imageSeams[minValueLocation - imageWidth];
					nextPixelL = imageSeams[minValueLocation - imageWidth - 1];
				} else {
					nextPixelR = imageSeams[minValueLocation - 1 - imageWidth];
					nextPixelC = imageSeams[minValueLocation - 1];
					nextPixelL = imageSeams[minValueLocation - 1 + imageWidth];
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

				// increment the deviation counter according to this pixels deviation
				if (countGoR > countGoL) {
					seamDeviation -= 1;
				} else if (countGoR < countGoL) {
					seamDeviation += 1;
				}
			}
		
			// finf the absolute value of the deviation
			if (lastSeamDeviation < 0) {
				lastSeamDeviationABS = lastSeamDeviation * -1;
			} else {
				lastSeamDeviationABS = lastSeamDeviation;
			}

			// finf the absolute value of the deviation
			if (seamDeviation < 0) {
				seamDeviationABS = seamDeviation * -1;
			} else {
				seamDeviationABS = seamDeviation;
			}

			// 
			// This is the begining of a new text line
			// 
			if ((lastSeamDeviationABS <= imageSeamZeroValue) && (seamDeviationABS > imageSeamZeroValue)) {
				// 
				// Not presently in a text line
				// 
				if (textLineDepth < 1) {
					textLineDepth += 1;
					printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 128);

					negDeviationCount = 0;
					posDeviationCount = 0;
					// printf("%d\t%d\t%d\t%d\t%d\t BEGIN \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
				
				// 
				// Already in a text line, so just keep going
				// TODO: Consider making this the end of a text line
				// 
				} else {
					if (seamDeviation < 0) {
						negDeviationCount += 1;
					} else {
						posDeviationCount += 1;
					}
					// printf("%d\t%d\t%d\t%d\t%d\t SKP \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

					// TODO: Remove, DEBUG only
					printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 68);
				}

			// 
			// This is not the begining of a new text line
			// 
			} else {
				// 
				// Already in a text line
				// 
				if (textLineDepth >= 1) {
					// 
					// The current seam is a straight line
					// or the last seam's deviation was less than required to keep the text line going
					// 
					if ((seamDeviationABS <= 0) || (lastSeamDeviationABS < minLineContinue)) {
						// 
						// The text line has the required minimum number of image seam lines
						// 
						if (textLineDepth > imageSeamZeroValue) {
							textLineDepth = 0;
							printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 0);

							tmp = (double)negDeviationCount / ((double)negDeviationCount + (double)posDeviationCount);
							// printf("%d\t%d\t%d\t%d\t%d\t END (%d / %d = %f) \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth, negDeviationCount, posDeviationCount, tmp);
						
						// 
						// The text line does not have the required minimum number of image seam lines
						// 
						} else {
							// 
							// However, this seam and the last one was straight, so halt out of this text line
							// 
							if ((seamDeviationABS <= 0) && (lastSeamDeviationABS <= 0)) {
								textLineDepth = 0;
								printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 0);

								// printf("%d\t%d\t%d\t%d\t%d\t HALT \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

							// 
							// Assume we are at the begining of a jagged text line, keep going
							// 
							} else {
								if (seamDeviation < 0) {
									negDeviationCount += 1;
								} else {
									posDeviationCount += 1;
								}
								// printf("%d\t%d\t%d\t%d\t%d\t GAP \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

								// TODO: Remove, DEBUG only
								printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 72);
							}
						}

					// 
					// Already in a text line, so just keep going
					// 
					} else {
						textLineDepth += 1;
						if (seamDeviation < 0) {
							negDeviationCount += 1;
						} else {
							posDeviationCount += 1;
						}
						// printf("%d\t%d\t%d\t%d\t%d\t RUN \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

						// TODO: Remove, DEBUG only
						printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 64);
					}
				
				// 
				// Not presently in a text line, just ignore and move on
				// 
				} else {
					// printf("%d\t%d\t%d\t%d\t%d\t --- \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
				}
			}

			lastSeamDeviation = seamDeviation;
		}
	}
	return totalSeamValue;
}

static void setPixelPathVertical(int *imageSeams, int imageWidth, int currentPixel, int currentCol)
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
			aboveL = imageSeams[pixelAbove - 1];
			aboveC = imageSeams[pixelAbove];
			aboveR = imageSeams[pixelAbove + 1];
			newValue = min3(aboveL, aboveC, aboveR);
		} else {
			aboveL = imageSeams[pixelAbove - 1];
			aboveC = imageSeams[pixelAbove];
			aboveR = INT_MAX;
			newValue = min(aboveL, aboveC);
		}
	} else {
		aboveL = INT_MAX;
		aboveC = imageSeams[pixelAbove];
		aboveR = imageSeams[pixelAbove + 1];
		newValue = min(aboveC, aboveR);
	}
	imageSeams[currentPixel] += newValue;
}

static void fillSeamMatrixVertical(int *imageSeams, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	// do not process the first row, start with j=1
	for (int j = 1; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			setPixelPathVertical(imageSeams, imageWidth, currentPixel, i);
		}
	}
}

static int findSeamsVertical(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig)
{
	return findSeams(imageSeams, imageWidth, imageHeight, imageOrig, 0);
}

static void setPixelPathHorizontal(int *imageSeams, int imageWidth, int imageHeight, int currentPixel, int currentCol)
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
				leftT = imageSeams[pixelLeft - imageWidth];
				leftM = imageSeams[pixelLeft];
				leftB = imageSeams[pixelLeft + imageWidth];
				newValue = min3(leftT, leftM, leftB);
			} else {
				leftT = imageSeams[pixelLeft - imageWidth];
				leftM = imageSeams[pixelLeft];
				leftB = INT_MAX;
				newValue = min(leftT, leftM);
			}
		} else {
			leftT = INT_MAX;
			leftM = imageSeams[pixelLeft];
			leftB = imageSeams[pixelLeft + imageWidth];
			newValue = min(leftM, leftB);
		}
		imageSeams[currentPixel] += newValue;
	}
}

static void fillSeamMatrixHorizontal(int *imageSeams, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	// do not process the first row, start with j=1
	// must be in reverse order from verticle seam, calulate colums as we move across (top down, left to right)
	for (int i = 0; i < imageWidth; ++i) {
		for (int j = 1; j < imageHeight; ++j) {
			currentPixel = (j * imageWidth) + i;
			setPixelPathHorizontal(imageSeams, imageWidth, imageHeight, currentPixel, i);
		}
	}
}

static int findSeamsHorizontal(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig)
{
	return findSeams(imageSeams, imageWidth, imageHeight, imageOrig, 1);
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight)
{
	double imageScale = 0.25;
	int smallImageWidth = getScaledSize(imageWidth, imageScale);
	int smallImageHeight = getScaledSize(imageHeight, imageScale);
	int *smallImage = (int*)xmalloc((unsigned long)smallImageHeight * (unsigned long)smallImageWidth * sizeof(int));
	resize(imageVector, imageWidth, imageHeight, smallImage, smallImageWidth, smallImageHeight);
	
	int *newImage = (int*)xmalloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	int *newImage2 = (int*)xmalloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	int *newImageEnergy = (int*)xmalloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	
	int *newImageSeams = (int*)xmalloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	int *newImageSeams2 = (int*)xmalloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	
	int currentPixel = 0;

	// create an image of the original image's energies
	for (int j = 0; j < smallImageHeight; ++j) {
		for (int i = 0; i < smallImageWidth; ++i) {
			currentPixel = (j * smallImageWidth) + i;
			// mutable copy of the original image, to return the original image with seams shown
			newImage[currentPixel] = smallImage[currentPixel];
			newImage2[currentPixel] = smallImage[currentPixel];
			// original energies of the original image, to return the energies with seams shown
			newImageEnergy[currentPixel] = getPixelEnergySimple(smallImage, smallImageWidth, smallImageHeight, currentPixel, 1);
			// top down energy seam data of the original image
			newImageSeams[currentPixel] = newImageEnergy[currentPixel];
			newImageSeams2[currentPixel] = newImageEnergy[currentPixel];
		}
	}

	fillSeamMatrixVertical(newImageSeams, smallImageWidth, smallImageHeight);
	fillSeamMatrixHorizontal(newImageSeams2, smallImageWidth, smallImageHeight);

	int verticalSeamCost = findSeamsVertical(newImageSeams, smallImageWidth, smallImageHeight, newImage);
	int horizontalSeamCost = findSeamsHorizontal(newImageSeams2, smallImageWidth, smallImageHeight, newImage2);
	printf("Sum traversal cost of all seams: vertical = %d, horizontal = %d\n", verticalSeamCost, horizontalSeamCost);

	free(smallImage);
	free(newImageEnergy);
	free(newImageSeams);
	free(newImageSeams2);

	if (horizontalSeamCost < verticalSeamCost) {
		return newImage2;
	} else {
		return newImage;
	}

	//return newImage;
	//return newImage2;
	//return smallImage;
	//return newImageSeams;
	//return newImageSeams2;
	//return newImageEnergy;
}

#endif
