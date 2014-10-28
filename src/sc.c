/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#include <unistd.h>
#include <stdio.h>
#include "libpngHelper.c"
#include "libSeamCarve.c"
#include "libResize.c"

#define PROGRAM_NAME "Seam Carving Tests"
#define PROGRAM_VERS "0.0"
#define PROGRAM_COPY "Copyright 2014, Chrisotpher Stoll"

static void carve(char *sourceFile, char *resultFile, int forceDirection, int verbose)
{
	int *imageVector;
	int imageWidth = 0;
	int imageHeight = 0;
	imageVector = readPNGFile(sourceFile, &imageWidth, &imageHeight, verbose);
	if (!imageVector || !imageWidth || !imageHeight) {
		fprintf(stderr, "Error loading PNG image.\n");
		exit(1);
	}

	int *newImageVector;
	newImageVector = seamCarve(imageVector, imageWidth, forceDirection, imageHeight);

	double imageScale = 0.25;
	int imagePadding = 4;
	int newWidth = getScaledSize(imageWidth, imageScale) + imagePadding + imagePadding;
	int newHeight = getScaledSize(imageHeight, imageScale) + imagePadding + imagePadding;
	write_png_file(newImageVector, newWidth, newHeight, resultFile);
	//write_png_file(newImageVector, imageWidth, imageHeight, resultFile);
}

int main(int argc, char const *argv[])
{
	char **argumentVector = (char**)argv;

	char *dvalue = 0;
	int forceDir = 0;
	int verboseFlag = 0;
	char *sourceFile = 0;
	char *resultFile = 0;

	int c;
	opterr = 0;
	/*
	 * Currently accepted arguments:
	 *  s -- takes a parameter, but does nothing with it (TODO: remove)
	 *  v -- verbose mode
	 *  source_file -- the PNG image file to open
	 *  result_file -- the PNG file to save results to
	 */
	while ((c = getopt (argc, argumentVector, "d:v")) != -1) {
		switch (c) {
			case 'd':
				dvalue = optarg;
				forceDir = (int)dvalue[0] - 48;
				break;
			case 'v':
				verboseFlag = 1;
				break;
			case '?':
				printf(PROGRAM_NAME " v" PROGRAM_VERS "\n");
				printf(PROGRAM_COPY "\n\n");
				printf("usage: sc [-d 1|2] [-v] source_PNG_file result_PNG_file\n");
				printf("          '-d 1' is to force horizontal direction seams\n");
				printf("          '-d 2' is to force vertical direction seams\n");
				return 1;
			default:
				fprintf(stderr, "Unexpected argument character code: %c (0x%04x)\n", (char)c, c);
		}
	}

	int index;
	// Look at unnamed arguments to get source and result file names
	for (index = optind; index < argc; index++) {
		if (!sourceFile) {
			sourceFile = (char*)argv[index];
		} else if (!resultFile) {
			resultFile = (char*)argv[index];
		} else {
			fprintf(stderr, "Argument ignored: %s\n", argv[index]);
		}
	}

	// Make sure we have source and result files
	if (!sourceFile) {
		fprintf(stderr, "Required argument missing: source_file\n");
		return 1;
	} else if (!resultFile) {
		fprintf(stderr, "Required argument missing: result_file\n");
		return 1;
	}

	// Go ahead if the source file exists
	if (access(sourceFile, R_OK) != -1) {
		carve(sourceFile, resultFile, forceDir, verboseFlag);
	} else {
		fprintf(stderr, "Error reading file %s\n", sourceFile);
		return 1;
	}

	return 0;
}
