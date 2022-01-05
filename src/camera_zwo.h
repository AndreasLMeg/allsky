#ifndef CAMERA_ZWO_H
#define CAMERA_ZWO_H

#include "camera.h"
#include "include/ASICamera2.h"

// Derived class
class CameraZWO : public Camera {
  public:
		CameraZWO () {};
		CameraZWO (int argc, char *argv[]) 
		{
			init(argc, argv);
		};

		void kill();
		void initCamera();
		void setupForCapture();
		int capture();
		void postCapture(void);
		static char * getRetCode(ASI_ERROR_CODE code);
		static int bytesPerPixel(ASI_IMG_TYPE imageType);
		#ifdef USE_HISTOGRAM
		static int computeHistogram(unsigned char *imageBuffer, int width, int height, ASI_IMG_TYPE imageType, int *histogram);
		#endif
		static ASI_ERROR_CODE setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto);
		static int determineGainChange(int dayGain, int nightGain);
		static bool resetGainTransitionVariables(int dayGain, int nightGain);
		static void * SaveImgThd(void *para);
		static double time_diff_us(int64 start, int64 end);
		static bool check_max_errors(int *e, int max_errors);
		static void writeTemperatureToFile(float val);
		static long roundTo(long n, int roundTo);
		static void* Display(void *params);

	private:
		ASI_ERROR_CODE takeOneExposure(int cameraId,
			long exposure_time_us,
			unsigned char *imageBuffer, long width, long height,  // where to put image and its size
			ASI_IMG_TYPE imageType,
			int *histogram,
			int *mean);
};

#endif