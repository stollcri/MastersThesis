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

static void findSeamDirection(int *imageSeams, int *imageTraces, int imageWidth, int currentPixel, int currentCol)
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

	// record the track we have followed
	if (newValue == aboveC) {
		imageTraces[currentPixel] = TRACE_CENTER;
	} else if (newValue == aboveL) {
		imageTraces[currentPixel] = TRACE_LEFT;
	} else {
		imageTraces[currentPixel] = TRACE_RIGHT;
	}
}

static void findSeams(int *imageSeams, int *imageTraces, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	// do not process the first row, start with j=1
	for (int j = 1; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			findSeamDirection(imageSeams, imageTraces, imageWidth, currentPixel, i);
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

	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;

	int *currentPath = (int*)malloc((unsigned long)imageWidth * sizeof(int));
	int pixelBelowL = 0;
	int pixelBelowC = 0;
	int pixelBelowR = 0;
	int nextPixel = 0;
	int currentCol = 0;

	for (int n=0; n<1; ++n) {

		// find the minimum seam energy in the bottom row
		minValue = INT_MAX;
		minValueLocation = INT_MAX;
		for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
			if (newImageSeams[i] < minValue) {
				minValue = newImageSeams[i];
				minValueLocation = i;
			}
			// below only shows when the above condition is "<=" -- bug? compiler optimization?
			//newImage[minValueLocation] = 92;
		}

		// from the minimum energy in the bottom row backtrack up the image
		for (int j = imageHeight; j >= 0; --j) {
			currentPath[j] = minValueLocation;

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

			newImageSeams[minValueLocation] = INT_MAX;
		}

		for (int k = 0; k < imageHeight; ++k) {
			pixelBelowL = currentPath[k] + imageWidth - 1;
			pixelBelowC = currentPath[k] + imageWidth;
			pixelBelowR = currentPath[k] + imageWidth + 1;
			nextPixel = currentPath[k+1];

			printf(">> %d %d %d %d :: ", nextPixel, pixelBelowL, pixelBelowC, pixelBelowR);

			if (nextPixel != pixelBelowL) {
				//if (newImageTraces[pixelBelowL] == TRACE_RIGHT) {
					printf("L");
					currentCol = pixelBelowL % imageWidth;
					printf("\t %d %d", newImageSeams[pixelBelowL], newImageTraces[pixelBelowL]);
					findSeamDirection(newImageSeams, newImageTraces, imageWidth, pixelBelowL, currentCol);
					printf("\t %d %d \t", newImageSeams[pixelBelowL], newImageTraces[pixelBelowL]);
				//}
			}

			if (nextPixel != pixelBelowC) {
				//if (newImageTraces[pixelBelowC] == TRACE_CENTER) {
					printf("C");
					currentCol = pixelBelowC % imageWidth;
					printf("\t %d %d", newImageSeams[pixelBelowC], newImageTraces[pixelBelowC]);
					findSeamDirection(newImageSeams, newImageTraces, imageWidth, pixelBelowC, currentCol);
					printf("\t %d %d \t", newImageSeams[pixelBelowC], newImageTraces[pixelBelowC]);
				//}
			}

			if (nextPixel != pixelBelowR) {
				//if (newImageTraces[pixelBelowR] == TRACE_LEFT) {
					printf("R");
					currentCol = pixelBelowR % imageWidth;
					printf("\t %d %d", newImageSeams[pixelBelowR], newImageTraces[pixelBelowR]);
					findSeamDirection(newImageSeams, newImageTraces, imageWidth, pixelBelowR, currentCol);
					printf("\t %d %d", newImageSeams[pixelBelowR], newImageTraces[pixelBelowR]);
				//}
			}
			printf("\n");

			//printf("%d: %d %d\n", k, currentPath[k], newImageSeams[currentPath[k]]);
			newImageSeams[currentPath[k]] = 0;
			//newImageSeams[pixelBelowL] = 255;
			//newImageSeams[pixelBelowR] = 255;
			//findSeamDirection(imageSeams, imageTraces, imageWidth, currentPixel, currentCol);
		}
	}

	//return newImage;
	return newImageSeams;
	return newImageEnergy;
}

#endif
