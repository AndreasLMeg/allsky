#ifndef CAMERA_RPIHQ_H
#define CAMERA_RPIHQ_H

#include "camera.h"

// Derived class
class CameraRPi : public Camera {
  public:
		CameraRPi () {};
		CameraRPi (int argc, char *argv[]) 
		{
			init(argc, argv);
		};

		void kill();
		void initCamera();
		void setupForCapture();
		int capture();
		void postCapture(void);
	private:
		int RPiHQcapture(int auto_exposure, int *exposure_us, int auto_gain, int auto_AWB, double gain, int bin, double WBR, double WBB, int rotation, int flip, float saturation, int currentBrightness, int quality, const char* fileName, int time, const char* ImgText, int fontsize, int *fontcolor, int background, int taking_dark_frames, int preview, int width, int height, bool libcamera);
};

#endif