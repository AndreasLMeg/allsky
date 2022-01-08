#pragma once

#include "camera.h"

// Derived class
class CameraNewCam : public Camera {
  public:
		CameraNewCam () {};
		CameraNewCam (int argc, char *argv[]) 
		{
			printf("CameraNewCam (int argc, char *argv[])\n");
			Camera::init(argc, argv);
		};

		void kill();
		void initCamera();
		void setupForCapture();
		int capture();
		void postCapture(void);
	private:
};
