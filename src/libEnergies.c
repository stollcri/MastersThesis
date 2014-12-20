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

static int getPixelEnergyLaplacian(struct pixel *imageVector, int imageWidth, int imageHeight, int currentPixel)
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
	int laplace = (4 * p5val) - p1val - p4val - p6val - p8val;

	return min(max(laplace, 0), 255);
}

static int getPixelGaussian(struct pixel *imageVector, int imageWidth, int imageHeight, int pixelDepth, int currentPixel, int sigma)
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
	pointValues[0] = (double)imageVector[points[0]].bright;
	pointValues[1] = (double)imageVector[points[1]].bright;
	pointValues[2] = (double)imageVector[points[2]].bright;
	pointValues[3] = (double)imageVector[points[3]].bright;
	pointValues[4] = (double)imageVector[points[4]].bright;
	pointValues[5] = (double)imageVector[points[5]].bright;
	pointValues[6] = (double)imageVector[points[6]].bright;
	pointValues[7] = (double)imageVector[points[7]].bright;
	pointValues[8] = (double)imageVector[points[8]].bright;
	pointValues[9] = (double)imageVector[points[9]].bright;
	pointValues[10] = (double)imageVector[points[10]].bright;
	pointValues[11] = (double)imageVector[points[11]].bright;
	pointValues[12] = (double)imageVector[points[12]].bright;
	pointValues[13] = (double)imageVector[points[13]].bright;
	pointValues[14] = (double)imageVector[points[14]].bright;
	pointValues[15] = (double)imageVector[points[15]].bright;
	pointValues[16] = (double)imageVector[points[16]].bright;
	pointValues[17] = (double)imageVector[points[17]].bright;
	pointValues[18] = (double)imageVector[points[18]].bright;
	pointValues[19] = (double)imageVector[points[19]].bright;
	pointValues[20] = (double)imageVector[points[20]].bright;
	pointValues[21] = (double)imageVector[points[21]].bright;
	pointValues[22] = (double)imageVector[points[22]].bright;
	pointValues[23] = (double)imageVector[points[23]].bright;
	pointValues[24] = (double)imageVector[points[24]].bright;
	
	double gaussL1 = 0.0;
	double gaussL2 = 0.0;
	double gaussL3 = 0.0;
	double gaussL4 = 0.0;
	double gaussL5 = 0.0;
	double gaussAll = 0.0;
	double gaussDvsr = 1.0;
	double weights[25];

	// scale courtesy: http://dev.theomader.com/gaussian-kernel-calculator/
	if (sigma == 80) {
		// scaling factor / standard deviation / sigma = 8.0
		weights[0]  = 0.038764;
		weights[1]  = 0.039682;
		weights[2]  = 0.039993;
		weights[6]  = 0.040622;
		weights[7]  = 0.040940;
		weights[12] = 0.041261;
	} else if (sigma == 50) {
		// scaling factor / standard deviation / sigma = 5.0
		weights[0]  = 0.036894;
		weights[1]  = 0.039167;
		weights[2]  = 0.039956;
		weights[6]  = 0.041581;
		weights[7]  = 0.042418;
		weights[12] = 0.043272;
	} else if (sigma == 40) {
		// scaling factor / standard deviation / sigma = 4.0
		weights[0]  = 0.035228;//0.01247764154323288;
		weights[1]  = 0.038671;//0.02641516735431067;
		weights[2]  = 0.039892;//0.03391774626899505;
		weights[6]  = 0.042452;//0.05592090972790156;
		weights[7]  = 0.043792;//0.07180386941492609;
		weights[12] = 0.045175;//0.09219799334529226;
	} else if (sigma == 24) {
		// scaling factor / standard deviation / sigma = 2.4
		weights[0]  = 0.027840;
		weights[1]  = 0.035986;
		weights[2]  = 0.039201;
		weights[6]  = 0.046517;
		weights[7]  = 0.050672;
		weights[12] = 0.055198;
	} else if (sigma == 22) {
		// scaling factor / standard deviation / sigma = 2.2
		weights[0]  = 0.025903;
		weights[1]  = 0.035128;
		weights[2]  = 0.038882;
		weights[6]  = 0.047638;
		weights[7]  = 0.052729;
		weights[12] = 0.058364;
	} else if (sigma == 21) {
		// scaling factor / standard deviation / sigma = 2.1
		weights[0]  = 0.024777;
		weights[1]  = 0.034594;
		weights[2]  = 0.038665;
		weights[6]  = 0.048301;
		weights[7]  = 0.053985;
		weights[12] = 0.060338;
	} else if (sigma == 20) {
		// scaling factor / standard deviation / sigma = 2.0
		weights[0]  = 0.023528;//0.007073763959958826;
		weights[1]  = 0.033969;//0.020430990591169273;
		weights[2]  = 0.038393;//0.029096162287720977;
		weights[6]  = 0.049045;//0.05901036264417242;
		weights[7]  = 0.055432;//0.08403777978803546;
		weights[12] = 0.062651;//0.11967980055109502;
	} else if (sigma == 16) {
		// scaling factor / standard deviation / sigma = 1.6
		weights[0]  = 0.017056;
		weights[1]  = 0.030076;
		weights[2]  = 0.036334;
		weights[6]  = 0.053035;
		weights[7]  = 0.064071;
		weights[12] = 0.077404;
	} else if (sigma == 15) {
		// scaling factor / standard deviation / sigma = 1.5
		weights[0]  = 0.015026;
		weights[1]  = 0.028569;
		weights[2]  = 0.035391;
		weights[6]  = 0.054318;
		weights[7]  = 0.067288;
		weights[12] = 0.083355;
	} else if (sigma == 14) {
		// scaling factor / standard deviation / sigma = 1.4
		gaussDvsr = 159;
		weights[0]  = 2;
		weights[1]  = 4;
		weights[2]  = 5;
		weights[6]  = 9;
		weights[7]  = 12;
		weights[12] = 15;
	} else if (sigma == 13) {
		// scaling factor / standard deviation / sigma = 1.3
		weights[0]  = 0.010534;
		weights[1]  = 0.024530;
		weights[2]  = 0.032508;
		weights[6]  = 0.057120;
		weights[7]  = 0.075698;
		weights[12] = 0.100318;
	} else if (sigma == 125) {
		// scaling factor / standard deviation / sigma = 1.25
		weights[0]  = 0.009355;
		weights[1]  = 0.023256;
		weights[2]  = 0.031498;
		weights[6]  = 0.057816;
		weights[7]  = 0.078305;
		weights[12] = 0.106055;
	} else if (sigma == 12) {
		// scaling factor / standard deviation / sigma = 1.2
		weights[0]  = 0.008173;
		weights[1]  = 0.021861;
		weights[2]  = 0.030337;
		weights[6]  = 0.058473;
		weights[7]  = 0.081144;
		weights[12] = 0.112606;
	} else if (sigma == 11) {
		// scaling factor / standard deviation / sigma = 1.1
		weights[0]  = 0.005865;
		weights[1]  = 0.018686;
		weights[2]  = 0.027481;
		weights[6]  = 0.059536;
		weights[7]  = 0.087555;
		weights[12] = 0.128760;
	} else if (sigma == 10) {
		// scaling factor / standard deviation / sigma = 1
		gaussDvsr = 273;
		weights[0]  = 1;
		weights[1]  = 4;
		weights[2]  = 7;
		weights[6]  = 16;
		weights[7]  = 26;
		weights[12] = 41;
	} else if (sigma == 5) {
		// scaling factor / standard deviation / sigma = 0.5
		weights[0]  = 0.000002;
		weights[1]  = 0.000212;
		weights[2]  = 0.000922;
		weights[6]  = 0.024745;
		weights[7]  = 0.107391;
		weights[12] = 0.466066;
	} else {
		weights[0]  = 1;
		weights[1]  = 2;
		weights[2]  = 4;
		weights[6]  = 8;
		weights[7]  = 16;
		weights[12] = 32;
	}
	// line 1 has 2 duplicated values
	weights[3] = weights[1];
	weights[4] = weights[0];
	// line 2 has 3 duplicated values
	weights[5] = weights[1];
	weights[8] = weights[6];
	weights[9] = weights[5];
	// line 3 has 4 duplicated values
	weights[10] = weights[2];
	weights[11] = weights[7];
	weights[13] = weights[11];
	weights[14] = weights[10];
	// line 4 is the same as line 2
	weights[15] = weights[5];
	weights[16] = weights[6];
	weights[17] = weights[7];
	weights[18] = weights[8];
	weights[19] = weights[9];
	// line 5 is the  same as line 1
	weights[20] = weights[0];
	weights[21] = weights[1];
	weights[22] = weights[2];
	weights[23] = weights[3];
	weights[24] = weights[4];

	gaussL1 = (weights[1]  * pointValues[0])  + (weights[1]  * pointValues[1])  + (weights[2]  * pointValues[2])  + (weights[3]  * pointValues[3])  + (weights[4]  * pointValues[4]);
	gaussL2 = (weights[5]  * pointValues[5])  + (weights[6]  * pointValues[6])  + (weights[7]  * pointValues[7])  + (weights[8]  * pointValues[8])  + (weights[9]  * pointValues[9]);
	gaussL3 = (weights[10] * pointValues[10]) + (weights[11] * pointValues[11]) + (weights[12] * pointValues[12]) + (weights[13] * pointValues[13]) + (weights[14] * pointValues[14]);
	gaussL4 = (weights[15] * pointValues[15]) + (weights[16] * pointValues[16]) + (weights[17] * pointValues[17]) + (weights[18] * pointValues[18]) + (weights[19] * pointValues[19]);
	gaussL5 = (weights[20] * pointValues[20]) + (weights[21] * pointValues[21]) + (weights[22] * pointValues[22]) + (weights[23] * pointValues[23]) + (weights[24] * pointValues[24]);
	gaussAll = (gaussL1 + gaussL2 + gaussL3 + gaussL4 + gaussL5) / gaussDvsr;
	return min(max((int)gaussAll, 0), 255);
}

