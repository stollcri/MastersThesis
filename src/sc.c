/**
 * sc.c
 *
 * Masters Thesis Work
 * Christopher Stoll, 2014
 */

#include <unistd.h>
#include <stdio.h>
#include "libpngHelper.c"
#include "libSeamCarve.c"
#include "libResize.c"

#define PROGRAM_NAME "Experiments with Seam Carving"
#define PROGRAM_VERS "0.4"
#define PROGRAM_COPY "Copyright 201-2015, Christopher Stoll"

static void carve(char *sourceFile, char *resultFile, int forceBrt, int forceClr, int forceDir, int forceEdge, int forceGauss, int skipEdge, int verbose)
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
	newImageVector = seamCarve(imageVector, imageWidth, imageHeight, imageDepth, forceBrt, forceClr, forceDir, forceEdge, forceGauss, skipEdge);

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
	char *cvalue = 0;
	char *dvalue = 0;
	char *evalue = 0;
	char *gvalue = 0;
	int forceBrt = 0;
	int forceClr = 0;
	int forceDir = 0;
	int forceEdge = 0;
	int forceGauss = 0;
	int skipEdge = 0;
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
	while ((c = getopt (argc, argumentVector, "b:c:d:e:g:sv")) != -1) {
		switch (c) {
			case 'b':
				bvalue = optarg;
				forceBrt = (int)bvalue[0] - 48;
				break;
			case 'c':
				cvalue = optarg;
				forceClr = (int)cvalue[0] - 48;
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
			case 's':
				skipEdge = 1;
				break;
			case 'v':
				verboseFlag = 1;
				break;
			case '?':
				printf(PROGRAM_NAME " v" PROGRAM_VERS "\n");
				printf(PROGRAM_COPY "\n\n");
				printf("usage: sc [-b 0-8] [-c 0-5] [-d 0-9] [-e 0-8] [-g 0-3] [-s] [-v] source_PNG_file result_PNG_file \n");
				printf("       \n");
				printf("       Brightness Calculation Method \n");
				printf("         '-b 0' Average Intensity / Brightness (default) \n");
				printf("         '-b 1' HSV hexcone (Max Channel) \n");
				printf("         '-b 2' Luma luminance - sRGB / BT.709 \n");
				printf("         '-b 3' Luma luminance - NTSC / BT.601 \n");
				printf("         '-b 4' Relative luminance \n");
				printf("         '-b 5' HSP? \n");
				printf("         '-b 6' Euclidian distance (generally poor results) \n");
				printf("         '-b 7' Estimated relative luminance \n");
				printf("         '-b 8' Estimated luma luminance - NTSC / BT.601 \n");
				printf("         '-b 9' Euclidean dist. of Lab color over Gaussian dist. \n");
				printf("          \n");
				printf("       Contrast Adjustments \n");
				printf("         '-c 0' none (default) \n");
				printf("         '-c 1' use cosine adjusted brightness \n");
				printf("         '-c 2' use double-pass cosine adjusted brightness \n");
				printf("         '-c 3' use triple-pass cosine adjusted brightness \n");
				printf("         '-c 4' use quadruple-pass cosine adjusted brightness \n");
				printf("         '-c 5' use Otsu binarization (before any Gaussian blurring) \n");
				printf("          \n");
				printf("       Force Seam Direction (or other output) \n");
				printf("         '-d 0' automatically selected (default) \n");
				printf("         '-d 1' force horizontal direction seams \n");
				printf("         '-d 2' force vertical direction seams \n");
				printf("         '-d 3' force both direction seams \n");
				printf("         '-d 4' output brightness values \n");
				printf("         '-d 5' output energy values \n");
				printf("         '-d 6' output seam values (horizontal) \n");
				printf("         '-d 7' output seam values (vertical) \n");
				printf("         '-d 8' output seams (horizontal) \n");
				printf("         '-d 9' output seams (vertical) \n");
				printf("         '-d a' output areas (horizontal) \n");
				printf("         '-d b' output areas (vertical) \n");
				printf("         '-d c' output areas (both) \n");
				printf("         '-d d' \n");
				printf("         '-d e' 6 and 7 combined \n");
				printf("         '-d f' 8 and 9 combined \n");
				printf("         '-d g' 8 and 9 combined (mask utput) \n");
				printf("          \n");
				printf("       Energy Calculation Method \n");
				printf("         '-e 0' use Difference of Gaussian (default) \n");
				printf("         '-e 1' use Laplacian of Gaussian (sigma=8) \n");
				printf("         '-e 2' use Laplacian of Gaussian (sigma=4) \n");
				printf("         '-e 3' use Laplacian of Gaussian (sigma=2) \n");
				printf("         '-e 4' use Sobel \n");
				printf("         '-e 5' use LoG Simple \n");
				printf("         '-e 6' use Simple Gradient \n");
				printf("         '-e 7' use DoG + Sobel \n");
				printf("         '-e 8' use LoG (sigma=8) AND Sobel \n");
				printf("         '-e 9' experimental (Stoll) \n");
				printf("          \n");
				printf("       Pre-processing Options \n");
				printf("         '-g 0' none (default) \n");
				printf("         '-g 1' pre-Gaussian blur (sigma=2) \n");
				printf("         '-g 2' pre-Gaussian blur (sigma=4) \n");
				printf("         '-g 3' pre-Gaussian blur (sigma=8) \n");
				printf("          \n");
				printf("        Other Options\n");
				printf("         '-s'   Skip seams that touch the edge (do not back track them)\n");
				printf("         '-v'   Verbose mode\n");
				printf("          \n");
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
		carve(sourceFile, resultFile, forceBrt, forceClr, forceDir, forceEdge, forceGauss, skipEdge, verboseFlag);
	} else {
		fprintf(stderr, "Error reading file %s\n", sourceFile);
		return 1;
	}

	return 0;
}
