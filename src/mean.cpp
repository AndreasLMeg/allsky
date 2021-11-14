#include "allsky.h"

#include "mean.h"

int Mean::computeMean(unsigned char *imageBuffer, int width, int height, ASI_IMG_TYPE imageType, int *histogram) 
{
    int h, i;
    unsigned char *buf = imageBuffer;

    // Clear the histogram array.
    for (h = 0; h < 256; h++) {
        histogram[h] = 0;
    }

    // Different image types have a different number of bytes per pixel.
    // todo use bytesPerPixel
    int bpp = bytesPerPixel;
    width *= bpp;
    int roiX1 = (width * histogramBoxPercentFromLeft) - (histogramBoxSizeX * bpp / 2);
    int roiX2 = roiX1 + (bpp * histogramBoxSizeX);
    int roiY1 = (height * histogramBoxPercentFromTop) - (histogramBoxSizeY / 2);
    int roiY2 = roiY1 + histogramBoxSizeY;

    // Start off and end on a logical pixel boundries.
    roiX1 = (roiX1 / bpp) * bpp;
    roiX2 = (roiX2 / bpp) * bpp;

    // For RGB24, data for each pixel is stored in 3 consecutive bytes: blue, green, red.
    // For all image types, each row in the image contains one row of pixels.
    // bpp doesn't apply to rows, just columns.
    switch (imageType) {
    case ASI_IMG_RGB24:
    case ASI_IMG_RAW8:
    case ASI_IMG_Y8:
        for (int y = roiY1; y < roiY2; y++) {
            for (int x = roiX1; x < roiX2; x+=bpp) {
                i = (width * y) + x;
                int total = 0;
                for (int z = 0; z < bpp; z++)
                {
                    // For RGB24 this averages the blue, green, and red pixels.
                    total += buf[i+z];
                }
                int avg = total / bpp;
                histogram[avg]++;
            }
        }
        break;
    case ASI_IMG_RAW16:
        for (int y = roiY1; y < roiY2; y++) {
            for (int x = roiX1; x < roiX2; x+=bpp) {
                i = (width * y) + x;
                int pixelValue;
                // This assumes the image data is laid out in big endian format.
                // We are going to grab the most significant byte
                // and use that for the histogram value ignoring the
                // least significant byte so we can use the 256 value histogram array.
                // If it's acutally little endian then add a +1 to the array subscript for buf[i].
                pixelValue = buf[i];
                histogram[pixelValue]++;
            }
        }
        break;
    case ASI_IMG_END:
        break;
    }

    // Now calculate the mean.
    int meanBin = 0;
    int a = 0, b = 0;
    for (int h = 0; h < 256; h++) {
        a += (h+1) * histogram[h];
        b += histogram[h];
    }

    if (b == 0)
    {
        // todo
        sprintf(Allsky::debug_text, "*** ERROR: calculating histogram: b==0\n");
        Allsky::displayDebugText(Allsky::debug_text, 0);
        return(0);
    }

    meanBin = a/b - 1;
    return meanBin;
};