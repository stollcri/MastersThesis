/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#include <unistd.h>
#include <stdio.h>
#include "libpngHelper.c"
#include "libSeamCarve.c"

#define PROGRAM_NAME "Seam Carving Tests"
#define PROGRAM_VERS "0.0"
#define PROGRAM_COPY "Copyright 2014, Chrisotpher Stoll"

static void carve(char *sourceFile, char *resultFile, int verbose)
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
	newImageVector = seamCarve(imageVector, imageWidth, imageHeight);

	write_png_file(newImageVector, imageWidth, imageHeight, resultFile);
}

int main(int argc, char const *argv[])
{
	char **argumentVector = (char**)argv;

	char *svalue = 0;
	int verboseFlag = 0;
	char *sourceFile = 0;
	char *resultFile = 0;

	int c;
	opterr = 0;
	while ((c = getopt (argc, argumentVector, "s:v")) != -1) {
		switch (c) {
			case 's':
				svalue = optarg;
				break;
			case 'v':
				verboseFlag = 1;
				break;
			case '?':
				printf(PROGRAM_NAME " v" PROGRAM_VERS "\n");
				printf(PROGRAM_COPY "\n\n");
				printf("usage: sc [-v] source_file result_file\n");
				return 1;
			default:
				fprintf(stderr, "Unexpected argument character code: %c (0x%04x)\n", (char)c, c);
		}
	}

	int index;
	for (index = optind; index < argc; index++) {
		if (!sourceFile) {
			sourceFile = (char*)argv[index];
		} else if (!resultFile) {
			resultFile = (char*)argv[index];
		} else {
			fprintf(stderr, "Argument ignored: %s\n", argv[index]);
		}
	}

	if (!sourceFile) {
		fprintf(stderr, "Required argument missing: source_file\n");
		return 1;
	} else if (!resultFile) {
		fprintf(stderr, "Required argument missing: result_file\n");
		return 1;
	}

	if (access(sourceFile, R_OK) != -1) {
		carve(sourceFile, resultFile, verboseFlag);
	} else {
		fprintf(stderr, "Error reading file %s\n", sourceFile);
		return 1;
	}

	return 0;
}
