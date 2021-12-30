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
#include "camera.h"
//#include "camera_rpihq.h"
#include "camera_zwo.h"
#include "allskyStatic.h"

//#define USE_HISTOGRAM                     // use the histogram code as a workaround to ZWO's bug

// Forward definitions
//char *getRetCode(ASI_ERROR_CODE);

/*
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
int Allsky::showTime = DEFAULT_SHOWTIME;
cv::Mat Allsky::pRgb;	// the image
char Allsky::bufTime[128] = { 0 };
int Allsky::iTextX = DEFAULT_ITEXTX;
int Allsky::iTextY = DEFAULT_ITEXTY;
int Allsky::currentBin = NOT_SET;
double Allsky::fontsize = DEFAULT_FONTSIZE;
int Allsky::linewidth = DEFAULT_LINEWIDTH;
int Allsky::linetype[3] = { cv::LINE_AA, 8, 4 };
int Allsky::linenumber = DEFAULT_LINENUMBER;
int Allsky::fontname[] = { cv::FONT_HERSHEY_SIMPLEX,        cv::FONT_HERSHEY_PLAIN,         cv::FONT_HERSHEY_DUPLEX,
					   cv::FONT_HERSHEY_COMPLEX,        cv::FONT_HERSHEY_TRIPLEX,       cv::FONT_HERSHEY_COMPLEX_SMALL,
					   cv::FONT_HERSHEY_SCRIPT_SIMPLEX, cv::FONT_HERSHEY_SCRIPT_COMPLEX };
char const *Allsky::fontnames[] = {		// Character representation of names for clarity:
		"SIMPLEX",                      "PLAIN",                       "DUPEX",
		"COMPLEX",                      "TRIPLEX",                     "COMPLEX_SMALL",
		"SCRIPT_SIMPLEX",               "SCRIPT_COMPLEX" };
int Allsky::fontnumber = DEFAULT_FONTNUMBER;
int Allsky::fontcolor[3] = { 255, 0, 0 };
int Allsky::smallFontcolor[3] = { 0, 0, 255 };
int Allsky::Image_type = DEFAULT_IMAGE_TYPE;
int Allsky::outlinefont = DEFAULT_OUTLINEFONT;
int Allsky::iTextLineHeight = DEFAULT_ITEXTLINEHEIGHT;
char const *Allsky::ImgText   = "";
char Allsky::bufTemp[128] = { 0 };
char Allsky::bufTemp2[50] = { 0 };
int Allsky::showTemp = 0;
int Allsky::showExposure = 0;
int Allsky::showGain = 0;
int Allsky::showBrightness = 0;
int Allsky::showMean = 0;
int Allsky::showFocus = 0;
int Allsky::currentExposure_us = NOT_SET;
int Allsky::currentBrightness = NOT_SET;
int Allsky::currentAutoGain = NOT_SET;
double Allsky::currentGain = NOT_SET;
double Allsky::lastGain = NOT_SET;
float Allsky::lastMean = -1;
int Allsky::currentAutoExposure= 0;
long Allsky::actualTemp = 0;	// actual sensor temp, per the camera
char const *Allsky::tempType       = "C";	// Celsius
char Allsky::debugText[500];		// buffer to hold debug messages
int Allsky::debugLevel = 0;
char const *Allsky::ImgExtraText = "";
int Allsky::extraFileAge = 0;   // 0 disables it
bool Allsky::tty = false;	// are we on a tty?
int Allsky::notificationImages = DEFAULT_NOTIFICATIONIMAGES;
char const *Allsky::timeFormat = DEFAULT_TIMEFORMAT;
bool Allsky::use_new_exposure_algorithm = true;
int Allsky::asiBandwidth = DEFAULT_ASIBANDWIDTH;
int Allsky::asiAutoBandwidth = 0;	// is Auto Bandwidth on or off?
int Allsky::asiAutoAWB    = DEFAULT_AUTOWHITEBALANCE;	// is Auto White Balance on or off?
int Allsky::asiWBR = DEFAULT_ASIWBR;
int Allsky::asiWBB = DEFAULT_ASIWBB;
ASI_CAMERA_INFO Allsky::ASICameraInfo;
long Allsky::asiTargetTemp = 0;
int Allsky::asiCoolerEnabled = 0;
int Allsky::aggression = DEFAULT_AGGRESSION; // ala PHD2.  Percent of change made, 1 - 100.
int Allsky::taking_dark_frames = 0;
int Allsky::preview = 0;
int Allsky::showDetails = 0;
const char *Allsky::locale = DEFAULT_LOCALE;
	// angle of the sun with the horizon
	// (0=sunset, -6=civil twilight, -12=nautical twilight, -18=astronomical twilight)
char const *Allsky::angle = DEFAULT_ANGLE;
char const *Allsky::latitude = DEFAULT_LATITUDE;
char const *Allsky::longitude = DEFAULT_LONGITUDE;
char const *Allsky::fileName = DEFAULT_FILENAME;
int Allsky::asiFlip = 0;
int Allsky::width = DEFAULT_WIDTH;		
int Allsky::originalWidth = DEFAULT_WIDTH;
int Allsky::height = DEFAULT_HEIGHT;	
int Allsky::originalHeight = DEFAULT_HEIGHT;
int Allsky::dayBin = DEFAULT_DAYBIN;
int Allsky::nightBin  = DEFAULT_NIGHTBIN;
int Allsky::asiGamma = DEFAULT_ASIGAMMA;
// Maximum number of auto-exposure frames to skip when starting the program.
// This helps eliminate overly bright or dark images before the auto-exposure algorith kicks in.
// At night, don't use too big a number otherwise it takes a long time to get the first frame.
int Allsky::day_skip_frames = DEFAULT_DAYSKIPFRAMES;
int Allsky::night_skip_frames = DEFAULT_NIGHTSKIPFRAMES;
int Allsky::current_skip_frames = NOT_SET;
int Allsky::asiDayBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiNightBrightness = DEFAULT_BRIGHTNESS;
int Allsky::gainTransitionTime = DEFAULT_GAIN_TRANSITION_TIME;
int Allsky::asiNightMaxGain = DEFAULT_ASINIGHTMAXGAIN;
int Allsky::asiNightAutoGain = DEFAULT_NIGHTAUTOGAIN;	// is Auto Gain on or off for nighttime?
int Allsky::asiNightGain = DEFAULT_ASINIGHTGAIN;
int Allsky::dayDelay_ms = DEFAULT_DAYDELAY;	// Delay in milliseconds.
int Allsky::nightDelay_ms = DEFAULT_NIGHTDELAY;	// Delay in milliseconds.
int Allsky::asi_night_max_autoexposure_ms = DEFAULT_ASINIGHTMAXAUTOEXPOSURE_MS;
long Allsky::current_max_autoexposure_us  = NOT_SET;
long Allsky::asi_day_exposure_us = DEFAULT_ASIDAYEXPOSURE;
int Allsky::asi_day_max_autoexposure_ms= DEFAULT_ASIDAYMAXAUTOEXPOSURE_MS;
int Allsky::asiDayAutoExposure = DEFAULT_DAYAUTOEXPOSURE;	// is it on or off for daylight?
long Allsky::asi_night_exposure_us = DEFAULT_ASINIGHTEXPOSURE;
int Allsky::asiNightAutoExposure = DEFAULT_NIGHTAUTOEXPOSURE;	// is it on or off for nighttime?
int Allsky::daytimeCapture = DEFAULT_DAYTIMECAPTURE;  // are we capturing daytime pictures?
int Allsky::quality = NOT_SET;
const char *Allsky::sType;		// displayed in output
bool Allsky::gotSignal = true;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
int Allsky::CamNum = 0;
bool Allsky::bDisplay = false;
pthread_t Allsky::thread_display = 0;
pthread_t Allsky::hthdSave = 0;
void *Allsky::retval;
bool Allsky::bSaveRun = false;
pthread_mutex_t Allsky::mtx_SaveImg;
pthread_cond_t Allsky::cond_SatrtSave;
std::string Allsky::dayOrNight;
*/

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
Camera* myCam;

