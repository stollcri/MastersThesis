/**
 * sc3d.c
 *
 * Seam Carving in 3D
 * Christopher Stoll, 2015
 */

#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <teem/nrrd.h>
#include <teem/biff.h>

#include "libEnergies.c"
#include "libpngHelper.c"
#include "libSeamCarve.c"

#define PROGRAM_NAME "Experiments with Seam Carving in 3D"
#define PROGRAM_VERS "0.1"
#define PROGRAM_COPY "Copyright 2015, Christopher Stoll"

// void loopAll(Nrrd *nrrd) {
// 	double (*lup)(const void *, size_t I);
// 	// double (*ins)(void *, size_t I, double v);
// 	double val;
// 	size_t I, N;

// 	lup = nrrdDLookup[nrrd->type];
// 	// ins = nrrdDInsert[nrrd->type];
// 	N = nrrdElementNumber(nrrd);
// 	for (I=0; I<N; I++) {
// 		val = lup(nrrd->data, I);
// 		printf("%f, ", val);
// 		// ins(nrrd->data, I, val);
// 	}

// 	printf("\n");
// 	return;
// }

static void sc3d(char *sourceFile, char *resultFile, int verbose)
{
	Nrrd *nin = nrrdNew();
	if (nrrdLoad(nin, sourceFile, NULL)) {
		char *err = biffGetDone(NRRD);
		fprintf(stderr, "Trouble reading '%s':\n%s", sourceFile, err);
		free(err);
		return;
	}

	if (nin->dim < 3) {
		fprintf(stderr, "Not enough dimensions (dim = %d)", nin->dim);
		nrrdNuke(nin);
		return;
	}

	size_t* dimSizes[NRRD_DIM_MAX];
	nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, dimSizes);
	int imageWidth = (int)dimSizes[0];
	int imageHeight = (int)dimSizes[1];
	int imageDepth = 4;
	int pixelsPerSlice = imageWidth * imageHeight;

	int *sourceImage = (int*)malloc((unsigned long)pixelsPerSlice * (unsigned long)imageDepth * sizeof(int));
	int *sourceImageLast = (int*)malloc((unsigned long)pixelsPerSlice * (unsigned long)imageDepth * sizeof(int));
	int *sourceImageCurrent = (int*)malloc((unsigned long)pixelsPerSlice * (unsigned long)imageDepth * sizeof(int));
	int *newImageVector = NULL;

	char outfile[24];

	int startPixel = 0;
	int endPixel = 0;
	int val = 0;
	int valmin = INT_MAX;
	int valmax = 0;

	double (*lup)(const void *, size_t I);
	for(int slice = 0; slice < 93; ++slice) {
		startPixel = pixelsPerSlice * slice;
		endPixel = startPixel + pixelsPerSlice;
		lup = nrrdDLookup[nin->type];
		for(int i = startPixel; i < endPixel; ++i) {
			val = lup(nin->data, i);

			if (val > valmax) {
				valmax = val;
			}
			if (val < valmin) {
				valmin = val;
			}
		}
	}
	int scalefactor = (int)(pow(2, 16) / (valmax - valmin));

	for(int slice = 0; slice < 93; ++slice) {
		startPixel = pixelsPerSlice * slice;
		endPixel = startPixel + pixelsPerSlice;

		lup = nrrdDLookup[nin->type];
		int k = 0;
		for(int i = startPixel; i < endPixel; ++i) {
			val = lup(nin->data, i);
			sourceImageCurrent[(k*imageDepth)] = val * scalefactor;
			sourceImageCurrent[(k*imageDepth)+1] = val * scalefactor;
			sourceImageCurrent[(k*imageDepth)+2] = val * scalefactor;
			sourceImageCurrent[(k*imageDepth)+3] = INT_MAX;

			// sourceImage[(k*imageDepth)] = val * scalefactor;
			// sourceImage[(k*imageDepth)+1] = val * scalefactor;
			// sourceImage[(k*imageDepth)+2] = val * scalefactor;

			// sourceImageCurrent[(k*imageDepth)] = sourceImage[(k*imageDepth)] + sourceImageLast[(k*imageDepth)];
			// sourceImageCurrent[(k*imageDepth)+1] = sourceImage[(k*imageDepth)+1] + sourceImageLast[(k*imageDepth)+1];
			// sourceImageCurrent[(k*imageDepth)+2] = sourceImage[(k*imageDepth)+2] + sourceImageLast[(k*imageDepth)+2];
			// sourceImageCurrent[(k*imageDepth)+3] = INT_MAX;

			// sourceImageLast[(k*imageDepth)] = sourceImage[(k*imageDepth)];
			// sourceImageLast[(k*imageDepth)+1] = sourceImage[(k*imageDepth)+1];
			// sourceImageLast[(k*imageDepth)+2] = sourceImage[(k*imageDepth)+2];

			++k;
		}

		// newImageVector = seamCarve(sourceImageCurrent, imageWidth, imageHeight, imageDepth, 6, 0, 3, 7, 1);
		// TODO: 49 (and thus 51) are seg faulting, find the problem
		// newImageVector = seamCarve(sourceImageCurrent, imageWidth, imageHeight, imageDepth, 6, 0, 50, 7, 1);
		newImageVector = seamCarve(sourceImageCurrent, imageWidth, imageHeight, imageDepth, 0, 2, 53, 4, 0);

		// double (*ins)(const void *, size_t I, double v);
		// ins = nrrdDInsert[nin->type];
		// k = 0;
		// for(int i = startPixel; i < endPixel; ++i) {
		// 	val = sourceImageCurrent[(k*imageDepth)];
		// 	ins(nin->data, i, val);
		// 	++k;
		// }

		sprintf(outfile, "out/headsq/sc3d-%02d.png", (slice + 1));
		write_png_file(newImageVector, imageWidth, imageHeight, outfile);

		free(newImageVector);
		newImageVector = NULL;
	}

	printf(": \"%s\" is a %d-dimensional nrrd of type %d (%s)\n", sourceFile, nin->dim, nin->type, airEnumStr(nrrdType, nin->type));
	printf(": the array contains %d elements, each %d bytes in size\n", (int)nrrdElementNumber(nin), (int)nrrdElementSize(nin));
	printf(": valmin= %d / valmax= %d (%d)\n", valmin, valmax, scalefactor);

	// if (nrrdSave(resultFile, nin, NULL)) {
	// 	char *err = biffGetDone(NRRD);
	// 	fprintf(stderr, "Trouble writing \"%s\":\n%s", resultFile, err);
	// 	free(err);
	// }
	// write_png_file(newImageVector, imageWidth, imageHeight, resultFile);
	nrrdNuke(nin);
}

int main(int argc, char const *argv[])
{
	char **argumentVector = (char**)argv;

	int verboseFlag = 0;
	char *sourceFile = 0;
	char *resultFile = 0;

	int c;
	/*
	 * Currently accepted arguments:
	 *  v -- verbose mode
	 *  source_file -- the PNG image file to open
	 *  result_file -- the PNG file to save results to
	 */
	while ((c = getopt (argc, argumentVector, "v")) != -1) {
		switch (c) {
			case 'v':
				verboseFlag = 1;
				break;
			case '?':
				printf(PROGRAM_NAME " v" PROGRAM_VERS "\n");
				printf(PROGRAM_COPY "\n\n");
				printf("usage: sc3d [-v] source_file result_file \n");
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
		sc3d(sourceFile, resultFile, verboseFlag);
	} else {
		fprintf(stderr, "Error reading file %s\n", sourceFile);
		return 1;
	}

	return 0;
}
