/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef PIXEL_H
#define PIXEL_H

struct pixel {
	int r;
	int g;
	int b;
	int a;
	int bright;
	int energy;
	int seamvalH;
	int seamvalV;
	int usecountH;
	int usecountV;
	int areaBoundaryH;
	int areaBoundaryV;
};

#endif
