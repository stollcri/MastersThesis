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

struct minMax {
	int min;
	int max;
};

struct areaOfInterest {
	int imageWidth;
	int imageHeight;

	int domainMin;
	int domainMax;
	
	int rangeBeginEntry;
	int rangeBeginMin;
	int rangeBeginMax;

	int rangeEndEntry;
	int rangeEndMin;
	int rangeEndMax;

	int domainSize;
	int rangeSize;
};

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

static struct minMax *findSeamMinMax(int *seamPath, int imageWidth, int imageHeight, int direction)
{
	struct minMax *results = (struct minMax*)xmalloc(sizeof(struct minMax));
	results->min = INT_MAX;
	results->max = 0;

	// TODO: create macro definition
	int directionVertical = 0;
	int directionHorizontal = 1;
	if ((direction != directionVertical) && (direction != directionHorizontal)) {
		return results;
	}

	int currentPixel = 0;
	if (direction == directionVertical) {
		for (int j = imageHeight; j > 0; --j) {
			currentPixel = seamPath[j];

			if (currentPixel < results->min) {
				results->min = currentPixel;
			}

			if (currentPixel > results->max) {
				results->max = currentPixel;
			}
		}
	} else {
		for (int j = (imageWidth - 1); j >= 0; --j) {
			currentPixel = seamPath[j];

			if (currentPixel < results->min) {
				results->min = currentPixel;
			}

			if (currentPixel > results->max) {
				results->max = currentPixel;
			}
		}
	}

	return results;
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
	int imageSize = 0; // width when going horizontal, height when going vertical
	int loopBeg = 0; // where the outer loop begins
	int loopEnd = 0; // where the outer loop ends
	int loopInc = 0; // the increment of the outer loop
	int innerLoopBeg = 0; // where the inner loop begins
	int innerLoopEnd = 0; // where the inner loop ends
	int innerLoopInc = 0; // the increment of the inner loop

	// the return value of the function, the sum traversal cost of all seams
	int totalSeamValue = 0;
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;

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
		for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
			totalSeamValue += imageSeams[i]; 
			
