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

static void carve(char *sourceFile, char *resultFile, int forceBinarization, int forceDirection, int forceEdge, int forceGauss, int verbose)
{
	int *imageVector;
	int imageWidth = 0;
	int imageHeight = 0;
	int imageDepth = 0;
	imageVector = readPNGFile(sourceFile, &imageWidth, &imageHeight, &imageDepth, verbose);
	if (!imageVector || !imageWidth || !imageHeight) {
		fprintf(stderr, "Error loading PNG image.\n");
		exit(1);
	}

	int *newImageVector;
	if (!forceEdge) {
		forceEdge = 1;
	}
	newImageVector = seamCarve(imageVector, imageWidth, imageHeight, imageDepth, forceBinarization, forceDirection, forceEdge, forceGauss);

	/*
	double imageScale = 1;//0.125;
	int imagePadding = 4;
	int newWidth = getScaledSize(imageWidth, imageScale) + imagePadding + imagePadding;
	int newHeight = getScaledSize(imageHeight, imageScale) + imagePadding + imagePadding;
	write_png_file(newImageVector, newWidth, newHeight, resultFile);
	*/
	write_png_file(newImageVector, imageWidth, imageHeight, resultFile);
}

int main(int argc, char const *argv[])
{
	char **argumentVector = (char**)argv;

	char *bvalue = 0;
	char *dvalue = 0;
	char *evalue = 0;
	char *gvalue = 0;
	int forceBin = 0;
	int forceDir = 0;
	int forceEdge = 0;
	int forceGauss = 0;
	int verboseFlag = 0;
	char *sourceFile = 0;
	char *resultFile = 0;

	int c;
	opterr = 0;
	/*
	 * Currently accepted arguments:
	 *  d -- the direction to seam carve
	 *  e -- the edge detection method
	 *  v -- verbose mode
	 *  source_file -- the PNG image file to open
	 *  result_file -- the PNG file to save results to
	 */
	while ((c = getopt (argc, argumentVector, "b:d:e:g:v")) != -1) {
		switch (c) {
			case 'b':
				bvalue = optarg;
				forceBin = (int)bvalue[0] - 48;
				break;
			case 'd':
				dvalue = optarg;
				forceDir = (int)dvalue[0] - 48;
				break;
			case 'e':
				evalue = optarg;
				forceEdge = (int)evalue[0] - 48;
				break;
			case 'g':
				gvalue = optarg;
				forceGauss = (int)gvalue[0] - 48;
				break;
			case 'v':
				verboseFlag = 1;
				break;
			case '?':
				printf(PROGRAM_NAME " v" PROGRAM_VERS "\n");
				printf(PROGRAM_COPY "\n\n");
				printf("usage: sc [-b 1] [-d 1-3] [-e 1-7] [-g 1-3] [-v] source_PNG_file result_PNG_file\n");
				
				printf("          '-b 1' use Otsu binarization (before any Gaussian blurring) \n");

				printf("          '-d 1' force horizontal direction seams \n");
				printf("          '-d 2' force vertical direction seams \n");
				printf("          '-d 3' force both direction seams \n");
				printf("          '-d 4' output brightness values \n");
				printf("          '-d 5' output energy values \n");
				printf("          '-d 6' output seam values \n");
				
				printf("          '-e 1' use Difference of Gaussian (default) \n");
				printf("          '-e 2' use Laplacian of Gaussian (sigma=8)\n");
				printf("          '-e 3' use Laplacian of Gaussian (sigma=4)\n");
				printf("          '-e 4' use Laplacian of Gaussian (sigma=2)\n");
				printf("          '-e 5' use Sobel \n");
				printf("          '-e 6' use LoG Simple\n");
				printf("          '-e 7' use Simple Gradient \n");

				printf("          '-g 1' pre-Gaussian blur (sigma=2) \n");
				printf("          '-g 2' pre-Gaussian blur (sigma=4) \n");
				printf("          '-g 3' pre-Gaussian blur (sigma=8) \n");
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
		carve(sourceFile, resultFile, forceBin, forceDir, forceEdge, forceGauss, verboseFlag);
	} else {
		fprintf(stderr, "Error reading file %s\n", sourceFile);
		return 1;
	}

	return 0;
}
