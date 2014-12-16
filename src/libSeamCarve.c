/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBSEAMCARVE_C
#define LIBSEAMCARVE_C

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "libWrappers.c"
#include "libResize.c"
#include "libMinMax.c"
#include "itoa.c"

struct minMax {
	int min;
	int max;
};

struct areaOfInterest {
	int imageSize;;
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

	int *pathBegin;
	int *pathEnd;
};

static void addPixelToPartialImage(int *image, int pixelValue, int pixelCol, int pixelRow, int pixelOffset, int fullImageWidth)
{
	int partialImagePixel = ((pixelRow * fullImageWidth) + pixelCol) - pixelOffset;
	image[partialImagePixel] = pixelValue;
}

static void savePartialImageVertical(int *image, struct areaOfInterest *interestingArea)
{
	int fullImageSize = interestingArea->imageSize;
	int fullImageWidth = interestingArea->imageWidth;
	int fullImageHeight = interestingArea->imageHeight;

	int fullImageStartRow = interestingArea->domainMin / fullImageWidth;
	int fullImageEndRow = interestingArea->domainMax / fullImageWidth;

	int fullImageEntryCol = (interestingArea->rangeBeginEntry % fullImageWidth);
	int fullImageStartCol = (interestingArea->rangeBeginMin % fullImageWidth);
	int fullImageBeginCol = (interestingArea->rangeBeginMax % fullImageWidth);
	int fullImageExitCol = (interestingArea->rangeEndEntry % fullImageWidth);
	int fullImageFinCol = (interestingArea->rangeEndMin % fullImageWidth);
	int fullImageEndCol = (interestingArea->rangeEndMax % fullImageWidth);

	int newImageWidth = fullImageEndCol - fullImageStartCol;
	int newImageHeight = fullImageEndRow - fullImageStartRow;

	int *partialImage = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));

	int newImageRowOffsetFactor = fullImageEntryCol - fullImageStartCol;
	int newImageRowOffset = newImageRowOffsetFactor * newImageWidth;

	int currentPixel = 0;
	int currentCol = 0;
	int currentRow = 0;
	int newImagePixel = 0;
	int *seamPath = 0;

	// process the begining seam, the top of the image
	seamPath = interestingArea->pathBegin;
	for (int i = (fullImageSize - 1); i >= 0; --i) {
		currentPixel = seamPath[i];
		currentRow = currentPixel / fullImageWidth;
		
		// only deal with pixels within the domain
		if ((currentRow > fullImageStartRow) && (currentRow < fullImageEndRow)) {
			currentCol = currentPixel % fullImageWidth;
			image[currentPixel] -= 192;
			image[currentPixel] = max(image[currentPixel], 0);

			// back fill pixels below current pixel
			if (currentCol < fullImageEntryCol) {
				// for (int pix = currentRow; pix < fullImageEntryRow; ++pix) {
				for (int pix = currentCol; pix < fullImageEntryCol; ++pix) {
					currentPixel += 1;
					image[currentPixel] -= 192;
					image[currentPixel] = max(image[currentPixel], 0);
				}
			}
		}
	}

	int bodyPixelsStart = 0;
	for (int i = fullImageEntryCol; i <= (fullImageFinCol - 1); ++i) {
		bodyPixelsStart = ((fullImageStartRow + 1) * fullImageWidth) + i + 1;

		for (int j = 0; j < (newImageHeight - 1); ++j) {
			image[bodyPixelsStart] -= 192;
			image[bodyPixelsStart] = max(image[bodyPixelsStart], 0);
			bodyPixelsStart += fullImageWidth;
		}
	}

	// process the ending seam, the bottom of the image
	seamPath = interestingArea->pathEnd;
	for (int i = (fullImageSize - 1); i >= 0; --i) {
		currentPixel = seamPath[i];
		currentRow = currentPixel / fullImageWidth;

		if ((currentRow > fullImageStartRow) && (currentRow < fullImageEndRow)) {
			//printf(" %d\t current: pix = %d, col = %d, row %d\n", i, currentPixel, currentCol, currentRow);
			image[currentPixel] -= 192;
			image[currentPixel] = max(image[currentPixel], 0);

			currentCol = currentPixel % fullImageWidth;
			if (currentCol > fullImageExitCol) {
				for (int pix = currentCol; pix > fullImageExitCol; --pix) {
					currentPixel -= 1;
					image[currentPixel] -= 192;
					image[currentPixel] = max(image[currentPixel], 0);
				}
			}
		}
	}

}