			if (imageSeams[i] < minValue) {
				minValue = imageSeams[i];
				minValueLocation = i;
			}
			// TODO: break if min value is zero?
		}

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

		imageSize = imageHeight;
	} else {
		for (int i = imageWidth; i < (imageWidth * imageHeight); i += imageWidth) {
			totalSeamValue += imageSeams[i]; 
			
			if (imageSeams[i] < minValue) {
				minValue = imageSeams[i];
				minValueLocation = i;
			}
			// TODO: break if min value is zero?
		}

		loopBeg = imageWidth - 1;
		loopEnd = (imageWidth * imageHeight) - 0;//1;
		loopInc = imageWidth;

		innerLoopBeg = (imageWidth - 1);
		innerLoopEnd = 0;
		innerLoopInc = -1;

		// also set the next pixel distances
		nextPixelDistR = imageWidth + 1;
		nextPixelDistC = 1;
		nextPixelDistL = (imageWidth - 1) * -1;

		imageSize = imageWidth;
	}

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
	int *thisPath = (int*)xmalloc((unsigned long)imageWidth * sizeof(int));
	struct minMax *thisMinMax = (struct minMax*)xmalloc(sizeof(struct minMax));

	// a seam is considered to have zero weight when it is less than this value
	// we raise it based upon the size of the image to help ignore dust and speckles
	int imageSeamZeroValue = imageWidth / 50; // 2% of the image width (TODO: improve heuristic)
	// the minimum deviation to prevent a text line (seam gap) from ending
	int minLineContinue = imageSeamZeroValue * 1; //2; // (TODO: improve heuristic)

	int textLineDepth = 0; // how far we are into a recognized text line

	// These variables are to aid in tracking where information begain in the other dimensions
	// (if we are looking at horizontal seams, at which x value did the information start and end)
	// (NOTE: we are backtracking, so it begins on the right and ends on the left when horizontal)
	int deviationBeganAt = 0; // the right x value or bottom y value where information begins (pixel location)
	int deviationEndedAt = 0; // the left x value or top y value where the information ends (pixel location)
	int lastInnerSeamDeviation = 0; // the last seam deviation, used in the inner loop
	int tookCenterPixelFrom = 0; // when going center, keep track of the source pixel location
	// We have to find the outermost begining and ending points.
	// The above variables help find the per seam value, these values help find the per block values
	int infoAreaXYmin = 0; // the pixel location of the start of information block
	int infoAreaXYmax = 0; // the pixel location of the end of the information block

	struct areaOfInterest *interestingArea = (struct areaOfInterest*)xmalloc(sizeof(struct areaOfInterest));
	interestingArea->imageWidth = imageWidth;
	interestingArea->imageHeight = imageHeight;

	// for every pixel in the right-most or bottom-most column of the image
	for (int k = loopBeg; k < loopEnd; k += loopInc) {
		// process seams with the lowest weights
		if (imageSeams[k] <= minValue) {
			// start from the left-most column
			minValueLocation = k;

			countGoR = 0;
			countGoL = 0;
			seamDeviation = 0;

			deviationBeganAt = 0;
			deviationEndedAt = 0;

			// move right-to-left ot bottom-to-top across/up the image
			for (int j = innerLoopBeg; j >= innerLoopEnd; j += innerLoopInc) {
				// add pixel to the current path
				thisPath[j] = minValueLocation;
				tookCenterPixelFrom = 0;

				// get the possible next pixles
				if (direction == directionVertical) {
					nextPixelR = imageSeams[minValueLocation - imageWidth + 1];
					nextPixelC = imageSeams[minValueLocation - imageWidth];
					nextPixelL = imageSeams[minValueLocation - imageWidth - 1];
				} else {
					nextPixelR = imageSeams[minValueLocation - 1 - imageWidth];
					nextPixelC = imageSeams[minValueLocation - 1];
					if ((k + 1) != loopEnd) {
						nextPixelL = imageSeams[minValueLocation - 1 + imageWidth];
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
						tookCenterPixelFrom = minValueLocation;
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
						tookCenterPixelFrom = minValueLocation;
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
						tookCenterPixelFrom = minValueLocation;
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

				// this loop went straight (horizontal or vertical)
				if (tookCenterPixelFrom != 0) {
					// there is no deviation from straight yet
					if (seamDeviation == 0) {
						// we are not at the end of the seam
						if (j != innerLoopEnd) {
							deviationBeganAt = tookCenterPixelFrom;
						} else {
							deviationBeganAt = 0;
						}

					// there was already a deviation from straight
					} else {
						// we have a begining location already
						if (deviationBeganAt != 0) {
							// just keep the first of consecutive striaght runs
							if (seamDeviation != lastInnerSeamDeviation) {
								deviationEndedAt = tookCenterPixelFrom;
								lastInnerSeamDeviation = seamDeviation;
							}
						}
					}
				}
			}
		
			// find the absolute value of the deviation
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
					infoAreaXYmin = deviationEndedAt;
					infoAreaXYmax = deviationBeganAt;
					printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 128);
					printf("%d\t%d\t%d\t%d\t%d\t BEGIN \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

					interestingArea->rangeBeginEntry = thisPath[0];
					thisMinMax = findSeamMinMax(thisPath, imageWidth, imageHeight, direction);
					interestingArea->rangeBeginMin = thisMinMax->min;
					interestingArea->rangeBeginMax = thisMinMax->max;
				
				// 
				// Already in a text line, so just keep going
				// TODO: Consider making this the end of a text line
				// 
				} else {
					if ((infoAreaXYmin == 0) || ((deviationEndedAt % imageSize) < (infoAreaXYmin % imageSize))) {
						infoAreaXYmin = deviationEndedAt;
					}
					if ((infoAreaXYmax == 0) || ((deviationBeganAt % imageSize) > (infoAreaXYmax % imageSize))) {
						infoAreaXYmax = deviationBeganAt;
					}

					printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 68);
					printf("%d\t%d\t%d\t%d\t%d\t SKP \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
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
					if ((seamDeviationABS <= 0) || (lastSeamDeviationABS < minLineContinue) || (k >= (loopEnd - 1))) {
						// 
						// The text line has the required minimum number of image seam lines
						// 
						if (textLineDepth > imageSeamZeroValue) {
							textLineDepth = 0;
							
							if (infoAreaXYmin != 0) {
								imageOrig[infoAreaXYmin] = 0;
								imageOrig[infoAreaXYmin+imageWidth] = 0;
								imageOrig[infoAreaXYmin+imageWidth-1] = 0;
								imageOrig[infoAreaXYmin+imageWidth+1] = 0;
								imageOrig[infoAreaXYmin+imageWidth+imageWidth] = 0;
							}
							if (infoAreaXYmax != 0) {
								imageOrig[infoAreaXYmax] = 0;
								imageOrig[infoAreaXYmax+imageWidth] = 0;
								imageOrig[infoAreaXYmax+imageWidth-1] = 0;
								imageOrig[infoAreaXYmax+imageWidth+1] = 0;
								imageOrig[infoAreaXYmax+imageWidth+imageWidth] = 0;
							}

							printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 0);
							printf("%d\t%d\t%d\t%d\t%d\t END \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

							interestingArea->domainMin = infoAreaXYmin;
							interestingArea->domainMax = infoAreaXYmax;
							interestingArea->domainSize = ((interestingArea->domainMax % imageSize) + 1) - ((interestingArea->domainMin % imageSize) + 1);
							interestingArea->rangeEndEntry = thisPath[0];
							thisMinMax = findSeamMinMax(thisPath, imageWidth, imageHeight, direction);
							interestingArea->rangeEndMin = thisMinMax->min;
							interestingArea->rangeEndMax = thisMinMax->max;
							interestingArea->rangeSize = ((interestingArea->rangeEndMax / imageSize) + 1) - ((interestingArea->rangeBeginMin / imageSize) + 1);
							printf("Range begin entry: %d (%d)\n", interestingArea->rangeBeginEntry, ((interestingArea->rangeBeginEntry / imageSize) + 1));
							printf("Range begin min: %d (%d)\n", interestingArea->rangeBeginMin, ((interestingArea->rangeBeginMin / imageSize) + 1));
							printf("Range begin max: %d (%d)\n", interestingArea->rangeBeginMax, ((interestingArea->rangeBeginMax / imageSize) + 1));
							printf("Range end entry: %d (%d)\n", interestingArea->rangeEndEntry, ((interestingArea->rangeEndEntry / imageSize) + 1));
							printf("Range end min: %d (%d)\n", interestingArea->rangeEndMin, ((interestingArea->rangeEndMin / imageSize) + 1));
							printf("Range end max: %d (%d)\n", interestingArea->rangeEndMax, ((interestingArea->rangeEndMax / imageSize) + 1));
							printf("Domain min: %d (%d)\n", interestingArea->domainMin, ((interestingArea->domainMin % imageSize) + 1));
							printf("Domain max: %d (%d)\n", interestingArea->domainMax, ((interestingArea->domainMax % imageSize) + 1));
							printf("Domain size: %d\n", interestingArea->domainSize);
							printf("Range size: %d\n", interestingArea->rangeSize);
							printf(" ----- \n");

							infoAreaXYmin = 0;
							infoAreaXYmax = 0;
						
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
								printf("%d\t%d\t%d\t%d\t%d\t HALT \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);

							// 
							// Assume we are at the begining of a jagged text line, keep going
							// 
							} else {
								if ((infoAreaXYmin == 0) || ((deviationEndedAt % imageSize) < (infoAreaXYmin % imageSize))) {
									infoAreaXYmin = deviationEndedAt;
								}
								if ((infoAreaXYmax == 0) || ((deviationBeganAt % imageSize) > (infoAreaXYmax % imageSize))) {
									infoAreaXYmax = deviationBeganAt;
								}

								printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 72);
								printf("%d\t%d\t%d\t%d\t%d\t GAP \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
							}
						}

					// 
					// Already in a text line, so just keep going
					// 
					} else {
						textLineDepth += 1;
						
						if ((infoAreaXYmin == 0) || ((deviationEndedAt % imageSize) < (infoAreaXYmin % imageSize))) {
							infoAreaXYmin = deviationEndedAt;
						}
						if ((infoAreaXYmax == 0) || ((deviationBeganAt % imageSize) > (infoAreaXYmax % imageSize))) {
							infoAreaXYmax = deviationBeganAt;
						}

						printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 64);
						printf("%d\t%d\t%d\t%d\t%d\t RUN \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
					}
				
				// 
				// Not presently in a text line, just ignore and move on
				// 
				} else {
					printf("%d\t%d\t%d\t%d\t%d\t --- \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
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

	//fillSeamMatrixVertical(newImageSeams, smallImageWidth, smallImageHeight);
	fillSeamMatrixHorizontal(newImageSeams2, smallImageWidth, smallImageHeight);

	//int verticalSeamCost = findSeamsVertical(newImageSeams, smallImageWidth, smallImageHeight, newImage);
	int horizontalSeamCost = findSeamsHorizontal(newImageSeams2, smallImageWidth, smallImageHeight, newImage2);
	//printf("Sum traversal cost of all seams: vertical = %d, horizontal = %d\n", verticalSeamCost, horizontalSeamCost);

	free(smallImage);
	free(newImageEnergy);
	free(newImageSeams);
	free(newImageSeams2);

	//if (horizontalSeamCost < verticalSeamCost) {
		return newImage2;
	//} else {
	//	return newImage;
	//}

	//return newImage;
	//return newImage2;
	//return smallImage;
	//return newImageSeams;
	//return newImageSeams2;
	//return newImageEnergy;
}

#endif
