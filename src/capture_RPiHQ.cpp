#include <sys/time.h>
//#include <time.h>
#include <unistd.h>
#include <string.h>
//#include <sys/types.h>
#include <errno.h>
#include <string>
#include <iomanip>
#include <cstring>
#include <sstream>
//#include <cstdio>
#include <tr1/memory>
//#include <ctime>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <stdarg.h>

#include "allsky.h"
#include "camera.h"
#include "camera_rpihq.h"

using namespace std;


//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//Allsky myAllsky;
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
modeMeanSetting Allsky::myModeMeanSetting;
raspistillSetting Allsky::myRaspistillSetting;
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
int Allsky::asiNightBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiDayBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiNightAutoExposure = DEFAULT_NIGHTAUTOEXPOSURE;	// is it on or off for nighttime?
int Allsky::asiDayAutoExposure = DEFAULT_DAYAUTOEXPOSURE;	// is it on or off for daylight?
int Allsky::asiAutoAWB = DEFAULT_AUTOWHITEBALANCE;	// is Auto White Balance on or off?
int Allsky::asiNightAutoGain = DEFAULT_NIGHTAUTOGAIN;	// is Auto Gain on or off for nighttime?
double Allsky::asiWBR         = 2.5;
double Allsky::asiWBB         = 2;
double Allsky::asiNightGain   = 4.0;
double Allsky::asiDayGain     = 1.0;
const char *Allsky::locale = DEFAULT_LOCALE;
int Allsky::width = DEFAULT_WIDTH;		
int Allsky::height = DEFAULT_HEIGHT;	
int Allsky::quality = NOT_SET;
int Allsky::daytimeCapture = DEFAULT_DAYTIMECAPTURE;  // are we capturing daytime pictures?
char const *Allsky::angle = DEFAULT_ANGLE;
char const *Allsky::latitude = DEFAULT_LATITUDE;
char const *Allsky::longitude = DEFAULT_LONGITUDE;
char const *Allsky::fileName = DEFAULT_FILENAME;
int Allsky::taking_dark_frames = 0;
int Allsky::preview = 0;
int Allsky::asiFlip = 0;
int Allsky::asiRotation = 0;
int Allsky::originalHeight = DEFAULT_HEIGHT;
int Allsky::originalWidth = DEFAULT_WIDTH;
int Allsky::background = 0;
float Allsky::saturation = 0;
char const *Allsky::timeFormat = DEFAULT_TIMEFORMAT;
int Allsky::nightDelay_ms = 10;	// Delay in milliseconds.
int Allsky::dayDelay_ms = 15;	// Delay in milliseconds.
int Allsky::dayBin = DEFAULT_DAYBIN;
int Allsky::nightBin  = DEFAULT_NIGHTBIN;
int Allsky::showDetails = 0;
int Allsky::gotSignal = 0;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
std::string Allsky::dayOrNight;
int Allsky::min_brightness;					// what user enters on command line
int Allsky::max_brightness;
int Allsky::default_brightness;
float Allsky::min_saturation;				// produces black and white
float Allsky::max_saturation;
float Allsky::default_saturation;
bool Allsky::is_libcamera;
std::vector<int> Allsky::compression_params;
bool Allsky::endOfNight = false;
Log::Level Log::m_LogLevel = Log::LevelError;
int Allsky::asiNightExposure_us= 60 * US_IN_SEC; // TODO: notwendig ?
int Allsky::numExposures = 0;	// how many valid pictures have we taken so far?
int Allsky::asiDayAutoGain    = 0;
int Allsky::asiDayExposure_us = 32;
int Allsky::currentDelay_ms   = NOT_SET;

bool bMain					= true;

CameraRPi myCam;

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	//-------------------------------------------------------------------------------------------------------
	Allsky::init(argc, argv);
	//-------------------------------------------------------------------------------------------------------

	if (Allsky::taking_dark_frames)
	{
		// To avoid overwriting the optional notification inage with the dark image,
		// during dark frames we use a different file name.
		Allsky::fileName = "dark.jpg";
	}

	// Handle "auto" image_type.
	if (Allsky::Image_type == AUTO_IMAGE_TYPE)
	{
		// user will have to manually set for 8- or 16-bit mono mode
		Allsky::Image_type = ASI_IMG_RGB24;
	}

	//-------------------------------------------------------------------------------------------------------
	Allsky::info();
	//-------------------------------------------------------------------------------------------------------
	
	while (bMain)
	{
		std::string lastDayOrNight;

		Allsky::prepareForDayOrNight();

		// Next line is present for testing purposes
		// dayOrNight.assign("NIGHT");
		lastDayOrNight = Allsky::dayOrNight;

		// Wait for switch day time -> night time or night time -> day time
		while (bMain && (lastDayOrNight == Allsky::dayOrNight))
		{
			// date/time is added to many log entries to make it easier to associate them
			// with an image (which has the date/time in the filename).
			timeval t;
			t = Allsky::getTimeval();
			char exposureStart[128];
			char f[10] = "%F %T";
			snprintf(exposureStart, sizeof(exposureStart), "%s", Allsky::formatTime(t, f));
			Allsky::Info("STARTING EXPOSURE at: %s   @ %s\n", exposureStart, Allsky::length_in_units(Allsky::currentExposure_us, true));

			// Get start time for overlay.  Make sure it has the same time as exposureStart.
			if (Allsky::showTime == 1)
				sprintf(Allsky::bufTime, "%s", Allsky::formatTime(t, Allsky::timeFormat));

			myCam.setup();
			// Capture and save image
			int retCode = Allsky::capture();
			if (retCode == 0)
			{
				Allsky::postCapture();
			}
			else
			{
				printf(" >>> Unable to take picture, return code=%d\n", (retCode >> 8));
				Allsky::Warning("  > Sleeping from failed exposure: %.1f seconds\n", (float)Allsky::currentDelay_ms / MS_IN_SEC);
				usleep(Allsky::currentDelay_ms * US_IN_MS);
				continue;
			}

			Allsky::deliverImage();
			Allsky::waitForNextCapture();

			// Check for day or night based on location and angle
			Allsky::calculateDayOrNight(Allsky::latitude, Allsky::longitude, Allsky::angle);
		}

		// Check for night situation
		if (lastDayOrNight == "NIGHT")
		{
			// Flag end of night processing is needed
			Allsky::endOfNight = true;
		}
	}

	Allsky::closeUp(0);
}
