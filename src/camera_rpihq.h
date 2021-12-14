#ifndef CAMERA_RPIHQ_H
#define CAMERA_RPIHQ_H

#include "camera.h"

// Derived class
class CameraRPi : public Camera {
  public:
		//virtual ~CameraRPi(){};
    //void help();
		void kill();
		void setup();
		void capture();
};

#endif