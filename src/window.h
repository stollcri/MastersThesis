/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2015
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "libWrappers.c"

struct window {
	int xOrigin;
	int yOrigin;

	int xLength;
	int yLength;

	int xTerminus;
	int yTerminus;

	int xStep;
	int yStep;

	int pixelCount;
};

struct window *newWindow(int x, int y, int width, int height)
{
	struct window *newWindow = (struct window*)xmalloc(sizeof(struct window));

	newWindow->xOrigin = x;
	newWindow->yOrigin = y;
	newWindow->xLength = width;
	newWindow->yLength = height;
	newWindow->xTerminus = x + width;
	newWindow->yTerminus = y + height;
	newWindow->xStep = 1;
	newWindow->yStep = width;
	newWindow->pixelCount = width * height;

	return newWindow;
}

void freeWindow(struct window *thisWindow)
{
	if (thisWindow) {
		free(thisWindow);
	}
}

#endif
