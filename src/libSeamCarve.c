/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#include <limits.h>

#define TRACE_NONE 0
#define TRACE_LEFT 1
#define TRACE_CENTER 2
#define TRACE_RIGHT 3

static inline int min2(int a, int b)
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

			// do not process the first row
			if (j > 0) {
				pixelAbove = currentPixel - imageWidth;
				if (i > 0) {
					if (i < imageWidth) {
						aboveL = newImageVector[pixelAbove - 1];
						aboveC = newImageVector[pixelAbove];
						aboveR = newImageVector[pixelAbove + 1];
					} else {
						aboveL = newImageVector[pixelAbove - 1];
						aboveC = newImageVector[pixelAbove];
						aboveR = INT_MAX;
					}
				} else {
					aboveL = INT_MAX;
					aboveC = newImageVector[pixelAbove];
					aboveR = newImageVector[pixelAbove + 1];
				}

				newValue = min3(aboveL, aboveC, aboveR);
				newImageVector[currentPixel] += newValue;

				if (newValue == aboveL) {
					newImageTraces[currentPixel] = TRACE_LEFT;
				} else if (newValue == aboveC) {
					newImageTraces[currentPixel] = TRACE_CENTER;
				} else {
					newImageTraces[currentPixel] = TRACE_RIGHT;
				}

			// the first row is taken as is
			} else {
				newImageVector[currentPixel] = imageVector[currentPixel];
				newImageTraces[currentPixel] = TRACE_NONE;
			}
		}
	}

	return newImageVector;
}
