//
// RPiHQ_raspistill.h
//
// 2021-07-27  initial state
//
// https://www.raspberrypi.org/documentation/raspbian/applications/camera.md

#ifndef RPIHQ_RASPISTILL_H
#define RPIHQ_RASPISTILL_H

struct raspistillSetting {
 double analoggain = 1.0;       // Sets the analog gain value directly on the sensor
 double digitalgain = 1.0;      // Sets the digital gain value applied by the ISP (floating point value from 1.0 to 64.0, but values over about 4.0 will produce overexposed images)
 int shutter_us = 1*1000*1000;  // Sets the shutter open time to the specified value (in microseconds).
 int brightness = 50;           // Sets the brightness of the image. 50 is the default. 0 is black, 100 is white.
};

#endif