int main(int argc, char *argv[])
{
	Allsky::tty = isatty(fileno(stdout)) ? true : false;
	signal(SIGINT, Allsky::IntHandle);
	signal(SIGTERM, Allsky::IntHandle);	// The service sends SIGTERM to end this program.
	pthread_mutex_init(&Allsky::mtx_SaveImg, 0);
	pthread_cond_init(&Allsky::cond_SatrtSave, 0);



	// Some settings have both day and night versions, some have only one version that applies to both,
	// and some have either a day OR night version but not both.
	// For settings with both versions we keep a "current" variable (e.g., "currentBin") that's either the day
	// or night version so the code doesn't always have to check if it's day or night.
	// The settings have either "day" or "night" in the name.
	// In theory, almost every setting could have both day and night versions (e.g., width & height),
	// but the chances of someone wanting different versions.

	// #define the defaults so we can use the same value in the help message.

#define SMALLFONTSIZE_MULTIPLIER 0.08

	
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
				Allsky::deliverImage();
			}
			else {
				// Check if we reached the maximum number of consective errors
				// bMain = check_max_errors(&exitCode, maxErrors);
				printf(" >>> Unable to take picture, return code=%d\n", (retCode >> 8));
				Allsky::Warning("  > Sleeping from failed exposure: %.1f seconds\n", (float)Allsky::currentDelay_ms / MS_IN_SEC);
				usleep(Allsky::currentDelay_ms * US_IN_MS); // TODO: move to waitForNextCapture
				continue; // TODO: ist das notwendig ?
			}
			Allsky::waitForNextCapture();
			
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
