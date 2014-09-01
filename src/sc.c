/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#include <unistd.h>
#include <stdio.h>
#include "libpngHelper.c"
#include "libSeamCarve.c"

#define LOADDOCUMENT_VERBOSE 0

static void carve(char *filename)
{
	int *imageVector;
	int imageWidth = 0;
	int imageHeight = 0;
	imageVector = readPNGFile(filename, &imageWidth, &imageHeight, LOADDOCUMENT_VERBOSE);
	if (!imageVector || !imageWidth || !imageHeight) {
		printf("Error loading PNG image.\n");
		exit(1);
	}

	int *newImageVector;
	newImageVector = seamCarve(imageVector, imageWidth, imageHeight);

	char fName[16] = "./tst/out.png";
	write_png_file(newImageVector, imageWidth, imageHeight, fName);
}

int main(int argc, char const *argv[])
{
	if (argc > 1) {
		char *filename = (char*)argv[1];
		if (access(filename, R_OK) != -1) {
			carve(filename);
		} else {
			printf("Error reading file %s\n", argv[1]);
		}
	} else {
		printf("Usage:\n");
		printf(" %s filename\n", argv[0]);
	}
}
