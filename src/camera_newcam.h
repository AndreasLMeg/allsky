#pragma once

#include "camera.h"

// Derived class
class CameraNewCam : public Camera {
  public:
		CameraNewCam () {};
		CameraNewCam (int argc, char *argv[]) 
		{
			init(argc, argv);
		};

		// overloaded
		void kill();
		void initCamera();
		void setupForCapture();
		int capture();
		void postCapture(void);
	private:
};
