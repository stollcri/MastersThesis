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
static int energySimple(int *imageVector, int imageWidth, int imageHeight, int currentPixel)
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

	// int fin = (yDif + xDif);
	// printf("%d ", fin);
	//return imageVector[currentPixel];
	if ((yDif + xDif) > 0) {
		return min((yDif + xDif), 255);
	} else {
		return 1;
	}
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	int *newImageEnergy = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageTraces = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageSeams = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));

	// create an image of the original image's energies
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			// printf("%d\n", currentPixel);
			// newImageEnergy[currentPixel] = imageVector[currentPixel];
			newImageEnergy[currentPixel] = energySimple(imageVector, imageWidth, imageHeight, currentPixel);
			newImageTraces[currentPixel] = TRACE_NONE;
			newImageSeams[currentPixel] = newImageEnergy[currentPixel];
		}
	}

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
			// avoid falling of the ends
			if (i > 0) {
				if (i < imageWidth) {
					aboveL = newImageSeams[pixelAbove - 1];
					aboveC = newImageSeams[pixelAbove];
					aboveR = newImageSeams[pixelAbove + 1];
					newValue = min3(aboveL, aboveC, aboveR);
				} else {
					aboveL = newImageSeams[pixelAbove - 1];
					aboveC = newImageSeams[pixelAbove];
					aboveR = INT_MAX;
					newValue = min(aboveL, aboveC);
				}
			} else {
				aboveL = INT_MAX;
				aboveC = newImageSeams[pixelAbove];
				aboveR = newImageSeams[pixelAbove + 1];
				newValue = min(aboveC, aboveR);
			}

			if ((j<20) && (i<120)) {
				printf("j: %d, i: %d == %d<%d>%d => %d + %d\n", j, i, aboveL, aboveC, aboveR, newImageSeams[currentPixel], newValue);
			}
			// add the minimum above adjacent pixel to the current
			//printf("%d %d \n", newImageSeams[currentPixel], newValue);
			//newImageSeams[currentPixel] -= 1;
			newImageSeams[currentPixel] += newValue;
			//newImageSeams[currentPixel] = min(newImageSeams[currentPixel], 255);
			
			//newImageEnergy[currentPixel] = min(newImageEnergy[currentPixel], 255);
			//printf("%3d ", newValue);

			// record the track we have followed
			if (newValue == aboveC) {
				newImageTraces[currentPixel] = TRACE_CENTER;
			} else if (newValue == aboveL) {
				newImageTraces[currentPixel] = TRACE_LEFT;
			} else {
				newImageTraces[currentPixel] = TRACE_RIGHT;
			}
		}
	}

	// TODO: find the minimum values along the bottom
	int minValueLocation = 0;
	int minValue = INT_MAX;
	for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth); --i) {
		// if (newImageEnergy[i] <= minValue) {
		if (newImageSeams[i] <= 5200) {
			minValueLocation = i;
			minValue = newImageSeams[minValueLocation];
			//printf("%d %d \n", minValue, minValueLocation);

			for (int j = imageHeight; j > 0; --j) {
				//printf("%d %d \n", minValueLocation, newImageTraces[minValueLocation]);
				newImageEnergy[minValueLocation] = 255;

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
		}
	}
	
	// TODO: backtrack to find the seams
	// for (int i = imageHeight; i > 0; --i) {
	// 	printf("%d %d \n", minValueLocation, newImageTraces[minValueLocation]);
	// 	newImageEnergy[minValueLocation] = 255;

	// 	if (newImageTraces[minValueLocation] == TRACE_LEFT) {
	// 		minValueLocation -= (imageWidth + 1);
	// 	} else if (newImageTraces[minValueLocation] == TRACE_CENTER) {
	// 		minValueLocation -= imageWidth;
	// 	} else if (newImageTraces[minValueLocation] == TRACE_RIGHT) {
	// 		minValueLocation -= (imageWidth - 1);
	// 	} else {
	// 		minValueLocation -= imageWidth;
	// 	}
	// }

	//return newImageSeams;
	return newImageEnergy;
}

#endif