static int getPixelEnergyDoG(struct pixel *imageVector, int currentPixel)
{
	int gaussianValue1 = imageVector[currentPixel].gaussA;
	int gaussianValue2 = imageVector[currentPixel].gaussB;
	
	double greyPixel = 0.0;
	/*
	double radianShift = 3.14159265;
	double radianRange = 3.14159265;
	double radianPixel = 0;
	double scaledPixel = 0;
	*/
	//double currentValue = getGreyValue(imageVector[currentPixel], imageVector[currentPixel+1], imageVector[currentPixel+2]);
	if (gaussianValue1 > gaussianValue2) {
		greyPixel = (gaussianValue1 - gaussianValue2);
		//return (gaussianValue1 - gaussianValue2);
		//return min(max((gaussianValue1 - gaussianValue2), 0), 255);
		//return min(max( (int)((gaussianValue1 - gaussianValue2) * 8), 0), 255);
	} else {
		greyPixel = (gaussianValue2 - gaussianValue1);
		//return (gaussianValue2 - gaussianValue1);
		//return min(max((gaussianValue2 - gaussianValue1), 0), 255);
		//return min(max( (int)((gaussianValue2 - gaussianValue1) * 8), 0), 255);
	}
	return min(max(greyPixel, 0), 255);

	// experiment with cosine threshholding
	/*
	radianPixel = (greyPixel / 255) * 3.14159265;
	scaledPixel = ((1 - cos(radianPixel)) / 2) * 255;
	return (int)scaledPixel;
	*/
}

#endif
