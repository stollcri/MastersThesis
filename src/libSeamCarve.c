/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBSEAMCARVE_C
#define LIBSEAMCARVE_C

#include <stdio.h>
#include <limits.h>
#include "libResize.c"

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

/*
 * WARNING
 * 
 * This function was simply copied from FindSeamsHorizontal and modified to work for vertical seams
 * 
 * TODO: findSeamsVertical and findSeamsHorizontal do the same thing in perpendicular directions,
 * 		so we should find a way to combine the logic of these two functions
 */

static int findSeamsVertical(int *imageSeams, int imageWidth, int imageHeight, int *imageOrig)
{
	// the return value of the function, the sum traversal cost of all seams
	int totalSeamValue = 0;

	// find the minimum seam energy in the right-most column
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;
	// for (int i = imageWidth; i < (imageWidth * imageHeight); i += imageWidth) {
	for (int i = ((imageWidth * imageHeight) - 1); i > ((imageWidth * imageHeight) - imageWidth - 1); --i) {
		totalSeamValue += imageSeams[i]; 
		
		if (imageSeams[i] < minValue) {
			minValue = imageSeams[i];
			minValueLocation = i;
		}
		// TODO: break if min value is zero?
	}

	int aboveR = 0; // pixel to the left and above one line
	int aboveC = 0; // pixel to the left
	int aboveL = 0; // pixel to the left and below one line
	int currentMin = 0; // the minimum of aboveR, aboveC, and aboveL
	int countGoR = 0; // how many times the seam diverged upward
	int countGoL = 0; // how many times the seam diverged downward

	// the deviation variables record a seam's net divergence from being straight
	//  if a seam's average y position is above its starting line, then the deviation is negative
	//  if a seam's average y position is below its starting line, then the deviation is positive
	//  if a seam's average y position is the same as its starting line, then the deviation is 0
	//  (the sign was arbitrarily choosen and does not matter latter, the ABS will be taken)
	//  ( ^^^ TODO: so, maybe we can simplify the deviation calculation ^^^ )
	int colDeviation = 0; // current row's net deviation from being stright
	int lastColDeviation = 0; // last row's net deviation from being stright
	int colDeviationABS = 0; // absolute value of the current row's deviation
	int lastColDeviationABS = 0; // absolute value of last row's deviation
	int negDeviationCount = 0;
	int posDeviationCount = 0;
	double tmp = 0.0;
	int *thisPath = (int*)malloc((unsigned long)imageWidth * sizeof(int));

	// a seam is considered to have zero weight when it is less than this value
	// we raise it based upon the size of the image to help ignore dust and speckles
	int imageSeamZeroValue = imageWidth / 50; // 2% of the image width (TODO: improve heuristic)
	// the minimum deviation to prevent a text line (seam gap) from ending
	int minLineContinue = imageSeamZeroValue * 1; //2; // (TODO: improve heuristic)

	int textLineDepth = 0; // how far we are into a recognized text line
	int currentPixel = 0; // used for drawing begining and ending seam

	// for every pixel in the left-most column of the image
	//for (int k = (imageWidth - 1); k < ((imageWidth * imageHeight) - 1); k += imageWidth) {
	for (int k = ((imageWidth * imageHeight) - 1 - imageWidth); k < ((imageWidth * imageHeight) - 1); ++k) {
		// process seams with the lowest weights
		if (imageSeams[k] <= minValue) {
			// start from the left-most column
			minValueLocation = k;

			countGoR = 0;
			countGoL = 0;
			colDeviation = 0;

			// move right-to-left across the image
			//for (int j = (imageWidth - 1); j >= 0; --j) {
			for (int j = imageHeight; j > 0; --j) {
				// add pixel to the current path
				thisPath[j] = minValueLocation;

				// do we go above-left, left, or below-left?
				//aboveR = imageSeams[minValueLocation - 1 - imageWidth];
				aboveR = imageSeams[minValueLocation - imageWidth + 1];
				//aboveC = imageSeams[minValueLocation - 1];
				aboveC = imageSeams[minValueLocation - imageWidth];
				//aboveL = imageSeams[minValueLocation - 1 + imageWidth];
				aboveL = imageSeams[minValueLocation - imageWidth - 1];
				currentMin = min3(aboveR, aboveC, aboveL);

				// attempt to make the seam go back down if it was forced up and ice versa
				// the goal is to end on the same line which the seam started on, this
				// minimizes crazy diagonal seams which cut out important information
				if (countGoR == countGoL) {
					if (currentMin == aboveC) {
						//minValueLocation -= 1;
						minValueLocation -= imageWidth;
					} else if (currentMin == aboveR) {
						//minValueLocation -= (imageWidth + 1);
						minValueLocation -= (imageWidth - 1);
						++countGoR;
					} else if (currentMin == aboveL) {
						//minValueLocation += (imageWidth - 1);
						minValueLocation -= (imageWidth + 1);
						++countGoL;
					}
				} else if (countGoR > countGoL) {
					if (currentMin == aboveL) {
						//minValueLocation += (imageWidth - 1);
						minValueLocation -= (imageWidth + 1);
						++countGoL;
					} else if (currentMin == aboveC) {
						//minValueLocation -= 1;
						minValueLocation -= imageWidth;
					} else {
						//minValueLocation -= (imageWidth + 1);
						minValueLocation -= (imageWidth - 1);
						++countGoR;
					}
				} else if (countGoR < countGoL) {
					if (currentMin == aboveR) {
						//minValueLocation -= (imageWidth + 1);
						minValueLocation -= (imageWidth - 1);
						++countGoR;
					} else if (currentMin == aboveC) {
						//minValueLocation -= 1;
						minValueLocation -= imageWidth;
					} else {
						//minValueLocation += (imageWidth - 1);
						minValueLocation -= (imageWidth + 1);
						++countGoL;
					}
				}

				// increment the deviation counter according to this pixels deviation
				if (countGoR > countGoL) {
					colDeviation -= 1;
				} else if (countGoR < countGoL) {
					colDeviation += 1;
				}
			}
		
			// finf the absolute value of the deviation
			if (lastColDeviation < 0) {
				lastColDeviationABS = lastColDeviation * -1;
			} else {
				lastColDeviationABS = lastColDeviation;
			}

			// finf the absolute value of the deviation
			if (colDeviation < 0) {
				colDeviationABS = colDeviation * -1;
			} else {
				colDeviationABS = colDeviation;
			}

			// 
			// This is the begining of a new text line
			// 
			if ((lastColDeviationABS <= imageSeamZeroValue) && (colDeviationABS > imageSeamZeroValue)) {
				// 
				// Not presently in a text line
				// 
				if (textLineDepth < 1) {
					textLineDepth += 1;
					//for (int j = (imageWidth - 1); j >= 0; --j) {
					for (int j = imageHeight; j > 0; --j) {
						currentPixel = thisPath[j];
						imageOrig[currentPixel] = 128;
					}

					negDeviationCount = 0;
					posDeviationCount = 0;
					// printf("%d\t%d\t%d\t%d\t%d\t BEGIN \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth);
				
				// 
				// Already in a text line, so just keep going
				// 
				} else {
					if (colDeviation < 0) {
						negDeviationCount += 1;
					} else {
						posDeviationCount += 1;
					}
					// printf("%d\t%d\t%d\t%d\t%d\t SKP \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth);

					// TODO: Remove, DEBUG only
					//for (int j = (imageWidth - 1); j >= 0; --j) {
					for (int j = imageHeight; j > 0; --j) {
						currentPixel = thisPath[j];
						imageOrig[currentPixel] = 68;
					}
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
					if ((colDeviationABS <= 0) || (lastColDeviationABS < minLineContinue)) {
						// 
						// The text line has the required minimum number of image seam lines
						// 
						if (textLineDepth > imageSeamZeroValue) {
							textLineDepth = 0;
							//for (int j = (imageWidth - 1); j >= 0; --j) {
							for (int j = imageHeight; j > 0; --j) {
								currentPixel = thisPath[j];
								imageOrig[currentPixel] = 0;
							}

							tmp = (double)negDeviationCount / ((double)negDeviationCount + (double)posDeviationCount);
							// printf("%d\t%d\t%d\t%d\t%d\t END (%d / %d = %f) \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth, negDeviationCount, posDeviationCount, tmp);
						
						// 
						// The text line does not have the required minimum number of image seam lines
						// 
						} else {
							// 
							// However, this seam and the last one was straight, so halt out of this text line
							// 
							if ((colDeviationABS <= 0) && (lastColDeviationABS <= 0)) {
								textLineDepth = 0;
								//for (int j = (imageWidth - 1); j >= 0; --j) {
								for (int j = imageHeight; j > 0; --j) {
									currentPixel = thisPath[j];
									imageOrig[currentPixel] = 0;
								}

								// printf("%d\t%d\t%d\t%d\t%d\t HALT \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth);

							// 
							// Assume we are at the begining of a jagged text line, keep going
							// 
							} else {
								if (colDeviation < 0) {
									negDeviationCount += 1;
								} else {
									posDeviationCount += 1;
								}
								// printf("%d\t%d\t%d\t%d\t%d\t GAP \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth);

								// TODO: Remove, DEBUG only
								//for (int j = (imageWidth - 1); j >= 0; --j) {
								for (int j = imageHeight; j > 0; --j) {
									currentPixel = thisPath[j];
									imageOrig[currentPixel] = 72;
								}
							}
						}

					// 
					// Already in a text line, so just keep going
					// 
					} else {
						textLineDepth += 1;
						if (colDeviation < 0) {
							negDeviationCount += 1;
						} else {
							posDeviationCount += 1;
						}
						// printf("%d\t%d\t%d\t%d\t%d\t RUN \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth);

						// TODO: Remove, DEBUG only
						//for (int j = (imageWidth - 1); j >= 0; --j) {
						for (int j = imageHeight; j > 0; --j) {
							currentPixel = thisPath[j];
							imageOrig[currentPixel] = 64;
						}
					}
				
				// 
				// Not presently in a text line, just ignore and move on
				// 
				} else {
					// printf("%d\t%d\t%d\t%d\t%d\t --- \n", lastColDeviation, lastColDeviationABS, colDeviation, colDeviationABS, textLineDepth);
				}
			}

			lastColDeviation = colDeviation;
		}
	}
	return totalSeamValue;
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
	// the return value of the function, the sum traversal cost of all seams
	int totalSeamValue = 0;

	// find the minimum seam energy in the right-most column
	int minValue = INT_MAX;
	int minValueLocation = INT_MAX;
	for (int i = imageWidth; i < (imageWidth * imageHeight); i += imageWidth) {
		totalSeamValue += imageSeams[i]; 
		
		if (imageSeams[i] < minValue) {
			minValue = imageSeams[i];
			minValueLocation = i;
		}
		// TODO: break if min value is zero?
	}

	int leftT = 0; // pixel to the left and above one line
	int leftM = 0; // pixel to the left
	int leftB = 0; // pixel to the left and below one line
	int currentMin = 0; // the minimum of leftT, leftM, and leftB
	int countGoT = 0; // how many times the seam diverged upward
	int countGoB = 0; // how many times the seam diverged downward

	// the deviation variables record a seam's net divergence from being straight
	//  if a seam's average y position is above its starting line, then the deviation is negative
	//  if a seam's average y position is below its starting line, then the deviation is positive
	//  if a seam's average y position is the same as its starting line, then the deviation is 0
	//  (the sign was arbitrarily choosen and does not matter latter, the ABS will be taken)
	//  ( ^^^ TODO: so, maybe we can simplify the deviation calculation ^^^ )
	int rowDeviation = 0; // current row's net deviation from being stright
	int lastRowDeviation = 0; // last row's net deviation from being stright
	int rowDeviationABS = 0; // absolute value of the current row's deviation
	int lastRowDeviationABS = 0; // absolute value of last row's deviation
	int negDeviationCount = 0;
	int posDeviationCount = 0;
	double tmp = 0.0;
	int *thisPath = (int*)malloc((unsigned long)imageWidth * sizeof(int));

	// a seam is considered to have zero weight when it is less than this value
	// we raise it based upon the size of the image to help ignore dust and speckles
	int imageSeamZeroValue = imageWidth / 50; // 2% of the image width (TODO: improve heuristic)
	// the minimum deviation to prevent a text line (seam gap) from ending
	int minLineContinue = imageSeamZeroValue * 1; //2; // (TODO: improve heuristic)

	int textLineDepth = 0; // how far we are into a recognized text line
	int currentPixel = 0; // used for drawing begining and ending seam

	// for every pixel in the left-most column of the image
	for (int k = (imageWidth - 1); k < ((imageWidth * imageHeight) - 1); k += imageWidth) {
		// process seams with the lowest weights
		if (imageSeams[k] <= minValue) {
			// start from the left-most column
			minValueLocation = k;

			countGoT = 0;
			countGoB = 0;
			rowDeviation = 0;

			// move right-to-left across the image
			for (int j = (imageWidth - 1); j >= 0; --j) {
				// add pixel to the current path
				thisPath[j] = minValueLocation;

				// do we go above-left, left, or below-left?
				leftT = imageSeams[minValueLocation - 1 - imageWidth];
				leftM = imageSeams[minValueLocation - 1];
				leftB = imageSeams[minValueLocation - 1 + imageWidth];
				currentMin = min3(leftT, leftM, leftB);

				// attempt to make the seam go back down if it was forced up and ice versa
				// the goal is to end on the same line which the seam started on, this
				// minimizes crazy diagonal seams which cut out important information
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

				// increment the deviation counter according to this pixels deviation
				if (countGoT > countGoB) {
					rowDeviation -= 1;
				} else if (countGoT < countGoB) {
					rowDeviation += 1;
				}
			}
		
			// finf the absolute value of the deviation
			if (lastRowDeviation < 0) {
				lastRowDeviationABS = lastRowDeviation * -1;
			} else {
				lastRowDeviationABS = lastRowDeviation;
			}

			// finf the absolute value of the deviation
			if (rowDeviation < 0) {
				rowDeviationABS = rowDeviation * -1;
			} else {
				rowDeviationABS = rowDeviation;
			}

			// 
			// This is the begining of a new text line
			// 
			if ((lastRowDeviationABS <= imageSeamZeroValue) && (rowDeviationABS > imageSeamZeroValue)) {
				// 
				// Not presently in a text line
				// 
				if (textLineDepth < 1) {
					textLineDepth += 1;
					for (int j = (imageWidth - 1); j >= 0; --j) {
						currentPixel = thisPath[j];
						imageOrig[currentPixel] = 128;
					}

					negDeviationCount = 0;
					posDeviationCount = 0;
					// printf("%d\t%d\t%d\t%d\t%d\t BEGIN \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth);
				
				// 
				// Already in a text line, so just keep going
				// 
				} else {
					if (rowDeviation < 0) {
						negDeviationCount += 1;
					} else {
						posDeviationCount += 1;
					}
					// printf("%d\t%d\t%d\t%d\t%d\t SKP \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth);

					// TODO: Remove, DEBUG only
					for (int j = (imageWidth - 1); j >= 0; --j) {
						currentPixel = thisPath[j];
						imageOrig[currentPixel] = 68;
					}
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
					if ((rowDeviationABS <= 0) || (lastRowDeviationABS < minLineContinue)) {
						// 
						// The text line has the required minimum number of image seam lines
						// 
						if (textLineDepth > imageSeamZeroValue) {
							textLineDepth = 0;
							for (int j = (imageWidth - 1); j >= 0; --j) {
								currentPixel = thisPath[j];
								imageOrig[currentPixel] = 0;
							}

							tmp = (double)negDeviationCount / ((double)negDeviationCount + (double)posDeviationCount);
							// printf("%d\t%d\t%d\t%d\t%d\t END (%d / %d = %f) \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth, negDeviationCount, posDeviationCount, tmp);
						
						// 
						// The text line does not have the required minimum number of image seam lines
						// 
						} else {
							// 
							// However, this seam and the last one was straight, so halt out of this text line
							// 
							if ((rowDeviationABS <= 0) && (lastRowDeviationABS <= 0)) {
								textLineDepth = 0;
								for (int j = (imageWidth - 1); j >= 0; --j) {
									currentPixel = thisPath[j];
									imageOrig[currentPixel] = 0;
								}

								// printf("%d\t%d\t%d\t%d\t%d\t HALT \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth);

							// 
							// Assume we are at the begining of a jagged text line, keep going
							// 
							} else {
								if (rowDeviation < 0) {
									negDeviationCount += 1;
								} else {
									posDeviationCount += 1;
								}
								// printf("%d\t%d\t%d\t%d\t%d\t GAP \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth);

								// TODO: Remove, DEBUG only
								for (int j = (imageWidth - 1); j >= 0; --j) {
									currentPixel = thisPath[j];
									imageOrig[currentPixel] = 72;
								}
							}
						}

					// 
					// Already in a text line, so just keep going
					// 
					} else {
						textLineDepth += 1;
						if (rowDeviation < 0) {
							negDeviationCount += 1;
						} else {
							posDeviationCount += 1;
						}
						// printf("%d\t%d\t%d\t%d\t%d\t RUN \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth);

						// TODO: Remove, DEBUG only
						for (int j = (imageWidth - 1); j >= 0; --j) {
							currentPixel = thisPath[j];
							imageOrig[currentPixel] = 64;
						}
					}
				
				// 
				// Not presently in a text line, just ignore and move on
				// 
				} else {
					// printf("%d\t%d\t%d\t%d\t%d\t --- \n", lastRowDeviation, lastRowDeviationABS, rowDeviation, rowDeviationABS, textLineDepth);
				}
			}

			lastRowDeviation = rowDeviation;
		}
	}
	return totalSeamValue;
}

