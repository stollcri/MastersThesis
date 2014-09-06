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
#define TRACE_TOP 4
#define TRACE_MIDDLE 5
#define TRACE_BOTTOM 6

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
static int getPixelEnergySimple(int *imageVector, int imageWidth, int imageHeight, int currentPixel)
{
	// We can pull from two pixels above instead of summing one above and one below
	int pixelAbove = 0;
	if (currentPixel > (imageWidth + imageWidth + imageWidth + imageWidth)) {
		pixelAbove = currentPixel - imageWidth - imageWidth - imageWidth - imageWidth;
	}

	int yDif = 0;
	if (imageVector[pixelAbove] > imageVector[currentPixel]) {
		yDif = imageVector[pixelAbove] - imageVector[currentPixel];
	} else {
		yDif = imageVector[currentPixel] - imageVector[pixelAbove];
	}

	int pixelLeft = 0;
	// TODO: fix this from rolling back to the other side?
	pixelLeft = currentPixel - 4;
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

static void findSeamsVertical(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig)
{
	// find the minimum seam energy in the bottom row
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;
	for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
		if (imageSeams[i] < minValue) {
			minValue = imageSeams[i];
			minValueLocation = i;
		}
		// TODO: break if min value is zero
		// 
		// below only shows when the above condition is "<=" -- bug? compiler optimization?
		//newImage[minValueLocation] = 92;
	}

	int aboveL = 0;
	int aboveC = 0;
	int aboveR = 0;
	int currentMin = 0;
	int countGoL = 0;
	int countGoR = 0;

	int lastEndingPixel = 0;
	int seamColor = 0;
	for (int k = ((imageWidth * imageHeight) - imageWidth - 1); k < ((imageWidth * imageHeight) - 1); ++k) {
		if (imageSeams[k] <= minValue) {
			minValueLocation = k;

			countGoL = 0;
			countGoR = 0;

			// from the minimum energy in the bottom row backtrack up the image
			for (int j = imageHeight; j >= 0; --j) {
				//newImageEnergy[minValueLocation] = 255;
				imageOrig[minValueLocation] = seamColor;

				// new
				aboveL = imageSeams[minValueLocation - imageWidth - 1];
				aboveC = imageSeams[minValueLocation - imageWidth];
				aboveR = imageSeams[minValueLocation - imageWidth + 1];
				currentMin = min3(aboveL, aboveC, aboveR);
				if (countGoL == countGoR) {
					if (currentMin == aboveC) {
						minValueLocation -= imageWidth;
					} else if (currentMin == aboveL) {
						minValueLocation -= (imageWidth + 1);
						++countGoL;
					} else if (currentMin == aboveR) {
						minValueLocation -= (imageWidth - 1);
						++countGoR;
					}
				} else if (countGoL > countGoR) {
					if (currentMin == aboveR) {
						minValueLocation -= (imageWidth - 1);
						++countGoR;
					} else if (currentMin == aboveC) {
						minValueLocation -= imageWidth;
					} else if (currentMin == aboveL) {
						minValueLocation -= (imageWidth + 1);
						++countGoL;
					}
				} else if (countGoL < countGoR) {
					if (currentMin == aboveL) {
						minValueLocation -= (imageWidth + 1);
						++countGoL;
					} else if (currentMin == aboveC) {
						minValueLocation -= imageWidth;
					} else if (currentMin == aboveR) {
						minValueLocation -= (imageWidth - 1);
						++countGoR;
					}
				}
			}

			//if ((countGoL > 0) || (countGoR > 0)) {
			if (minValueLocation > (lastEndingPixel + 1)) {
				seamColor += 64;
			}
			lastEndingPixel = minValueLocation;
		}
	}
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

static void findSeamsHorizontal(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig)
{
	// find the minimum seam energy in the right column
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;
	for (int i = imageWidth; i < (imageWidth * imageHeight); i += imageWidth) {
		if (imageSeams[i] < minValue) {
			minValue = imageSeams[i];
			minValueLocation = i;
		}
		// TODO: break if min value is zero
		// 
		// below only shows when the above condition is "<=" -- bug? compiler optimization?
		//newImage[minValueLocation] = 92;
	}

	int leftT = 0;
	int leftM = 0;
	int leftB = 0;
	int currentMin = 0;
	int countGoT = 0;
	int countGoB = 0;

	int lastEndingPixel = 0;
	int seamColor = 0;
	for (int k = imageWidth; k < (imageWidth * imageHeight); k += imageWidth) {
		if (imageSeams[k] <= minValue) {
			minValueLocation = k;

			countGoT = 0;
			countGoB = 0;

			// from the minimum energy in the bottom row backtrack up the image
			for (int j = imageWidth; j >= 0; --j) {
				//newImageEnergy[minValueLocation] = 255;
				imageOrig[minValueLocation] = seamColor;

				// new
				
				leftT = imageSeams[minValueLocation - imageWidth - 1];
				leftM = imageSeams[minValueLocation - imageWidth];
				leftB = imageSeams[minValueLocation - imageWidth + 1];
				currentMin = min3(leftT, leftM, leftB);
				// attempt to make the seam go back down if it was forced up and ice versa
				if (countGoT == countGoB) {
					if (currentMin == leftM) {
						minValueLocation -= 1;
					} else if (currentMin == leftT) {
						minValueLocation -= (imageWidth + 1);
						++countGoT;
					} else if (currentMin == leftB) {
						minValueLocation += (imageWidth - 1);
						++countGoB;
					}
				} else if (countGoT > countGoB) {
					if (currentMin == leftB) {
						minValueLocation += (imageWidth - 1);
						++countGoB;
					} else if (currentMin == leftM) {
						minValueLocation -= 1;
					} else {
						minValueLocation -= (imageWidth + 1);
						++countGoT;
					}
				} else if (countGoT < countGoB) {
					if (currentMin == leftT) {
						minValueLocation -= (imageWidth + 1);
						++countGoT;
					} else if (currentMin == leftM) {
						minValueLocation -= 1;
					} else {
						minValueLocation += (imageWidth - 1);
						++countGoB;
					}
				}
			}
			//if ((countGoT > 0) || (countGoB > 0)) {
			if (minValueLocation > (lastEndingPixel + imageWidth)) {
				seamColor += 64;
			}
			lastEndingPixel = minValueLocation;
		}
	}
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight)
{
	int currentPixel = 0;
	int *newImage = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageEnergy = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	
	int *newImageSeams = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));
	int *newImageSeams2 = (int*)malloc((unsigned long)imageHeight * (unsigned long)imageWidth * sizeof(int));

	// create an image of the original image's energies
	for (int j = 0; j < imageHeight; ++j) {
		for (int i = 0; i < imageWidth; ++i) {
			currentPixel = (j * imageWidth) + i;
			// mutable copy of the original image, to return the original image with seams shown
			newImage[currentPixel] = imageVector[currentPixel];
			// original energies of the original image, to return the energies with seams shown
			newImageEnergy[currentPixel] = getPixelEnergySimple(imageVector, imageWidth, imageHeight, currentPixel);
			// top down energy seam data of the original image
			newImageSeams[currentPixel] = newImageEnergy[currentPixel];
			newImageSeams2[currentPixel] = newImageEnergy[currentPixel];
		}
	}

	fillSeamMatrixVertical(newImageSeams, imageWidth, imageHeight);
	fillSeamMatrixHorizontal(newImageSeams2, imageWidth, imageHeight);

	// for (int j = 0; j < imageHeight; ++j) {
	// 	for (int i = 0; i < imageWidth; ++i) {
	// 		currentPixel = (j * imageWidth) + i;
	// 		if ((newImageSeams[currentPixel] == 0) || (newImageSeams2[currentPixel] == 0)) {
	// 			newImageSeams[currentPixel] = 0;
	// 			newImageSeams2[currentPixel] = 0;
	// 		} else {
	// 			newImageSeams[currentPixel] = ((newImageSeams[currentPixel] + newImageSeams2[currentPixel]) / 2);
	// 			newImageSeams2[currentPixel] = newImageSeams[currentPixel];
	// 		}
	// 	}
	// }

	//findSeamsHorizontal(newImageSeams2, imageWidth, imageHeight, newImage);
	findSeamsVertical(newImageSeams, imageWidth, imageHeight, newImage);
	//findSeamsHorizontal(newImageSeams2, imageWidth, imageHeight, newImage);

	return newImage;
	return newImageSeams2;
	return newImageEnergy;
}

#endif
