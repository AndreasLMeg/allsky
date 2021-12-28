#ifndef CAMERA_H
#define CAMERA_H

// Base class
class Camera: public Allsky {
  public:
		virtual void setup(void) = 0;
		virtual int capture(void) = 0;
		virtual void postCapture(void) = 0;
};

#endif