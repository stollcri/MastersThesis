/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBENERGIES_C
#define LIBENERGIES_C

#include "pixel.h"
#include "libMinMax.c"

// Simple energy function, basically a gradient magnitude calculation
static int getPixelEnergySimple(struct pixel *imageVector, int imageWidth, int imageHeight, int currentPixel, int gradientSize)
{
	// We can pull from two pixels above instead of summing one above and one below
	int pixelAbove = 0;
	if (currentPixel > (imageWidth * gradientSize)) {
		pixelAbove = currentPixel - (imageWidth * gradientSize);
	}

	int yDif = 0;
	if (imageVector[pixelAbove].bright > imageVector[currentPixel].bright) {
		yDif = imageVector[pixelAbove].bright - imageVector[currentPixel].bright;
	} else {
		yDif = imageVector[currentPixel].bright - imageVector[pixelAbove].bright;
	}

	int pixelLeft = 0;
	pixelLeft = currentPixel - gradientSize;
	if (pixelLeft < 0) {
		pixelLeft = 0;
	}

	int pixelCol = currentPixel % imageWidth;
	int xDif = 0;
	if (pixelCol > 0) {
		if (imageVector[pixelLeft].bright > imageVector[currentPixel].bright) {
			xDif = imageVector[pixelLeft].bright - imageVector[currentPixel].bright;
		} else {
			xDif = imageVector[currentPixel].bright - imageVector[pixelLeft].bright;
		}
	}

	return min((yDif + xDif), 255);
}

static int getPixelEnergySobel(struct pixel *imageVector, int imageWidth, int imageHeight, int currentPixel)
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
        (currentCol < (imageByteWidth - pixelDepth))) {
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
        return 0;//33; // zero and INT_MAX are significant, so return 1
    }
    
    // get the pixel values from the image array
    int p1val = imageVector[p1].bright;
    int p2val = imageVector[p2].bright;
    int p3val = imageVector[p3].bright;
    
    int p4val = imageVector[p4].bright;
    int p5val = imageVector[p5].bright;
    int p6val = imageVector[p6].bright;

    int p7val = imageVector[p7].bright;
    int p8val = imageVector[p8].bright;
    int p9val = imageVector[p9].bright;
    
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



