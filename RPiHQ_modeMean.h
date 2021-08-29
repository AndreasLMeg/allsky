//
// RPiHQ_modeMean.h
//
// 2021-07-31  initial state - move some variables into structure
//

#ifndef RPIHQ_MODE_MEAN_H
#define RPIHQ_MODE_MEAN_H

struct modeMeanSetting {
    bool mode_mean       = false;   // activate mode mean
    double mean_value    = 0.5;     // mean value for well exposed images
    double mean_threshold = 0.02;   // threshold value
    double shuttersteps = 6.0;      // shuttersteps
    double fastforward = 4.0;       // magic number for fastforward  [0...10]
    int longplay = 0;               // make delay between captures 
    int brightnessControl = 0;      //
    int historySize = 3;            // use the last images for mean calculation
    int maskHorizon = 0;            //
    int info = 0;                   // show some debug infos
    int quickstart = 10;            // Sets the quickstart. Deactivate delay between captures for quickstart times.
    // some other values
    int ExposureLevel = 1;  // current ExposureLevel 
};

void RPiHQcalcMean(const char* fileName, int asiExposure, double asiGain, int asiBrightness, raspistillSetting &currentRaspistillSetting, modeMeanSetting &currentModeMeanSetting);
void RPiHQmask(const char* fileName);

#endif
