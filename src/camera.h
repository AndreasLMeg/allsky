#pragma once

#include "allsky.h"
#include "modeMean.h"
#include <unistd.h>

// Base class
class Camera: public Allsky {
  public:

		Camera () 
		{
			Allsky::Debug("Camera::Camera\n");
		};
		Camera (int argc, char *argv[]);

		void init(int argc, char *argv[]);

		void setWaitForNextCapture(long waitForNextCapture_us) {m_waitForNextCapture_us = waitForNextCapture_us;};
		long getWaitForNextCapture(void) {return m_waitForNextCapture_us;};

		/* all camara depending things before using */
		virtual void initCamera(void) = 0;
		/* all camara depending things before the capture */
		//virtual void setupForCapture(void) = 0;
		/* Camera captures one image */
		//virtual int capture(void) = 0;
		/* all camara depending things after the capture */
		//virtual void postCapture(void) = 0;

		void waitForNextCapture(void)
		{
			//todo: add code
			Info("TODO !!!! - Sleeping %.1f seconds...\n", (float)m_waitForNextCapture_us / US_IN_SEC);
			usleep(m_waitForNextCapture_us);
		}

		void prepareForDayOrNight(void)
		{
			//todo: add code
			Info("TODO !!!!\n");

			// Find out if it is currently DAY or NIGHT
			calculateDayOrNight();
		}

		private:
			long m_waitForNextCapture_us = 1000000;
};
