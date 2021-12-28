#ifndef CAMERA_H
#define CAMERA_H

// Base class
class Camera: public Allsky {
  public:
		/* all camara depending things before the capture */
		virtual void setupCapture(void) = 0;
		/* Camera captures one image */
		virtual int capture(void) = 0;
		/* all camara depending things after the capture */
		virtual void postCapture(void) = 0;
};

#endif