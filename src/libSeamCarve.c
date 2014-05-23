/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

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

static int energyE(int *imageVector, int imageWidth, int currentPixel)
{
	int pixelAbove = 0;
	if (currentPixel > imageWidth) {
		pixelAbove = currentPixel - imageWidth;
	}

	int xDif = 0;
	if (imageVector[pixelAbove] > imageVector[currentPixel]) {
		xDif = imageVector[pixelAbove] - imageVector[currentPixel];
	} else {
		xDif = imageVector[currentPixel] - imageVector[pixelAbove];
	}

	int pixelLeft = 0;
	// TODO: fix this from rolling back to the other side
	pixelLeft = currentPixel - 1;

	int yDif = 0;
	if (imageVector[pixelLeft] > imageVector[currentPixel]) {
		yDif = imageVector[pixelLeft] - imageVector[currentPixel];
	} else {
		yDif = imageVector[currentPixel] - imageVector[pixelLeft];
	}

	// int fin = (xDif + yDif);
	// printf("%d ", fin);
	return min((xDif + yDif), 255);
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	int pixelAbove = 0;
	int aboveL = 0;
	int aboveC = 0;
	int aboveR = 0;
	int newValue = 0;
	int *newImageVector = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageTraces = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));

	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			// printf("%d\n", currentPixel);
			newImageVector[currentPixel] = energyE(imageVector, imageWidth, currentPixel);
		}
	}

	// for (int j = 0; j < imageHeight; ++j) {
	// 	for (int i = 0; i < imageWidth; ++i) {
	// 		currentPixel = (j * imageWidth) + i;

	// 		// do not process the first row
	// 		if (j > 0) {
	// 			pixelAbove = currentPixel - imageWidth;
	// 			// avoid falling of the ends
	// 			if (i > 0) {
	// 				if (i < imageWidth) {
	// 					aboveL = newImageVector[pixelAbove - 1];
	// 					aboveC = newImageVector[pixelAbove];
	// 					aboveR = newImageVector[pixelAbove + 1];
	// 				} else {
	// 					aboveL = newImageVector[pixelAbove - 1];
	// 					aboveC = newImageVector[pixelAbove];
	// 					aboveR = INT_MAX;
	// 				}
	// 			} else {
	// 				aboveL = INT_MAX;
	// 				aboveC = newImageVector[pixelAbove];
	// 				aboveR = newImageVector[pixelAbove + 1];
	// 			}

	// 			// add the minimum above adjacent pixel to the current
	// 			newValue = min3(aboveL, aboveC, aboveR);
	// 			newImageVector[currentPixel] += newValue;
	// 			//newImageVector[currentPixel] = min(newImageVector[currentPixel], 255);
	// 			//printf("%3d ", newValue);

	// 			// record the track we have followed
	// 			if (newValue == aboveL) {
	// 				newImageTraces[currentPixel] = TRACE_LEFT;
	// 			} else if (newValue == aboveC) {
	// 				newImageTraces[currentPixel] = TRACE_CENTER;
	// 			} else {
	// 				newImageTraces[currentPixel] = TRACE_RIGHT;
	// 			}
	// 		}
	// 	}
	// }

	// TODO: find the minimum values along the bottom
	// 
	// TODO: backtrack to find the seams

	return newImageVector;
}