static int *seamCarve(int *imageVector, int imageWidth, int imageHeight)
{
	double imageScale = 0.25;
	int smallImageWidth = getScaledSize(imageWidth, imageScale);
	int smallImageHeight = getScaledSize(imageHeight, imageScale);
	int *smallImage = (int*)malloc((unsigned long)smallImageHeight * (unsigned long)smallImageWidth * sizeof(int));
	resize(imageVector, imageWidth, imageHeight, smallImage, smallImageWidth, smallImageHeight);
	
	int *newImage = (int*)malloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	int *newImageEnergy = (int*)malloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	
	int *newImageSeams = (int*)malloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	int *newImageSeams2 = (int*)malloc((unsigned long)smallImageWidth * (unsigned long)smallImageHeight * sizeof(int));
	
	int currentPixel = 0;

	// create an image of the original image's energies
	for (int j = 0; j < smallImageHeight; ++j) {
		for (int i = 0; i < smallImageWidth; ++i) {
			currentPixel = (j * smallImageWidth) + i;
			// mutable copy of the original image, to return the original image with seams shown
			newImage[currentPixel] = smallImage[currentPixel];
			// original energies of the original image, to return the energies with seams shown
			newImageEnergy[currentPixel] = getPixelEnergySimple(smallImage, smallImageWidth, smallImageHeight, currentPixel, 1);
			// top down energy seam data of the original image
			newImageSeams[currentPixel] = newImageEnergy[currentPixel];
			newImageSeams2[currentPixel] = newImageEnergy[currentPixel];
		}
	}

	fillSeamMatrixVertical(newImageSeams, smallImageWidth, smallImageHeight);
	fillSeamMatrixHorizontal(newImageSeams2, smallImageWidth, smallImageHeight);

	// for (int j = 0; j < smallImageHeight; ++j) {
	// 	for (int i = 0; i < smallImageWidth; ++i) {
	// 		currentPixel = (j * smallImageWidth) + i;
	// 		if ((newImageSeams[currentPixel] == 0) || (newImageSeams2[currentPixel] == 0)) {
	// 			newImageSeams[currentPixel] = 0;
	// 			//newImageSeams2[currentPixel] = newImageSeams[currentPixel];
	// 		} else {
	// 			newImageSeams[currentPixel] = ((newImageSeams[currentPixel] + newImageSeams2[currentPixel]) / 2);
	// 			newImageSeams2[currentPixel] = newImageSeams[currentPixel];
	// 		}
	// 	}
	// }

	int verticalSeamCost = findSeamsVertical(newImageSeams, smallImageWidth, smallImageHeight, newImage);
	int horizontalSeamCost = findSeamsHorizontal(newImageSeams2, smallImageWidth, smallImageHeight, newImage);
	printf("Sum traversal cost of all seams: vertical = %d, horizontal = %d\n", verticalSeamCost, horizontalSeamCost);

	return newImage;
	//return smallImage;
	//return newImageSeams;
	//return newImageSeams2;
	//return newImageEnergy;
}

#endif
