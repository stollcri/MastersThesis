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
	int gaussA;
	int gaussB;
	int energy;
	int seamval;
	int usecount;
	int usecountR;
	int usecountG;
	int usecountB;
};

#endif
