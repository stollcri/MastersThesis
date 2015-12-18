/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBBINARIZATION_C
#define LIBBINARIZATION_C

static int otsuBinarization(int *histogram, int pixelCount, int pixelDepth)
{
	unsigned long long sum = 0;
	unsigned long long oldsum = 0;
	for(int i = 1; i < pixelDepth; ++i) {
		oldsum = sum;
		sum += i * histogram[i];
		if(oldsum > sum) {
			printf("ERROR: Integer Overflow (%llu + n => %llu)\n", oldsum, sum);
		}
	}

	unsigned long long sumB = 0;
	unsigned long long wB = 0;
	unsigned long long wF = 0;
	unsigned long long mB;
	unsigned long long mF;
	long double max = 0.0;
	long double between = 0.0;
	long double threshold1 = 0.0;
	long double threshold2 = 0.0;

	for(int i = 0; i < pixelDepth; ++i) {
		wB += histogram[i];

		if(wB) {
			wF = pixelCount - wB;

			if(wF == 0) {
				break;
			}

			sumB += i * histogram[i];

			mB = sumB / wB;
			mF = (sum - sumB) / wF;
			between = wB * wF * ((mB - mF) * (mB - mF));

			if(between >= max) {
				threshold1 = i;
				if(between > max) {
					threshold2 = i;
				}
				max = between;
			}
		}
	}
	return (int)((threshold1 + threshold2) / 2.0);
}

#endif
