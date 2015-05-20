/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBCOLORCONV_C
#define LIBCOLORCONV_C

// [ X ]   [  0.412453  0.357580  0.180423 ]   [ R ] **
// [ Y ] = [  0.212671  0.715160  0.072169 ] * [ G ]
// [ Z ]   [  0.019334  0.119193  0.950227 ]   [ B ].

#define PIXEL_DEPTH 255.0
#define RGB_TO_XYZ_TRANS_MATRIX_00 0.412453
#define RGB_TO_XYZ_TRANS_MATRIX_01 0.357580
#define RGB_TO_XYZ_TRANS_MATRIX_02 0.180423
#define RGB_TO_XYZ_TRANS_MATRIX_10 0.212671
#define RGB_TO_XYZ_TRANS_MATRIX_11 0.715160
#define RGB_TO_XYZ_TRANS_MATRIX_12 0.072169
#define RGB_TO_XYZ_TRANS_MATRIX_20 0.019334
#define RGB_TO_XYZ_TRANS_MATRIX_21 0.119193
#define RGB_TO_XYZ_TRANS_MATRIX_22 0.950227

#define SIX_TWENTYNINTHS_TO_THIRD 0.008856451676
#define TWENTYNINE_SIXTHS_SQUARED 23.361111111
#define TWENTYNINE_SIXTHS_SQUARED_THIRD 7.787037037
#define FOUR_TWENTYNINTHS 0.1379310345
#define L_CONSTANT_A 116
#define L_CONSTANT_B 16
#define A_CONSTANT_A 500
#define B_CONSTANT_A 200

static double rgbToXyzX(int rin, int gin, int bin)
{
	double r = ((double)rin / PIXEL_DEPTH);
	double g = ((double)gin / PIXEL_DEPTH);
	double b = ((double)bin / PIXEL_DEPTH);
	return (r * RGB_TO_XYZ_TRANS_MATRIX_00) + (g * RGB_TO_XYZ_TRANS_MATRIX_01) + (b * RGB_TO_XYZ_TRANS_MATRIX_02);
}

static double rgbToXyzY(int rin, int gin, int bin)
{
	double r = ((double)rin / PIXEL_DEPTH);
	double g = ((double)gin / PIXEL_DEPTH);
	double b = ((double)bin / PIXEL_DEPTH);
	return (r * RGB_TO_XYZ_TRANS_MATRIX_10) + (g * RGB_TO_XYZ_TRANS_MATRIX_11) + (b * RGB_TO_XYZ_TRANS_MATRIX_12);
}

static double rgbToXyzZ(int rin, int gin, int bin)
{
	double r = ((double)rin / PIXEL_DEPTH);
	double g = ((double)gin / PIXEL_DEPTH);
	double b = ((double)bin / PIXEL_DEPTH);
	return (r * RGB_TO_XYZ_TRANS_MATRIX_20) + (g * RGB_TO_XYZ_TRANS_MATRIX_21) + (b * RGB_TO_XYZ_TRANS_MATRIX_22);
}


static double xyzToLabPreproc(double t)
{
	if (t > SIX_TWENTYNINTHS_TO_THIRD) {
		return pow(t, (1.0 / 3.0));
	} else {
		return (t * TWENTYNINE_SIXTHS_SQUARED_THIRD) + FOUR_TWENTYNINTHS;
	}
}

static double xyzToLabL(double xin, double yin, double zin)
{
	return (L_CONSTANT_A * xyzToLabPreproc(yin)) - L_CONSTANT_B;
}

static double xyzToLabA(double xin, double yin, double zin)
{
	return A_CONSTANT_A * (xyzToLabPreproc(xin) - xyzToLabPreproc(yin));
}

static double xyzToLabB(double xin, double yin, double zin)
{
	return B_CONSTANT_A * (xyzToLabPreproc(yin) - xyzToLabPreproc(zin));
}

#endif
