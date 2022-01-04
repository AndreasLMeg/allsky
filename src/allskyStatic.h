#ifndef ALLSKY_STATIC_H
#define ALLSKY_STATIC_H

#include "allsky.h"

// runtime / program
Allsky::Allsky_runtime Allsky::runtime;

// settings (json) - don't change !
Allsky::Allsky_settings Allsky::settings;

// todo: use structs !
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
pthread_mutex_t Allsky::mtx_SaveImg;
pthread_cond_t Allsky::cond_SatrtSave;
int Allsky::asi_day_max_autoexposure_ms= DEFAULT_ASIDAYMAXAUTOEXPOSURE_MS;
int Allsky::current_skip_frames = NOT_SET;
int Allsky::day_skip_frames = DEFAULT_DAYSKIPFRAMES;
int Allsky::night_skip_frames = DEFAULT_NIGHTSKIPFRAMES;
int Allsky::asi_night_max_autoexposure_ms = DEFAULT_ASINIGHTMAXAUTOEXPOSURE_MS;
long Allsky::current_max_autoexposure_us  = NOT_SET;
long Allsky::asi_day_exposure_us = DEFAULT_ASIDAYEXPOSURE;
long Allsky::asi_night_exposure_us = DEFAULT_ASINIGHTEXPOSURE;
int Allsky::CamNum = 0;
int Allsky::asiNightMaxGain = DEFAULT_ASINIGHTMAXGAIN;
long Allsky::asiTargetTemp = 0;
bool Allsky::use_new_exposure_algorithm = true;
int Allsky::asiBandwidth = DEFAULT_ASIBANDWIDTH;
int Allsky::asiAutoBandwidth = 0;	// is Auto Bandwidth on or off?
int Allsky::asiCoolerEnabled = 0;
int Allsky::aggression = DEFAULT_AGGRESSION; // ala PHD2.  Percent of change made, 1 - 100.
const char *Allsky::sType;		// displayed in output
int Allsky::asiGamma = DEFAULT_ASIGAMMA;
ASI_CAMERA_INFO Allsky::ASICameraInfo;
int Allsky::gainTransitionTime = DEFAULT_GAIN_TRANSITION_TIME;
pthread_t Allsky::thread_display = 0;
pthread_t Allsky::hthdSave = 0;
bool Allsky::bSaveRun = false;
void *Allsky::retval;
bool Allsky::bDisplay = false;
#endif

//Log Static Todo: move to own file ?
Log::Level Log::m_LogLevel = Log::LevelError;

// Todo:
int Allsky::originalHeight = DEFAULT_HEIGHT; //Todo: change to current an do not change settings
int Allsky::originalWidth = DEFAULT_WIDTH; //Todo: change to current an do not change settings
int Allsky::asiNightExposure_us= 60 * US_IN_SEC; // TODO: notwendig ?
int Allsky::asiDayAutoGain    = 0;
int Allsky::asiDayExposure_us = 32;

// undefined - should be empty, why not ?

#endif