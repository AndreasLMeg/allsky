#ifndef CAMERA_H
#define CAMERA_H

#include "allsky.h"

// Base class
class Camera: public Allsky {
  public:

		Camera () {};
		Camera (int argc, char *argv[]) 
		{
			//Allsky(argc, argv);
		};

		/* all camara depending things before using */
		virtual void initCamera(void) = 0;
		/* all camara depending things before the capture */
		//virtual void setupForCapture(void) = 0;
		/* Camera captures one image */
		//virtual int capture(void) = 0;
		/* all camara depending things after the capture */
		//virtual void postCapture(void) = 0;
};

#endif