#ifndef ALLSKY_STATIC_H
#define ALLSKY_STATIC_H

#include "allsky.h"

// program
Allsky::Status Allsky::status = StatusUndefined;
bool Allsky::endOfNight = false;
bool Allsky::gotSignal = false;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
std::string Allsky::dayOrNight;
std::string Allsky::lastDayOrNight;
Log::Level Log::m_LogLevel = Log::LevelError;

// settings (json) - don't change !
int Allsky::daytimeCapture = DEFAULT_DAYTIMECAPTURE;  // are we capturing daytime pictures?
int Allsky::debugLevel = 0;
bool Allsky::tty = false;	// are we on a tty?
int Allsky::notificationImages = DEFAULT_NOTIFICATIONIMAGES;
char const *Allsky::angle = DEFAULT_ANGLE;
char const *Allsky::latitude = DEFAULT_LATITUDE;
char const *Allsky::longitude = DEFAULT_LONGITUDE;
int Allsky::taking_dark_frames = 0;
int Allsky::preview = 0;
int Allsky::nightDelay_ms = 10;	// Delay in milliseconds.
#ifdef CAM_RPIHQ
int Allsky::dayDelay_ms = 15;	// Delay in milliseconds.
#else
int Allsky::dayDelay_ms = DEFAULT_DAYDELAY;	// Delay in milliseconds.
#endif
//   - image-capture
cv::Mat Allsky::pRgb;	// the image
int Allsky::asiFlip = 0;
#ifdef CAM_RPIHQ
int Allsky::asiRotation = 0;
#endif
//   - image-destination
char const *Allsky::fileName = DEFAULT_FILENAME;
int Allsky::Image_type = DEFAULT_IMAGE_TYPE;
int Allsky::width = DEFAULT_WIDTH;		
int Allsky::height = DEFAULT_HEIGHT;	
int Allsky::quality = NOT_SET;
//   - camera
char const *Allsky::cameraName = "RPiHQ";
#ifdef CAM_RPIHQ
bool Allsky::is_libcamera;
#endif
int Allsky::asiNightBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiDayBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiNightAutoExposure = DEFAULT_NIGHTAUTOEXPOSURE;	// is it on or off for nighttime?
int Allsky::asiDayAutoExposure = DEFAULT_DAYAUTOEXPOSURE;	// is it on or off for daylight?
int Allsky::asiAutoAWB = DEFAULT_AUTOWHITEBALANCE;	// is Auto White Balance on or off?
int Allsky::asiNightAutoGain = DEFAULT_NIGHTAUTOGAIN;	// is Auto Gain on or off for nighttime?
#ifdef CAM_RPIHQ
float Allsky::saturation = 0;
double Allsky::asiWBR         = 2.5;
double Allsky::asiWBB         = 2;
double Allsky::asiNightGain   = 4.0;
double Allsky::asiDayGain     = 1.0;
#else
int Allsky::asiWBR = DEFAULT_ASIWBR;
int Allsky::asiWBB = DEFAULT_ASIWBB;
int Allsky::asiNightGain = DEFAULT_ASINIGHTGAIN;
#endif
int Allsky::dayBin = DEFAULT_DAYBIN;
int Allsky::nightBin  = DEFAULT_NIGHTBIN;
//   - annotate
const char *Allsky::locale = DEFAULT_LOCALE;
char const *Allsky::timeFormat = DEFAULT_TIMEFORMAT;
int Allsky::showDetails = 0;
int Allsky::showTime = DEFAULT_SHOWTIME;
int Allsky::showTemp = 0;
int Allsky::showExposure = 0;
int Allsky::showGain = 0;
int Allsky::showBrightness = 0;
int Allsky::showMean = 0;
int Allsky::showFocus = 0;
char const *Allsky::ImgText   = "";
char const *Allsky::tempType       = "C";	// Celsius
char const *Allsky::ImgExtraText = "";
int Allsky::extraFileAge = 0;   // 0 disables it
//   - fonts
int Allsky::iTextX = DEFAULT_ITEXTX;
int Allsky::iTextY = DEFAULT_ITEXTY;
double Allsky::fontsize = DEFAULT_FONTSIZE;
int Allsky::linewidth = DEFAULT_LINEWIDTH;
int Allsky::linenumber = DEFAULT_LINENUMBER;
int Allsky::fontnumber = DEFAULT_FONTNUMBER;
int Allsky::fontcolor[3] = { 255, 0, 0 };
int Allsky::smallFontcolor[3] = { 0, 0, 255 };
int Allsky::outlinefont = DEFAULT_OUTLINEFONT;
int Allsky::iTextLineHeight = DEFAULT_ITEXTLINEHEIGHT;
#ifdef CAM_RPIHQ
int Allsky::background = 0;
#endif

// current values
int Allsky::currentBin = NOT_SET;
int Allsky::currentExposure_us = NOT_SET;
int Allsky::currentBrightness = NOT_SET;
int Allsky::currentAutoGain = NOT_SET;
double Allsky::currentGain = NOT_SET;
int Allsky::currentAutoExposure= 0;
long Allsky::actualTemp = 0;	// actual sensor temp, per the camera
int Allsky::currentDelay_ms   = NOT_SET;

