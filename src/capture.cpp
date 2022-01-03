#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/viz.hpp>
#include "include/ASICamera2.h"
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <cstdio>
#include <tr1/memory>
#include <ctime>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <locale.h>
#include <stdarg.h>

#include "allsky.h"
#include "camera_zwo.h"
#include "allskyStatic.h"

#define USE_HISTOGRAM                     // use the histogram code as a workaround to ZWO's bug

Camera* myCam;

int main(int argc, char *argv[])
{
	/*
	Allsky::tty = isatty(fileno(stdout)) ? true : false;
	signal(SIGINT, Allsky::IntHandle);
	signal(SIGTERM, Allsky::IntHandle);	// The service sends SIGTERM to end this program.
	pthread_mutex_init(&Allsky::mtx_SaveImg, 0);
	pthread_cond_init(&Allsky::cond_SatrtSave, 0);
*/


	// Some settings have both day and night versions, some have only one version that applies to both,
	// and some have either a day OR night version but not both.
	// For settings with both versions we keep a "current" variable (e.g., "currentBin") that's either the day
	// or night version so the code doesn't always have to check if it's day or night.
	// The settings have either "day" or "night" in the name.
	// In theory, almost every setting could have both day and night versions (e.g., width & height),
	// but the chances of someone wanting different versions.

	// #define the defaults so we can use the same value in the help message.

//#define SMALLFONTSIZE_MULTIPLIER 0.08

	
	//-------------------------------------------------------------------------------------------------------
	Allsky::init(argc, argv);
	assert(Allsky::status == Allsky::StatusInit);

	//if (strcmp(Allsky::cameraName, "RPiHQ") == 0)
		//myCam = new CameraRPi();
	//else
		myCam = new CameraZWO();
	// TODO: other cameras....

	myCam->initCamera();

	Allsky::info();
	
	Allsky::status = Allsky::StatusLoop;
	while (Allsky::status == Allsky::StatusLoop)
	{
		Allsky::prepareForDayOrNight();

		while (Allsky::status == Allsky::StatusLoop && (Allsky::lastDayOrNight == Allsky::dayOrNight))
		{
			Allsky::preCapture();
			myCam->setupForCapture();
			int retCode = myCam->capture();
			if (retCode == 0) {
				myCam->postCapture();
				myCam->deliverImage();
			}
			else {
				// Check if we reached the maximum number of consective errors
				// bMain = check_max_errors(&exitCode, maxErrors);
				printf(" >>> Unable to take picture, return code=%d\n", (retCode >> 8));
				Allsky::Warning("  > Sleeping from failed exposure: %.1f seconds\n", (float)Allsky::currentDelay_ms / MS_IN_SEC);
				usleep(Allsky::currentDelay_ms * US_IN_MS); // TODO: move to waitForNextCapture
				continue; // TODO: ist das notwendig ?
			}
			myCam->waitForNextCapture();
			
			// Check for day or night based on location and angle
			Allsky::calculateDayOrNight(Allsky::latitude, Allsky::longitude, Allsky::angle);
			Allsky::Info("----------------------------\n");
		}

		Allsky::Info("============================\n");
		Allsky::Info("%s\n", Allsky::dayOrNight.c_str());
		Allsky::Info("============================\n");
	}

	Allsky::closeUp(Allsky::exitCode);
}