/*
 * TODO: this was designed only with horizontal seams in mind
 * 			once it works for horizontal seams it needs to work for vertical as well
 */
static void savePartialImageHorizontal(int *image, struct areaOfInterest *interestingArea)
{
	int fullImageSize = interestingArea->imageSize;
	int fullImageWidth = interestingArea->imageWidth;
	int fullImageHeight = interestingArea->imageHeight;

	int fullImageStartCol = interestingArea->domainMin % fullImageWidth;
	int fullImageEndCol = interestingArea->domainMax % fullImageWidth;

	int fullImageEntryRow = (interestingArea->rangeBeginEntry / fullImageWidth);
	int fullImageStartRow = (interestingArea->rangeBeginMin / fullImageWidth);
	int fullImageBeginRow = (interestingArea->rangeBeginMax / fullImageWidth);
	int fullImageExitRow = (interestingArea->rangeEndEntry / fullImageWidth);
	int fullImageFinRow = (interestingArea->rangeEndMin / fullImageWidth);
	// TODO: add one below?
	int fullImageEndRow = (interestingArea->rangeEndMax / fullImageWidth);

	int newImageWidth = fullImageEndCol - fullImageStartCol;
	int newImageHeight = fullImageEndRow - fullImageStartRow;

	int *partialImage = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	int partialImageOffset = ((fullImageWidth * fullImageHeight) - (newImageWidth * newImageHeight)) / 2;

	// int newImageRowOffsetFactor = fullImageEntryRow - fullImageStartRow;
	// int newImageRowOffset = newImageRowOffsetFactor * newImageWidth;

	int currentPixel = 0;
	int currentCol = 0;
	int currentRow = 0;
	int newImagePixel = 0;
	int *seamPath = 0;

	// process the begining seam, the top of the image
	seamPath = interestingArea->pathBegin;
	for (int i = (fullImageSize - 1); i >= 0; --i) {
		currentPixel = seamPath[i];
		currentCol = currentPixel % fullImageWidth;
		
		// only deal with pixels within the domain
		if ((currentCol > fullImageStartCol) && (currentCol < fullImageEndCol)) {
			currentRow = currentPixel / fullImageWidth;
//			addPixelToPartialImage(partialImage, image[currentPixel], currentCol, currentRow, partialImageOffset, fullImageWidth);
			image[currentPixel] -= 192;
			image[currentPixel] = max(image[currentPixel], 0);

			// back fill pixels below current pixel
			if (currentRow < fullImageEntryRow) {
				for (int pix = currentRow; pix < fullImageEntryRow; ++pix) {
					currentPixel += fullImageWidth;
//					addPixelToPartialImage(partialImage, image[currentPixel], currentCol, currentRow, partialImageOffset, fullImageWidth);
					image[currentPixel] -= 192;
					image[currentPixel] = max(image[currentPixel], 0);
				}
			}
		}
	}

	int bodyPixelsStart = 0;
	for (int i = fullImageBeginRow; i <= (fullImageFinRow - 1); ++i) {
		bodyPixelsStart = (i * fullImageWidth) + fullImageStartCol + 1;

		for (int j = 0; j < (newImageWidth - 1); ++j) {
//			addPixelToPartialImage(partialImage, image[currentPixel], currentCol, currentRow, partialImageOffset, fullImageWidth);
			image[bodyPixelsStart] -= 192;
			image[bodyPixelsStart] = max(image[bodyPixelsStart], 0);
			++bodyPixelsStart;
		}
	}

	// process the ending seam, the bottom of the image
	seamPath = interestingArea->pathEnd;
	for (int i = fullImageSize; i > 0; --i) {
		currentPixel = seamPath[i];
		currentCol = currentPixel % fullImageWidth;

		if ((currentCol > fullImageStartCol) && (currentCol < fullImageEndCol)) {
			currentRow = currentPixel / fullImageWidth;
//			addPixelToPartialImage(partialImage, image[currentPixel], currentCol, currentRow, partialImageOffset, fullImageWidth);
			image[currentPixel] -= 192;
			image[currentPixel] = max(image[currentPixel], 0);

			if (currentRow > fullImageExitRow) {
				for (int pix = currentRow; pix > fullImageExitRow; --pix) {
					currentPixel -= fullImageWidth;
//					addPixelToPartialImage(partialImage, image[currentPixel], currentCol, currentRow, partialImageOffset, fullImageWidth);
					image[currentPixel] -= 192;
					image[currentPixel] = max(image[currentPixel], 0);
				}
			}
		}
	}
	/*
	char resultFile[64];
	char resultFileA[16];
	char resultFileB[16];
	char *fileNameMin = (char*)xmalloc((unsigned long)newImageWidth * sizeof(char));
	char *fileNameMax = (char*)xmalloc((unsigned long)newImageWidth * sizeof(char));
	fileNameMin = itoa(interestingArea->domainMin);
	strcpy(resultFileA, fileNameMin);
	fileNameMax = itoa(interestingArea->domainMax);
	strcpy(resultFileB, fileNameMax);
	// if (fileNameMin) free(fileNameMin);
	// if (fileNameMax) free(fileNameMax);
	// printf("%d, %d => %s, %s\n", interestingArea->domainMin, interestingArea->domainMax, resultFileA, resultFileB);

	strcpy(resultFile, "./tst/out_part_");
	strcat(resultFile, resultFileA);
	strcat(resultFile, "-");
	strcat(resultFile, resultFileB);
	strcat(resultFile, ".png");
	
	printf("%s\n", resultFile);
	write_png_file(partialImage, newImageWidth, newImageHeight, resultFile);
	*/
}

