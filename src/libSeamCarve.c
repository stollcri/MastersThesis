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

	int columnDeviation = 0;
	int startingColumn = 0;
	int *lastPath = (int*)malloc((unsigned long)imageHeight * sizeof(int));

	int lastEndingPixel = 0;
	int seamColor = 192;
	int seamBagan = 0;
	for (int k = ((imageWidth * imageHeight) - 1 - imageWidth); k < ((imageWidth * imageHeight) - 1); ++k) {
		if (imageSeams[k] <= minValue) {
			minValueLocation = k;

			startingColumn = (minValueLocation + 1) % imageWidth;

			countGoL = 0;
			countGoR = 0;
			columnDeviation = 0;

			// from the minimum energy in the bottom row backtrack up the image
			for (int j = imageHeight; j > 0; --j) {
				//newImageEnergy[minValueLocation] = 255;
				//imageOrig[minValueLocation] = seamColor;
				
				columnDeviation += startingColumn - ((minValueLocation + 1) % imageWidth);
				lastPath[j] = minValueLocation;

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

			//if (columnDeviation != 0) {
			//if ((countGoL > countGoR) || (countGoR > countGoL)) {
			//if (minValueLocation > (lastEndingPixel + 1)) {
				//seamColor += 64;
				if (columnDeviation > 0) {
				//if (seamBagan == 0) {
					if (seamBagan == 0) {
						int currentPixel = 0;
						for (int j = imageHeight; j > 0; --j) {
							currentPixel = lastPath[j];
							imageOrig[currentPixel] = 0;
						}

						seamBagan = 1;
						seamColor = 0;
					}
				} else {
					if (seamBagan == 1) {
						int currentPixel = 0;
						for (int j = imageHeight; j > 0; --j) {
							currentPixel = lastPath[j];
							imageOrig[currentPixel] = 192;
						}

						seamBagan = 0;
						seamColor = 192;
					}
				}
			//}
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

	// zero path weight (when a seam has zero weight) is +/- 1% (of image width)
	int zeroPathWeight = imageWidth / 100;
	// the minimum to prevent a text line (seam gap) from ending
	int minLineContinue = zeroPathWeight * 5;

	int rowDeviation = 0;
	int lastRowDeviation = 0;
	int rowDeviationFix = 0;
	int lastRowDeviationFix = 0;
	int startingRow = 0;
	int *thisPath = (int*)malloc((unsigned long)imageHeight * sizeof(int));
	int *lastPath = (int*)malloc((unsigned long)imageHeight * sizeof(int));

	int lastEndingPixel = 0;
	int seamColor = 0;
	int seamBagan = 0;
	int currentPixel = 0;
	for (int k = imageWidth; k < (imageWidth * imageHeight); k += imageWidth) {
		if (imageSeams[k] <= minValue) {
			minValueLocation = k;

			++startingRow;
			//printf("%d %d %d \n", k, minValueLocation, startingRow);
			countGoT = 0;
			countGoB = 0;
			rowDeviation = 0;

			// from the minimum energy in the bottom row backtrack up the image
			// TODO: Change base condition below to > instead of >=
			for (int j = imageWidth; j >= 0; --j) {
				//newImageEnergy[minValueLocation] = 255;
				
				//imageOrig[minValueLocation] = seamColor;
				
				//rowDeviation += (countGoT - countGoB);
				lastPath[j] = thisPath[j];
				thisPath[j] = minValueLocation;
				// if ((startingRow == 2) && (j <= 4000000)) {
				// 	printf("%d %d %d | %d - %d = %d \n", j, countGoT, countGoB, startingRow, (countGoT - countGoB + 1), rowDeviation);
				// }

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

				if (countGoT > countGoB) {
					rowDeviation -= 1;
				} else if (countGoT < countGoB) {
					rowDeviation += 1;
				}
			}

			/*
			if ((minValueLocation > 0) && (rowDeviation < -2)) {
				//seamBagan = 1;
				//seamColor = 32;

				if (seamBagan >= 1) {
					if (seamBagan > 2) {
						seamBagan = 0;
						seamColor = 192;
					} else {
						seamBagan += 1;
					}
				} else {
					seamBagan += 1;
					seamColor = 32;
				}
			} else {
				//seamBagan = 0;
				//seamColor = 92;
				if (seamBagan >= 1) {
					seamBagan += 1;
				}
			}

			printf("%d \t %d - %d = %d \t %d\n", rowDeviation, minValueLocation, lastEndingPixel, (minValueLocation - lastEndingPixel), seamBagan);
			if (seamBagan >= 1) {
				currentPixel = 0;
				for (int j = imageWidth; j >= 0; --j) {
					currentPixel = thisPath[j];
					imageOrig[currentPixel] = seamColor;
				}
			}
			*/
		
			if (lastRowDeviation < 0) {
				lastRowDeviationFix = lastRowDeviation * -1;
			} else {
				lastRowDeviationFix = lastRowDeviation;
			}

			if (rowDeviation < 0) {
				rowDeviationFix = rowDeviation * -1;
			} else {
				rowDeviationFix = rowDeviation;
			}

			//if ((lastRowDeviation < 0) && (rowDeviation > 0)) {
			// skip first line
			if ((k > imageWidth) && (lastRowDeviationFix <= zeroPathWeight) && (rowDeviationFix > zeroPathWeight)) {
				if (seamBagan < 1) {
					seamBagan += 1;
					for (int j = imageWidth; j >= 0; --j) {
						currentPixel = lastPath[j];
						imageOrig[currentPixel] = 128;
					}

					// for (int j = imageWidth; j >= 0; --j) {
					// 	currentPixel = thisPath[j];
					// 	imageOrig[currentPixel] = 32;
					// }

					printf("%d \t %d \t %d \t BEGIN \n", lastRowDeviationFix, rowDeviationFix, seamBagan);
				} else {
					printf("%d \t %d \t %d \t SKP \n", lastRowDeviationFix, rowDeviationFix, seamBagan);
				}
			} else {
				if (seamBagan >= 1) {
					if (rowDeviationFix <= minLineContinue) {
					//if (((lastRowDeviationFix == 0 ) && (rowDeviationFix == 0 )) || ((lastRowDeviationFix > 0) && (rowDeviationFix < 0))) {
					//if (lastRowDeviationFix < rowDeviationFix) {
						if (seamBagan > zeroPathWeight) {
							seamBagan = 0;
							for (int j = imageWidth; j >= 0; --j) {
								currentPixel = thisPath[j];
								imageOrig[currentPixel] = 0;
							}

							printf("%d \t %d \t %d \t END \n", lastRowDeviationFix, rowDeviationFix, seamBagan);
						} else {
							printf("%d \t %d \t %d \t GAP \n", lastRowDeviationFix, rowDeviationFix, seamBagan);
						}
					} else {
						seamBagan += 1;
						printf("%d \t %d \t %d \t RUN \n", lastRowDeviationFix, rowDeviationFix, seamBagan);
					}
				} else {
					printf("%d \t %d \t %d \t --- \n", lastRowDeviationFix, rowDeviationFix, seamBagan);
				}
			}
			lastRowDeviation = rowDeviation;

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
			newImageEnergy[currentPixel] = getPixelEnergySimple(imageVector, imageWidth, imageHeight, currentPixel, 1);
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
	//findSeamsVertical(newImageSeams, imageWidth, imageHeight, newImage);
	findSeamsHorizontal(newImageSeams2, imageWidth, imageHeight, newImage);

	return newImage;
	//return newImageSeams2;
	return newImageEnergy;
}

#endif
