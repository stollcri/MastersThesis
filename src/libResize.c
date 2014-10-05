/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBRESIZE_C
#define LIBRESIZE_C

//#include <stdio.h>

double linearInterpolation(double s, double e, double t)
{
	return s + (e - s) * t;
}

double bilinearInterpolation(double c00, double c10, double c01, double c11, double tx, double ty)
{
    return linearInterpolation(linearInterpolation(c00, c10, tx), linearInterpolation(c01, c11, tx), ty);
}

static void scaleBilinearBW(int *srcImgVector, int srcImgWidth, int srcImgHeight, int *dstImgVector, int dstImgWidth, int dstImgHeight)
{
	const double dblSrcW = (double)srcImgWidth;
	const double dblSrcH = (double)srcImgHeight;
	const double dblDstW = (double)dstImgWidth;
	const double dblDstH = (double)dstImgHeight;
	const double scaleX = dblSrcW / dblDstW;
	const double scaleY = dblSrcH / dblDstH;

	double dblX = 0;
	double dblY = 0;

	double gx = 0;
	double gy = 0;
	int gxi = 0;
	int gyi = 0;

	int location00 = 0;
	int location10 = 0;
	int location01 = 0;
	int location11 = 0;

	double pixel00 = 0;
	double pixel10 = 0;
	double pixel01 = 0;
	double pixel11 = 0;

	double result = 0;
	int currentPixel = 0;

	for (int y = 0; y < dstImgHeight; ++y) {
		for (int x = 0; x < dstImgWidth; ++x) {
			dblX = (double)x;
			dblY = (double)y;

			// gx = dblX / dblDstW * (dblSrcW - 1);
			// gy = dblY / dblDstH * (dblSrcW - 1);
			gx = dblX * scaleX;
			gy = dblY * scaleY;
			gxi = (int)gx;
			gyi = (int)gy;

			location00 = (gyi * srcImgWidth) + gxi;
			location10 = (gyi * srcImgWidth) + gxi + 1;
			location01 = ((gyi + 1) * srcImgWidth) + gxi;
			location11 = ((gyi + 1) * srcImgWidth) + gxi + 1;

			pixel00 = srcImgVector[location00];
			pixel10 = srcImgVector[location10];
			pixel01 = srcImgVector[location01];
			pixel11 = srcImgVector[location11];

			result = bilinearInterpolation(pixel00, pixel10, pixel01, pixel11, (gx - gxi), (gy -gyi));

			currentPixel = (y * dstImgWidth) + x;
			dstImgVector[currentPixel] = result;
		}
	}
}

static void resize(int *srcImgVector, int srcImgWidth, int srcImgHeight, int *dstImgVector, int dstImgWidth, int dstImgHeight)
{
	scaleBilinearBW(srcImgVector, srcImgWidth, srcImgHeight, dstImgVector, dstImgWidth, dstImgHeight);
}

static int getScaledSize(int srcSize, double scalePercentage)
{
	double srcSizeDbl = (double)srcSize;
	double resultSizeDbl = srcSizeDbl * scalePercentage;
	return (int)resultSizeDbl;
}

static void scale(int *srcImgVector, int srcImgWidth, int srcImgHeight, double scalePercentage, int *dstImgVector)
{
	int dstImgWidth = getScaledSize(srcImgWidth, scalePercentage);
	int dstImgHeight = getScaledSize(srcImgHeight, scalePercentage);
	scaleBilinearBW(srcImgVector, srcImgWidth, srcImgHeight, dstImgVector, dstImgWidth, dstImgHeight);
}

#endif