// last values
double Allsky::lastGain = NOT_SET;
float Allsky::lastMean = -1;

// helper
char Allsky::debugText[500];		// buffer to hold debug messages
char Allsky::bufTime[128] = { 0 };
char Allsky::bufTemp[128] = { 0 };
char Allsky::bufTemp2[50] = { 0 };
int Allsky::linetype[3] = { cv::LINE_AA, 8, 4 };
int Allsky::fontname[] = { cv::FONT_HERSHEY_SIMPLEX,        cv::FONT_HERSHEY_PLAIN,         cv::FONT_HERSHEY_DUPLEX,
						 cv::FONT_HERSHEY_COMPLEX,        cv::FONT_HERSHEY_TRIPLEX,       cv::FONT_HERSHEY_COMPLEX_SMALL,
						 cv::FONT_HERSHEY_SCRIPT_SIMPLEX, cv::FONT_HERSHEY_SCRIPT_COMPLEX };
char const *Allsky::fontnames[] = {		// Character representation of names for clarity:
		"SIMPLEX",                      "PLAIN",                       "DUPEX",
		"COMPLEX",                      "TRIPLEX",                     "COMPLEX_SMALL",
		"SCRIPT_SIMPLEX",               "SCRIPT_COMPLEX" };
std::vector<int> Allsky::compression_params;
int Allsky::numExposures = 0;	// how many valid pictures have we taken so far?
char Allsky::exposureStart[128];

//camera depending vars
#ifdef CAM_RPIHQ
modeMeanSetting Allsky::myModeMeanSetting;
raspistillSetting Allsky::myRaspistillSetting;
float Allsky::min_saturation;				// produces black and white
float Allsky::max_saturation;
float Allsky::default_saturation;
int Allsky::min_brightness;					// what user enters on command line
int Allsky::max_brightness;
int Allsky::default_brightness;
#else
ASI_ERROR_CODE Allsky::asiRetCode;  // used for return code from ASI functions.
long Allsky::bufferSize;
#ifdef USE_HISTOGRAM
// % from left/top side that the center of the box is.  0.5 == the center of the image's X/Y axis
float Allsky::histogramBoxPercentFromLeft = DEFAULT_BOX_FROM_LEFT;
float Allsky::histogramBoxPercentFromTop = DEFAULT_BOX_FROM_TOP;

int Allsky::showHistogram      = 0;
int Allsky::maxHistogramAttempts   = 15;	// max number of times we'll try for a better histogram mean
int Allsky::showHistogramBox       = 0;

	// If we just transitioned from night to day, it's possible current_exposure_us will
	// be MUCH greater than the daytime max (and will possibly be at the nighttime's max exposure).
	// So, decrease current_exposure_us by a certain amount of the difference between the two so
	// it takes several frames to reach the daytime max (which is now in current_max_autoexposure_us).

	// If we don't do this, we'll be stuck in a loop taking an exposure
	// that's over the max forever.

	// Note that it's likely getting lighter outside with every exposure
	// so the mean will eventually get into the valid range.
const int Allsky::percent_change = DEFAULT_PERCENTCHANGE;
#endif	// USE_HISTOGRAM
int Allsky::current_histogramBoxSizeX = NOT_SET;
int Allsky::current_histogramBoxSizeY = NOT_SET;
int Allsky::histogramBoxSizeX      = DEFAULT_BOX_SIZEX;
int Allsky::histogramBoxSizeY      = DEFAULT_BOX_SIZEY;
int Allsky::originalITextX;
int Allsky::originalITextY;
int Allsky::originalFontsize;
int Allsky::originalLinewidth;
long Allsky::current_exposure_us = NOT_SET;
int Allsky::gainChange = 0;			// how much to change gain up or down
long Allsky::camera_max_autoexposure_us= NOT_SET;	// camera's max auto-exposure
long Allsky::camera_min_exposure_us= 100;	// camera's minimum exposure
int Allsky::asiDayGain = DEFAULT_ASIDAYGHTGAIN;
int Allsky::numGainChanges = 0;		// This is reset at every day/night and night/day transition.
bool Allsky::adjustGain = false;	// Should we adjust the gain?  Set by user on command line.
bool Allsky::currentAdjustGain = false;	// Adjusting it right now?
int Allsky::totalAdjustGain = 0;	// The total amount to adjust gain.
int Allsky::perImageAdjustGain = 0;	// Amount of gain to adjust each image
int Allsky::gainTransitionImages = 0;
// Have we displayed "not taking picture during day" message, if applicable?
int Allsky::displayedNoDaytimeMsg = 0;
int Allsky::exitCode = 0;    // Exit code for main()
std::vector<int> Allsky::compression_parameters;
bool Allsky::bSavingImg;
ASI_CONTROL_CAPS Allsky::ControlCaps;
#endif

// Todo:
int Allsky::originalHeight = DEFAULT_HEIGHT; //Todo: change to current an do not change settings
int Allsky::originalWidth = DEFAULT_WIDTH; //Todo: change to current an do not change settings
int Allsky::asiNightExposure_us= 60 * US_IN_SEC; // TODO: notwendig ?
int Allsky::asiDayAutoGain    = 0;
int Allsky::asiDayExposure_us = 32;

// undefined - should be empty, why not ?

#endif