/*
static double getGreyValue(double r, double g, double b)
{
    return ((r * COLOR_TO_GREY_FACTOR_R) + (g * COLOR_TO_GREY_FACTOR_G) + (b * COLOR_TO_GREY_FACTOR_B));
}

static int getPixelGaussian(unsigned char *imageVector, int imageWidth, int imageHeight, int pixelDepth, int currentPixel, int sigma)
{
    int imageByteWidth = imageWidth * pixelDepth;
    int points[25];
    double pointValues[25];
        
    points[0] = currentPixel - imageByteWidth - imageByteWidth - pixelDepth - pixelDepth;
    points[1] = currentPixel - imageByteWidth - imageByteWidth - pixelDepth;
    points[2] = currentPixel - imageByteWidth - imageByteWidth;
    points[3] = currentPixel - imageByteWidth - imageByteWidth + pixelDepth;
    points[4] = currentPixel - imageByteWidth - imageByteWidth + pixelDepth + pixelDepth;
    
    points[5] = currentPixel - imageByteWidth - pixelDepth - pixelDepth;
    points[6] = currentPixel - imageByteWidth - pixelDepth;
    points[7] = currentPixel - imageByteWidth;
    points[8] = currentPixel - imageByteWidth + pixelDepth;
    points[9] = currentPixel - imageByteWidth + pixelDepth + pixelDepth;
    
    points[10] = currentPixel - pixelDepth - pixelDepth;
    points[11] = currentPixel - pixelDepth;
    points[12] = currentPixel;
    points[13] = currentPixel + pixelDepth;
    points[14] = currentPixel + pixelDepth + pixelDepth;
    
    points[15] = currentPixel + imageByteWidth - pixelDepth - pixelDepth;
    points[16] = currentPixel + imageByteWidth - pixelDepth;
    points[17] = currentPixel + imageByteWidth;
    points[18] = currentPixel + imageByteWidth + pixelDepth;
    points[19] = currentPixel + imageByteWidth + pixelDepth + pixelDepth;
    
    points[20] = currentPixel + imageByteWidth + imageByteWidth - pixelDepth - pixelDepth;
    points[21] = currentPixel + imageByteWidth + imageByteWidth - pixelDepth;
    points[22] = currentPixel + imageByteWidth + imageByteWidth;
    points[23] = currentPixel + imageByteWidth + imageByteWidth + pixelDepth;
    points[24] = currentPixel + imageByteWidth + imageByteWidth + pixelDepth + pixelDepth;
    
    // TODO: this is wrong, fix it
    for (int i = 0; i < 25; ++i) {
        if (points[i] < 0) {
            points[i] = 0;
        } else if (points[i] >= (imageHeight * imageWidth * pixelDepth)) {
            points[i] = (imageHeight * imageWidth * pixelDepth);
        }
    }

    // get the pixel values from the image array
    pointValues[0] = getGreyValue(imageVector[points[0]], imageVector[points[0]+1], imageVector[points[0]+2]);
    pointValues[1] = getGreyValue(imageVector[points[1]], imageVector[points[1]+1], imageVector[points[1]+2]);
    pointValues[2] = getGreyValue(imageVector[points[2]], imageVector[points[2]+1], imageVector[points[2]+2]);
    pointValues[3] = getGreyValue(imageVector[points[3]], imageVector[points[3]+1], imageVector[points[3]+2]);
    pointValues[4] = getGreyValue(imageVector[points[4]], imageVector[points[4]+1], imageVector[points[4]+2]);
    pointValues[5] = getGreyValue(imageVector[points[5]], imageVector[points[5]+1], imageVector[points[5]+2]);
    pointValues[6] = getGreyValue(imageVector[points[6]], imageVector[points[6]+1], imageVector[points[6]+2]);
    pointValues[7] = getGreyValue(imageVector[points[7]], imageVector[points[7]+1], imageVector[points[7]+2]);
    pointValues[8] = getGreyValue(imageVector[points[8]], imageVector[points[8]+1], imageVector[points[8]+2]);
    pointValues[9] = getGreyValue(imageVector[points[9]], imageVector[points[9]+1], imageVector[points[9]+2]);
    pointValues[10] = getGreyValue(imageVector[points[10]], imageVector[points[10]+1], imageVector[points[10]+2]);
    pointValues[11] = getGreyValue(imageVector[points[11]], imageVector[points[11]+1], imageVector[points[11]+2]);
    pointValues[12] = getGreyValue(imageVector[points[12]], imageVector[points[12]+1], imageVector[points[12]+2]);
    pointValues[13] = getGreyValue(imageVector[points[13]], imageVector[points[13]+1], imageVector[points[13]+2]);
    pointValues[14] = getGreyValue(imageVector[points[14]], imageVector[points[14]+1], imageVector[points[14]+2]);
    pointValues[15] = getGreyValue(imageVector[points[15]], imageVector[points[15]+1], imageVector[points[15]+2]);
    pointValues[16] = getGreyValue(imageVector[points[16]], imageVector[points[16]+1], imageVector[points[16]+2]);
    pointValues[17] = getGreyValue(imageVector[points[17]], imageVector[points[17]+1], imageVector[points[17]+2]);
    pointValues[18] = getGreyValue(imageVector[points[18]], imageVector[points[18]+1], imageVector[points[18]+2]);
    pointValues[19] = getGreyValue(imageVector[points[19]], imageVector[points[19]+1], imageVector[points[19]+2]);
    pointValues[20] = getGreyValue(imageVector[points[20]], imageVector[points[20]+1], imageVector[points[20]+2]);
    pointValues[21] = getGreyValue(imageVector[points[21]], imageVector[points[21]+1], imageVector[points[21]+2]);
    pointValues[22] = getGreyValue(imageVector[points[22]], imageVector[points[22]+1], imageVector[points[22]+2]);
    pointValues[23] = getGreyValue(imageVector[points[23]], imageVector[points[23]+1], imageVector[points[23]+2]);
    pointValues[24] = getGreyValue(imageVector[points[24]], imageVector[points[24]+1], imageVector[points[24]+2]);
    
    double gaussL1 = 0.0;
    double gaussL2 = 0.0;
    double gaussL3 = 0.0;
    double gaussL4 = 0.0;
    double gaussL5 = 0.0;
    double gaussAll = 0.0;
    if (sigma == 14) {
        // apply the gaussian kernel (sigma = 1.4)
        gaussL1 = (2 * pointValues[0]) + (4 * pointValues[1]) + (5 * pointValues[2]) + (4 * pointValues[3]) + (2 * pointValues[4]);
        gaussL2 = (4 * pointValues[5]) + (9 * pointValues[6]) + (12 * pointValues[7]) + (9 * pointValues[8]) + (4 * pointValues[9]);
        gaussL3 = (5 * pointValues[10]) + (12 * pointValues[11]) + (15 * pointValues[12]) + (12 * pointValues[13]) + (5 * pointValues[14]);
        gaussL4 = (4 * pointValues[15]) + (9 * pointValues[16]) + (12 * pointValues[17]) + (9 * pointValues[18]) + (4 * pointValues[19]);
        gaussL5 = (2 * pointValues[20]) + (4 * pointValues[21]) + (5 * pointValues[22]) + (4 * pointValues[23]) + (2 * pointValues[24]);
        gaussAll = (gaussL1 + gaussL2 + gaussL3 + gaussL4 + gaussL5) / 159;
    } else {
        // apply the gaussian kernel (sigma = 1)
        gaussL1 = (1 * pointValues[0]) + (4 * pointValues[1]) + (7 * pointValues[2]) + (4 * pointValues[3]) + (1 * pointValues[4]);
        gaussL2 = (4 * pointValues[5]) + (16 * pointValues[6]) + (26 * pointValues[7]) + (16 * pointValues[8]) + (4 * pointValues[9]);
        gaussL3 = (7 * pointValues[10]) + (26 * pointValues[11]) + (41 * pointValues[12]) + (26 * pointValues[13]) + (7 * pointValues[14]);
        gaussL4 = (4 * pointValues[15]) + (16 * pointValues[16]) + (26 * pointValues[17]) + (16 * pointValues[18]) + (4 * pointValues[19]);
        gaussL5 = (1 * pointValues[20]) + (4 * pointValues[21]) + (7 * pointValues[22]) + (4 * pointValues[23]) + (1 * pointValues[24]);
        gaussAll = (gaussL1 + gaussL2 + gaussL3 + gaussL4 + gaussL5) / 273;
    }
    
    return min(max((int)gaussAll, 0), 255);
}

static int getPixelEnergyDoG(unsigned char *imageVector, int currentPixel, int gaussianValue1, int gaussianValue2)
static int getPixelEnergyDoG(int *imageVector, int imageWidth, int imageHeight, int currentPixel)
{
    //double currentValue = getGreyValue(imageVector[currentPixel], imageVector[currentPixel+1], imageVector[currentPixel+2]);
    if (gaussianValue1 > gaussianValue2) {
        return min(max( (int)((gaussianValue1 - gaussianValue2) * 8), 0), 255);
    } else {
        return min(max( (int)((gaussianValue2 - gaussianValue1) * 8), 0), 255);
    }
}
*/
#endif
