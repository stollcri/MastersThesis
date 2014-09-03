/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBSEAMCARVE_C
#define LIBSEAMCARVE_C

#include <stdio.h>
#include <limits.h>

#define TRACE_NONE 0
#define TRACE_LEFT 1
#define TRACE_CENTER 2
#define TRACE_RIGHT 3

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
static int findEnergiesSimple(int *imageVector, int imageWidth, int imageHeight, int currentPixel)
{
	// We can pull from two pixels above instead of summing one above and one below
	int pixelAbove = 0;
	if (currentPixel > (imageWidth + imageWidth)) {
		pixelAbove = currentPixel - imageWidth - imageWidth;
	}

	int yDif = 0;
	if (imageVector[pixelAbove] > imageVector[currentPixel]) {
		yDif = imageVector[pixelAbove] - imageVector[currentPixel];
	} else {
		yDif = imageVector[currentPixel] - imageVector[pixelAbove];
	}

	int pixelLeft = 0;
	// TODO: fix this from rolling back to the other side
	pixelLeft = currentPixel - 2;
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

static void findSeams(int *imageSeams, int *imageTraces, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	int pixelAbove = 0;
	int aboveL = 0;
	int aboveC = 0;
	int aboveR = 0;
	int newValue = 0;

	// do not process the first row, start with j=1
	for (int j = 1; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;

			pixelAbove = currentPixel - imageWidth;
			// avoid falling off the left end
			if (i > 0) {
				// avoid falling off the right end
				if (i < imageWidth) {
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

			// record the track we have followed
			if (newValue == aboveC) {
				imageTraces[currentPixel] = TRACE_CENTER;
			} else if (newValue == aboveL) {
				imageTraces[currentPixel] = TRACE_LEFT;
			} else {
				imageTraces[currentPixel] = TRACE_RIGHT;
			}
		}
	}
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	int *newImage = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageEnergy = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageTraces = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageSeams = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));

	// create an image of the original image's energies
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			// mutable copy of the original image, to return the original image with seams shown
			newImage[currentPixel] = imageVector[currentPixel];
			// original energies of the original image, to return the energies with seams shown
			newImageEnergy[currentPixel] = findEnergiesSimple(imageVector, imageWidth, imageHeight, currentPixel);
			// top down energy seam data of the original image
			newImageSeams[currentPixel] = newImageEnergy[currentPixel];
			// traces through the original image (LCR directions to speed backtracking)
			newImageTraces[currentPixel] = TRACE_NONE;
		}
	}

	findSeams(newImageSeams, newImageTraces, imageWidth, imageHeight);

	// find the minimum seam energy in the bottom row
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;
	for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
		if (newImageSeams[i] < minValue) {
			minValue = newImageSeams[i];
			minValueLocation = i;
		}
		// below only shows when the above condition is "<=" -- bug? compiler optimization?
		//newImage[minValueLocation] = 92;
	}

	// from the minimum energy in the bottom row backtrack up the image
	for (int j = imageHeight; j > 0; --j) {
		newImageEnergy[minValueLocation] = 255;
		newImage[minValueLocation] = 0;

		if (newImageTraces[minValueLocation] == TRACE_LEFT) {
			minValueLocation -= (imageWidth + 1);
		} else if (newImageTraces[minValueLocation] == TRACE_CENTER) {
			minValueLocation -= imageWidth;
		} else if (newImageTraces[minValueLocation] == TRACE_RIGHT) {
			minValueLocation -= (imageWidth - 1);
		} else {
			minValueLocation -= imageWidth;
		}
	}

	return newImage;
	return newImageSeams;
	return newImageEnergy;
}

#endif