static void savePartialImage(int *image, struct areaOfInterest *interestingArea, int direction)
{
	// TODO: create macro definition
	int directionVertical = 0;
	int directionHorizontal = 1;
	if ((direction != directionVertical) && (direction != directionHorizontal)) {
		return;
	}

	if (direction == directionVertical) {
		savePartialImageVertical(image, interestingArea);
	} else {
		savePartialImageHorizontal(image, interestingArea);
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
	pixelLeft = currentPixel - gradientSize;
	if (pixelLeft < 0) {
		pixelLeft = 0;
	}

	int pixelCol = currentPixel % imageWidth;
	int xDif = 0;
	if (pixelCol > 0) {
		if (imageVector[pixelLeft] > imageVector[currentPixel]) {
			xDif = imageVector[pixelLeft] - imageVector[currentPixel];
		} else {
			xDif = imageVector[currentPixel] - imageVector[pixelLeft];
		}
	}

	return min((yDif + xDif), 255);
}

static int getPixelEnergySobel(int *imageVector, int imageWidth, int imageHeight, int currentPixel)
{
	int pixelDepth = 1;
    int imageByteWidth = imageWidth * pixelDepth;
    int currentCol = currentPixel % imageByteWidth;
    int p1, p2, p3, p4, p5, p6, p7, p8, p9;

    // get pixel locations within the image array
    // image border pixels have undefined (zero) energy
    if ((currentPixel > imageByteWidth) &&
        (currentPixel < (imageByteWidth * (imageHeight - 1))) &&
        (currentCol > 0) &&
        (currentCol < (imageByteWidth - 4))) {
        p1 = currentPixel - imageByteWidth - pixelDepth;
        p2 = currentPixel - imageByteWidth;
        p3 = currentPixel - imageByteWidth + pixelDepth;
        
        p4 = currentPixel - pixelDepth;
        p5 = currentPixel;
        p6 = currentPixel + pixelDepth;
        
        p7 = currentPixel + imageByteWidth - pixelDepth;
        p8 = currentPixel + imageByteWidth;
        p9 = currentPixel + imageByteWidth + pixelDepth;
    } else {
        // TODO: consider attempting to evaluate border pixels
        return 33; // zero and INT_MAX are significant, so return 1
    }
    
    // get the pixel values from the image array
    int p1val = imageVector[p1];
    int p2val = imageVector[p2];
    int p3val = imageVector[p3];
    
    int p4val = imageVector[p4];
    int p5val = imageVector[p5];
    int p6val = imageVector[p6];

    int p7val = imageVector[p7];
    int p8val = imageVector[p8];
    int p9val = imageVector[p9];
    
    // apply the sobel filter
    int sobelX = (p3val + (p6val + p6val) + p9val - p1val - (p4val + p4val) - p7val);
    int sobelY = (p1val + (p2val + p2val) + p3val - p7val - (p8val + p8val) - p9val);

    // bounded gradient magnitude
    return min(max((int)(sqrt((sobelX * sobelX) + (sobelY * sobelY))/2), 0), 255);

    // alt method - laplacian
    // double sobelX = p5val + p5val + p5val + p5val - p2val - p4val - p6val - p8val;
    // return min((255-sobelX), 255);

    // alt method - gradient magnitude
    // double sobelX = p6val - p4val;
    // double sobelY = p8val - p2val;
    // return min((sobelX + sobelY), 255);
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
	int currentMinCol = INT_MAX;
	int currentMaxCol = 0;
	if (direction == directionVertical) {
		int currentCol = 0;
		for (int j = (imageHeight - 1); j >= 0; --j) {
			currentPixel = seamPath[j];
			currentCol = currentPixel % imageWidth;

			if (currentCol < currentMinCol) {
				currentMinCol = currentCol;
				results->min = currentPixel;
			}

			if (currentCol > currentMaxCol) {
				currentMaxCol = currentCol;
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
		for (int j = imageHeight; j >= 0; --j) {
			currentPixel = seamPath[j];
			if (currentPixel >= 0) {
				image[currentPixel] = grayScale;
			}
		}
	} else {
		for (int j = (imageWidth - 1); j >= 0; --j) {
			currentPixel = seamPath[j];
			if (currentPixel >= 0) {
				image[currentPixel] = grayScale;
			}
		}
	}
}

//
// TODO: Refactor, this function is a beast
// 
static int findSeams(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig, int direction, int *imageSeamCounts)
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
		// for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
		for (int i = ((imageWidth * imageHeight) - 1 - imageWidth); i < ((imageWidth * imageHeight) - 1); i += 1) {
			totalSeamValue += imageSeams[i]; 
			
			if (imageSeams[i] < minValue) {
				minValue = imageSeams[i];
				minValueLocation = i;
			}
			// TODO: break if min value is zero?
		}
		printf("v: %i \n", totalSeamValue);

		loopBeg = (imageWidth * imageHeight) - 1 - imageWidth;
		loopEnd = (imageWidth * imageHeight) - 1;
		loopInc = 1;

		// also set the next pixel distances
		nextPixelDistR = imageWidth - 1;
		nextPixelDistC = imageWidth;
		nextPixelDistL = imageWidth + 1;

		imageSize = imageHeight;
	} else {
		// for (int i = imageWidth; i < (imageWidth * imageHeight); i += imageWidth) {
		for (int i = (imageWidth - 1); i < (imageWidth * imageHeight); i += imageWidth) {
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
	int *thisPath = (int*)xmalloc((unsigned long)imageSize * sizeof(int));
	thisPath[0] = -1;
	thisPath[imageSize] = -1;
	struct minMax *thisMinMax = (struct minMax*)xmalloc(sizeof(struct minMax));

	// a seam is considered to have zero weight when it is less than this value
	// we raise it based upon the size of the image to help ignore dust and speckles
	int imageSeamZeroValue = imageSize / 25; // 4% of the image width (TODO: improve heuristic)
	int imageSeamMinSize = imageSeamZeroValue / 2;
	// printf(" imageSize: %d (/50) = imageSeamZeroValue = %d\n", imageSize, imageSeamZeroValue);
	// the minimum deviation to prevent a text line (seam gap) from ending
	int minLineContinue = imageSeamZeroValue * 1; //2; // (TODO: improve heuristic)
	// printf(" imageSeamZeroValue: %d (*1) = minLineContinue = %d\n", imageSeamZeroValue, minLineContinue);

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
	interestingArea->imageSize = imageSize;
	interestingArea->imageWidth = imageWidth;
	interestingArea->imageHeight = imageHeight;
	interestingArea->pathBegin = (int*)xmalloc((unsigned long)imageSize * sizeof(int));
	interestingArea->pathEnd = (int*)xmalloc((unsigned long)imageSize * sizeof(int));

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
			for (int j = (imageSize - 1); j >= 0; --j) {
				// add pixel to the current path
				thisPath[j] = minValueLocation;
				tookCenterPixelFrom = 0;

				if (imageSeamCounts[minValueLocation] < (255-32)) {
					imageSeamCounts[minValueLocation] += 32;
				}

				// get the possible next pixles
				if (direction == directionVertical) {
					if (((minValueLocation - imageWidth + 1) % imageWidth) != 0) {
						nextPixelR = imageSeams[minValueLocation - imageWidth + 1];
					} else {
						nextPixelR = INT_MAX;
					}
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
						if (j != 0) {
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
					// printf("%d\t%d\t%d\t%d\t%d\t BEGIN \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
					// printf("BEGIN infoAreaXYmin: %d (%d), infoAreaXYmax: %d (%d)\n", ((infoAreaXYmin % imageSize) + 1), infoAreaXYmin, ((infoAreaXYmax % imageSize) + 1), infoAreaXYmax);

					//interestingArea->pathBegin = thisPath;
					// cannot do this ^^^^ it's a reference, not values
					int *pixArray = interestingArea->pathBegin;
					for (int pix = 0; pix < imageSize; ++pix) {
						pixArray[pix] = thisPath[pix];
					}
					interestingArea->rangeBeginEntry = thisPath[0];
					thisMinMax = findSeamMinMax(thisPath, imageWidth, imageHeight, direction);
					interestingArea->rangeBeginMin = thisMinMax->min;
					interestingArea->rangeBeginMax = thisMinMax->max;
				
				// 
				// Already in a text line, so just keep going
				// TODO: Consider making this the end of a text line
				// 
				} else {
					if (direction == directionVertical) {
						if ((infoAreaXYmin == 0) || ((deviationEndedAt / imageSize) < (infoAreaXYmin / imageSize))) {
							infoAreaXYmin = deviationEndedAt;
						}
						if ((infoAreaXYmax == 0) || ((deviationBeganAt / imageSize) > (infoAreaXYmax / imageSize))) {
							infoAreaXYmax = deviationBeganAt;
						}
					} else {
						if ((infoAreaXYmin == 0) || ((deviationEndedAt % imageSize) < (infoAreaXYmin % imageSize))) {
							infoAreaXYmin = deviationEndedAt;
						}
						if ((infoAreaXYmax == 0) || ((deviationBeganAt % imageSize) > (infoAreaXYmax % imageSize))) {
							infoAreaXYmax = deviationBeganAt;
						}
					}

					printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 68);
					// printf("%d\t%d\t%d\t%d\t%d\t SKP \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
					// printf("  SKP infoAreaXYmin: %d (%d), infoAreaXYmax: %d (%d)\n", ((infoAreaXYmin % imageSize) + 1), infoAreaXYmin, ((infoAreaXYmax % imageSize) + 1), infoAreaXYmax);
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
						if (textLineDepth > imageSeamMinSize) {
							textLineDepth = 0;
							
							// if (infoAreaXYmin != 0) {
							// 	imageOrig[infoAreaXYmin] = 0;
							// 	imageOrig[infoAreaXYmin+imageWidth] = 0;
							// 	imageOrig[infoAreaXYmin+imageWidth-1] = 0;
							// 	imageOrig[infoAreaXYmin+imageWidth+1] = 0;
							// 	imageOrig[infoAreaXYmin+imageWidth+imageWidth] = 0;
							// }
							// if (infoAreaXYmax != 0) {
							// 	imageOrig[infoAreaXYmax] = 0;
							// 	imageOrig[infoAreaXYmax+imageWidth] = 0;
							// 	imageOrig[infoAreaXYmax+imageWidth-1] = 0;
							// 	imageOrig[infoAreaXYmax+imageWidth+1] = 0;
							// 	imageOrig[infoAreaXYmax+imageWidth+imageWidth] = 0;
							// }

							printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 0);
							// printf("%d\t%d\t%d\t%d\t%d\t END \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
							// printf("  END infoAreaXYmin: %d (%d), infoAreaXYmax: %d (%d)\n", ((infoAreaXYmin % imageSize) + 1), infoAreaXYmin, ((infoAreaXYmax % imageSize) + 1), infoAreaXYmax);

							interestingArea->pathEnd = thisPath;
							interestingArea->domainMin = infoAreaXYmin;
							interestingArea->domainMax = infoAreaXYmax;
							interestingArea->domainSize = ((interestingArea->domainMax % imageSize) + 1) - ((interestingArea->domainMin % imageSize) + 1);
							interestingArea->rangeEndEntry = thisPath[0];
							thisMinMax = findSeamMinMax(thisPath, imageWidth, imageHeight, direction);
							interestingArea->rangeEndMin = thisMinMax->min;
							interestingArea->rangeEndMax = thisMinMax->max;
							interestingArea->rangeSize = ((interestingArea->rangeEndMax / imageSize) + 1) - ((interestingArea->rangeBeginMin / imageSize) + 1);

							savePartialImage(imageOrig, interestingArea, direction);

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
								// printf("%d\t%d\t%d\t%d\t%d\t HALT \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
								// printf(" HALT infoAreaXYmin: %d (%d), infoAreaXYmax: %d (%d)\n", ((infoAreaXYmin % imageSize) + 1), infoAreaXYmin, ((infoAreaXYmax % imageSize) + 1), infoAreaXYmax);

								interestingArea->pathEnd = thisPath;
								interestingArea->domainMin = infoAreaXYmin;
								interestingArea->domainMax = infoAreaXYmax;
								interestingArea->domainSize = ((interestingArea->domainMax % imageSize) + 1) - ((interestingArea->domainMin % imageSize) + 1);
								interestingArea->rangeEndEntry = thisPath[0];
								thisMinMax = findSeamMinMax(thisPath, imageWidth, imageHeight, direction);
								interestingArea->rangeEndMin = thisMinMax->min;
								interestingArea->rangeEndMax = thisMinMax->max;
								interestingArea->rangeSize = ((interestingArea->rangeEndMax / imageSize) + 1) - ((interestingArea->rangeBeginMin / imageSize) + 1);

								savePartialImage(imageOrig, interestingArea, direction);

								infoAreaXYmin = 0;
								infoAreaXYmax = 0;

							// 
							// Assume we are at the begining of a jagged text line, keep going
							// 
							} else {
								if (direction == directionVertical) {
									if ((infoAreaXYmin == 0) || ((deviationEndedAt / imageSize) < (infoAreaXYmin / imageSize))) {
										infoAreaXYmin = deviationEndedAt;
									}
									if ((infoAreaXYmax == 0) || ((deviationBeganAt / imageSize) > (infoAreaXYmax / imageSize))) {
										infoAreaXYmax = deviationBeganAt;
									}
								} else {
									if ((infoAreaXYmin == 0) || ((deviationEndedAt % imageSize) < (infoAreaXYmin % imageSize))) {
										infoAreaXYmin = deviationEndedAt;
									}
									if ((infoAreaXYmax == 0) || ((deviationBeganAt % imageSize) > (infoAreaXYmax % imageSize))) {
										infoAreaXYmax = deviationBeganAt;
									}
								}

								printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 72);
								// printf("%d\t%d\t%d\t%d\t%d\t GAP \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
								// printf("  GAP infoAreaXYmin: %d (%d), infoAreaXYmax: %d (%d)\n", ((infoAreaXYmin % imageSize) + 1), infoAreaXYmin, ((infoAreaXYmax % imageSize) + 1), infoAreaXYmax);
							}
						}

					// 
					// Already in a text line, so just keep going
					// 
					} else {
						textLineDepth += 1;
						
						if (direction == directionVertical) {
							if ((infoAreaXYmin == 0) || ((deviationEndedAt / imageSize) < (infoAreaXYmin / imageSize))) {
								infoAreaXYmin = deviationEndedAt;
							}
							if ((infoAreaXYmax == 0) || ((deviationBeganAt / imageSize) > (infoAreaXYmax / imageSize))) {
								infoAreaXYmax = deviationBeganAt;
							}
						} else {
							if ((infoAreaXYmin == 0) || ((deviationEndedAt % imageSize) < (infoAreaXYmin % imageSize))) {
								infoAreaXYmin = deviationEndedAt;
							}
							if ((infoAreaXYmax == 0) || ((deviationBeganAt % imageSize) > (infoAreaXYmax % imageSize))) {
								infoAreaXYmax = deviationBeganAt;
							}
						}

						printSeam(thisPath, imageOrig, imageWidth, imageHeight, direction, 64);
						// printf("%d\t%d\t%d\t%d\t%d\t RUN \n", lastSeamDeviation, lastSeamDeviationABS, seamDeviation, seamDeviationABS, textLineDepth);
						// printf("  RUN infoAreaXYmin: %d (%d), infoAreaXYmax: %d (%d)\n", ((infoAreaXYmin % imageSize) + 1), infoAreaXYmin, ((infoAreaXYmax % imageSize) + 1), infoAreaXYmax);
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
	// printf("-----\t-----\t-----\t-----\t-----\t-------\n");
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

static int fillSeamMatrixVertical(int *imageSeams, int imageWidth, int imageHeight)
{
	int result = 0;
	int currentPixel = 0;
	// do not process the first row, start with j=1
	for (int j = 1; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			setPixelPathVertical(imageSeams, imageWidth, currentPixel, i);

			if (imageSeams[currentPixel] != 0) {
				++result;
			}
		}
	}
	return result;
}

static int findSeamsVertical(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig, int *imageSeamCounts)
{
	return findSeams(imageSeams, imageWidth, imageHeight, imageOrig, 0, imageSeamCounts);
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

static int fillSeamMatrixHorizontal(int *imageSeams, int imageWidth, int imageHeight)
{
	int result = 0;
	int currentPixel = 0;
	// do not process the first row, start with j=1
	// must be in reverse order from verticle seam, calulate colums as we move across (top down, left to right)
	for (int i = 0; i < imageWidth; ++i) {
		for (int j = 1; j < imageHeight; ++j) {
			currentPixel = (j * imageWidth) + i;
			setPixelPathHorizontal(imageSeams, imageWidth, imageHeight, currentPixel, i);

			if (imageSeams[currentPixel] != 0) {
				++result;
			}
		}
	}
	return result;
}

static int findSeamsHorizontal(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig, int *imageSeamCounts)
{
	return findSeams(imageSeams, imageWidth, imageHeight, imageOrig, 1, imageSeamCounts);
}

static int *seamCarve(int *imageVector, int imageWidth, int forceDirection, int imageHeight)
{
	double imageScale = 1;//0.125;
	int imagePadding = 4;
	int newImageWidth = getScaledSize(imageWidth, imageScale) + imagePadding + imagePadding;
	int newImageHeight = getScaledSize(imageHeight, imageScale) + imagePadding + imagePadding;
	int smallImageWidth = getScaledSize(imageWidth, imageScale);
	int smallImageHeight = getScaledSize(imageHeight, imageScale);
	int *smallImage = (int*)xmalloc((unsigned long)smallImageHeight * (unsigned long)smallImageWidth * sizeof(int));
	resize(imageVector, imageWidth, imageHeight, smallImage, smallImageWidth, smallImageHeight);
	
	int *newImage = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	int *newImage2 = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	int *newImageEnergy = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	
	int *newImageSeams = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	int *newImageSeams2 = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));

	int *imageSeamCounts = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	int *imageSeamCounts2 = (int*)xmalloc((unsigned long)newImageWidth * (unsigned long)newImageHeight * sizeof(int));
	
	int currentPixel = 0;
	int uncroppedPixel = 0;

	int jj = 0;
	int ii = 0;
	
	for (int j = 0; j < newImageHeight; ++j) {
		for (int i = 0; i < newImageWidth; ++i) {
			currentPixel = (j * newImageWidth) + i;
			if ((j >= imagePadding) && (j < (imagePadding + smallImageHeight)) && (i >= imagePadding) && (i < (imagePadding + smallImageWidth))) {
				uncroppedPixel = (jj * newImageWidth) + ii;
				++ii;
				if (ii >= newImageWidth) {
					ii = 0;
					++jj;
				} 

				
				// mutable copy of the original image, to return the original image with seams shown
				newImage[currentPixel] = smallImage[uncroppedPixel];
				newImage2[currentPixel] = smallImage[uncroppedPixel];
			} else {
				newImage[currentPixel] = 255;
				newImage2[currentPixel] = 255;
			}
			imageSeamCounts[currentPixel] = 0;
		}
	}

	// create an image of the original image's energies
	for (int j = 0; j < newImageHeight; ++j) {
		for (int i = 0; i < newImageWidth; ++i) {
			currentPixel = (j * newImageWidth) + i;
			if ((j >= imagePadding) && (j < (imagePadding + smallImageHeight)) && (i >= imagePadding) && (i < (imagePadding + smallImageWidth))) {
				// original energies of the original image, to return the energies with seams shown
				//newImageEnergy[currentPixel] = getPixelEnergySimple(newImage, newImageWidth, newImageHeight, currentPixel, 1);
				newImageEnergy[currentPixel] = getPixelEnergySobel(newImage, newImageWidth, newImageHeight, currentPixel);
				// top down energy seam data of the original image
				newImageSeams[currentPixel] = newImageEnergy[currentPixel];
				newImageSeams2[currentPixel] = newImageEnergy[currentPixel];
			} else {
				newImageEnergy[currentPixel] = 0;
				newImageSeams[currentPixel] = 0;
				newImageSeams2[currentPixel] = 0;
			}
		}
	}
	
	// DEBUG: To test each direction
	if (forceDirection == 1) {
		fillSeamMatrixHorizontal(newImageSeams2, newImageWidth, newImageHeight);
		int horizontalSeamCost = findSeamsHorizontal(newImageSeams2, newImageWidth, newImageHeight, newImage2, imageSeamCounts2);
		return newImage2;
	} else if (forceDirection == 2) {
		fillSeamMatrixVertical(newImageSeams, newImageWidth, newImageHeight);
		int verticalSeamCost = findSeamsVertical(newImageSeams, newImageWidth, newImageHeight, newImage, imageSeamCounts);
		return newImage;
	} else {
		int verticalSeamCost = fillSeamMatrixVertical(newImageSeams, newImageWidth, newImageHeight);
		int horizontalSeamCost = fillSeamMatrixHorizontal(newImageSeams2, newImageWidth, newImageHeight);

		findSeamsVertical(newImageSeams, newImageWidth, newImageHeight, newImage, imageSeamCounts);
		findSeamsHorizontal(newImageSeams2, newImageWidth, newImageHeight, newImage2, imageSeamCounts2);
		printf("Sum traversal cost of all seams: vertical = %d, horizontal = %d\n", verticalSeamCost, horizontalSeamCost);
		/*
		free(smallImage);
		free(newImageEnergy);
		free(newImageSeams);
		free(newImageSeams2);
		*/
		//return newImageEnergy;

		if (horizontalSeamCost < verticalSeamCost) {
			//return newImageSeams2;
			//return newImageEnergy;
			return imageSeamCounts2;
			return newImage2;
		} else {
			//return newImageSeams;
			//return newImageEnergy;
			return imageSeamCounts;
			return newImage;
		}
	}

	//return newImage;
	//return newImage2;
	//return smallImage;
	//return newImageSeams;
	//return newImageSeams2;
	//return newImageEnergy;
}

#endif
