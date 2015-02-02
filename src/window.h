/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2015
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "libWrappers.c"

struct window {
	int fullWidth;
	int fullHeight; // not really used?

	int xOrigin;
	int yOrigin;

	int xLength;
	int yLength;

	int xTerminus;
	int yTerminus;

	int xStep;
	int yStep;

	int firstPixel;
	int lastPixel;

	int pixelCount;
};

struct window *newWindow(int x, int y, int width, int height, int fullWidth, int fullHeight)
{
	struct window *newWindow = (struct window*)xmalloc(sizeof(struct window));

	newWindow->fullWidth = fullWidth;
	newWindow->fullHeight = fullHeight;
	newWindow->xOrigin = x;
	newWindow->yOrigin = y;
	newWindow->xLength = width;
	newWindow->yLength = height;
	newWindow->xTerminus = x + width;
	newWindow->yTerminus = y + height;
	newWindow->xStep = 1;
	newWindow->yStep = fullWidth;
	newWindow->firstPixel = (newWindow->yOrigin * fullWidth) + newWindow->xOrigin;
	newWindow->lastPixel = (newWindow->yTerminus * fullWidth) + newWindow->xTerminus;
	newWindow->pixelCount = width * height;

	if (newWindow->xTerminus > newWindow->fullWidth) {
		printf("TODO: Handle this error -- xTerminus > fullWidth\n");
	}
	if (newWindow->yTerminus > newWindow->fullHeight) {
		printf("TODO: Handle this error -- yTerminus > fullHeight\n");
	}

	return newWindow;
}

void freeWindow(struct window *thisWindow)
{
	if (thisWindow) {
		free(thisWindow);
	}
}

#endif
