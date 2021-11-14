#ifndef MEAN_H
#define MEAN_H

#include <string.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/viz.hpp>
#include "include/ASICamera2.h"
#include <unistd.h>


// Base class
class Mean {
  public:
    int bytesPerPixel; // set before using this class !!
    float histogramBoxPercentFromLeft = 0.5;
    float histogramBoxPercentFromTop = 0.5;
    int histogramBoxSizeX = 500;     // 500 px x 500 px box.  Must be a multiple of 2.
    int histogramBoxSizeY = 500;

    int computeMean(unsigned char *imageBuffer, int width, int height, ASI_IMG_TYPE imageType, int *histogram);
 };

// Derived class
class MeanAndreasLMeg : public Mean {
  public:
};

// Derived class
class MeanZWO : public Mean {
  public:
};


#endif