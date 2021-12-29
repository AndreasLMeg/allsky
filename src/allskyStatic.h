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
int Allsky::dayDelay_ms = 15;	// Delay in milliseconds.
//   - image-capture
cv::Mat Allsky::pRgb;	// the image
int Allsky::asiFlip = 0;
int Allsky::asiRotation = 0;
//   - image-destination
char const *Allsky::fileName = DEFAULT_FILENAME;
int Allsky::Image_type = DEFAULT_IMAGE_TYPE;
int Allsky::width = DEFAULT_WIDTH;		
int Allsky::height = DEFAULT_HEIGHT;	
int Allsky::quality = NOT_SET;
//   - camera
char const *Allsky::cameraName = "RPiHQ";
bool Allsky::is_libcamera;
int Allsky::asiNightBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiDayBrightness = DEFAULT_BRIGHTNESS;
int Allsky::asiNightAutoExposure = DEFAULT_NIGHTAUTOEXPOSURE;	// is it on or off for nighttime?
int Allsky::asiDayAutoExposure = DEFAULT_DAYAUTOEXPOSURE;	// is it on or off for daylight?
int Allsky::asiAutoAWB = DEFAULT_AUTOWHITEBALANCE;	// is Auto White Balance on or off?
int Allsky::asiNightAutoGain = DEFAULT_NIGHTAUTOGAIN;	// is Auto Gain on or off for nighttime?
float Allsky::saturation = 0;
double Allsky::asiWBR         = 2.5;
double Allsky::asiWBB         = 2;
double Allsky::asiNightGain   = 4.0;
double Allsky::asiDayGain     = 1.0;
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
int Allsky::background = 0;

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
modeMeanSetting Allsky::myModeMeanSetting;
raspistillSetting Allsky::myRaspistillSetting;
int Allsky::min_brightness;					// what user enters on command line
int Allsky::max_brightness;
int Allsky::default_brightness;
float Allsky::min_saturation;				// produces black and white
float Allsky::max_saturation;
float Allsky::default_saturation;
std::vector<int> Allsky::compression_params;
int Allsky::numExposures = 0;	// how many valid pictures have we taken so far?

// Todo:
int Allsky::originalHeight = DEFAULT_HEIGHT; //Todo: change to current an do not change settings
int Allsky::originalWidth = DEFAULT_WIDTH; //Todo: change to current an do not change settings
int Allsky::asiNightExposure_us= 60 * US_IN_SEC; // TODO: notwendig ?
int Allsky::asiDayAutoGain    = 0;
int Allsky::asiDayExposure_us = 32;

// undefined - should be empty, why not ?

#endif