#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <tr1/memory>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <stdarg.h>

#include "allsky.h"
#include "camera.h"
#include "camera_rpihq.h"
#include "allskyStatic.h"

using namespace std;

Camera* myCam;

//-------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	Allsky::init(argc, argv);
	assert(Allsky::status == Allsky::StatusInit);

	if (strcmp(Allsky::cameraName, "RPiHQ") == 0)
		myCam = new CameraRPi();
	// TODO: other cameras....

	Allsky::info();
	
	Allsky::status = Allsky::StatusLoop;
	while (Allsky::status == Allsky::StatusLoop)
	{
		Allsky::prepareForDayOrNight();

		// Wait for switch day time -> night time or night time -> day time
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
	
	Allsky::closeUp(0);
}
