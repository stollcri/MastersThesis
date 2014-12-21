/**
 * Masters Thesis Work: OCR
 * Christopher Stoll, 2014
 */

#ifndef LIBBINARIZATION_C
#define LIBBINARIZATION_C

static int otsuBinarization(int *histogram, int pixelCount)
{
	int sum = 0;
	for (int i = 1; i < 256; ++i) {
		sum += i * histogram[i];
	}
	
	int sumB = 0;
	int wB = 0;
	int wF = 0;
	int mB;
	int mF;
	double max = 0.0;
	double between = 0.0;
	double threshold1 = 0.0;
	double threshold2 = 0.0;

	for (int i = 0; i < 256; ++i) {
		wB += histogram[i];

		if (wB) {
			wF = pixelCount - wB;
			
			if (wF == 0) {
				break;
			}
			
			sumB += i * histogram[i];
			
			mB = sumB / wB;
			mF = (sum - sumB) / wF;
			between = wB * wF * pow(mB - mF, 2);
			
			if ( between >= max ) {
				threshold1 = i;
				if ( between > max ) {
					threshold2 = i;
				}
				max = between;            
			}
		}
	}
	return ( threshold1 + threshold2 ) / 2.0;
}

#endif
