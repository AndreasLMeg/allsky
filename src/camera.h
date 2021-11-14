#ifndef CAMERA_H
#define CAMERA_H

#include <string.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/viz.hpp>
#include "include/ASICamera2.h"
#include <unistd.h>

#define US_IN_MS 1000                     // microseconds in a millisecond
#define MS_IN_SEC 1000                    // milliseconds in a second
#define US_IN_SEC (US_IN_MS * MS_IN_SEC)  // microseconds in a second

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define NOT_SET -1	// signifies something isn't set yet

// Base class
class Camera {
  public:
      int fontname[8] = {
        cv::FONT_HERSHEY_SIMPLEX,        cv::FONT_HERSHEY_PLAIN,         cv::FONT_HERSHEY_DUPLEX,
        cv::FONT_HERSHEY_COMPLEX,        cv::FONT_HERSHEY_TRIPLEX,       cv::FONT_HERSHEY_COMPLEX_SMALL,
        cv::FONT_HERSHEY_SCRIPT_SIMPLEX, cv::FONT_HERSHEY_SCRIPT_COMPLEX };
    char const *fontnames[8] = {		// Character representation of names for clarity:
        "SIMPLEX",                      "PLAIN",                       "DUPEX",
        "COMPLEX",                      "TRIPLEX",                     "COMPLEX_SMALL",
        "SCRIPT_SIMPLEX",               "SCRIPT_COMPLEX" };

    cv::Mat pRgb;
    std::vector<int> compression_parameters;
    long actual_exposure_us = 0;	// actual exposure taken, per the camera
    long actualGain = 0;			// actual gain used, per the camera
ASI_BOOL bAuto = ASI_FALSE;		// "auto" flag returned by ASIGetControlValue, when we don't care what it is

ASI_BOOL wasAutoExposure = ASI_FALSE;
long bufferSize = NOT_SET;
bool adjustGain = false;	// Should we adjust the gain?  Set by user on command line.
bool currentAdjustGain = false;	// Adjusting it right now?
int totalAdjustGain = 0;	// The total amount to adjust gain.
int perImageAdjustGain = 0;	// Amount of gain to adjust each image
int gainTransitionImages = 0;
int numGainChanges = 0;		// This is reset at every day/night and night/day transition.



    bool argumentsQuoted = true;

  bool use_new_exposure_algorithm = true;

bool bMain = true;
static bool bDisplay;
std::string dayOrNight;

bool bSaveRun = false, bSavingImg = false;
pthread_mutex_t mtx_SaveImg;
pthread_cond_t cond_SatrtSave;

// These are global so they can be used by other routines.
#define NOT_SET -1	// signifies something isn't set yet
ASI_CONTROL_CAPS ControlCaps;
void *retval;
int numErrors            = 0;	// Number of errors in a row.
int gotSignal            = 0;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
int iNumOfCtrl           = 0;
int CamNum               = 0;
pthread_t thread_display = 0;
pthread_t hthdSave       = 0;
int numExposures         = 0;	// how many valid pictures have we taken so far?
int currentGain          = NOT_SET;
int min_exposure_us      = 100;
long current_exposure_us = NOT_SET;
long actualTemp          = 0;	// actual sensor temp, per the camera
int taking_dark_frames   = 0;

// Some command-line and other option definitions needed outside of main():
int tty = 0;	// 1 if we're on a tty (i.e., called from the shell prompt).
#define DEFAULT_NOTIFICATIONIMAGES 1
int notificationImages     = DEFAULT_NOTIFICATIONIMAGES;
#define DEFAULT_FILENAME     "image.jpg"
char const *fileName       = DEFAULT_FILENAME;
#define DEFAULT_TIMEFORMAT   "%Y%m%d %H:%M:%S"	// format the time should be displayed in
char const *timeFormat     = DEFAULT_TIMEFORMAT;

#define DEFAULT_ASIDAYEXPOSURE   500	// microseconds - good starting point for daytime exposures
long asi_day_exposure_us   = DEFAULT_ASIDAYEXPOSURE;
#define DEFAULT_ASIDAYMAXEXPOSURE_MS  (15 * MS_IN_SEC)	// 15 seconds
int asi_day_max_exposure_ms= DEFAULT_ASIDAYMAXEXPOSURE_MS;
#define DEFAULT_DAYAUTOEXPOSURE  1
int asiDayAutoExposure     = DEFAULT_DAYAUTOEXPOSURE;	// is it on or off for daylight?
#define DEFAULT_DAYDELAY     (5 * MS_IN_SEC)	// 5 seconds
int dayDelay               = DEFAULT_DAYDELAY;	// Delay in milliseconds.
#define DEFAULT_NIGHTDELAY   (10 * MS_IN_SEC)	// 10 seconds
int nightDelay             = DEFAULT_NIGHTDELAY;	// Delay in milliseconds.
#define DEFAULT_ASINIGHTMAXEXPOSURE_MS  (20 * MS_IN_SEC)	// 20 seconds
int asi_night_max_exposure_ms = DEFAULT_ASINIGHTMAXEXPOSURE_MS;
long current_max_exposure_us  = NOT_SET;

#define DEFAULT_GAIN_TRANSITION_TIME 5		// user specifies minutes
int gainTransitionTime     = DEFAULT_GAIN_TRANSITION_TIME;
ASI_BOOL currentAutoExposure = ASI_FALSE;	// is Auto Exposure currently on or off?

long camera_max_auto_exposure_us  = NOT_SET;	// camera's max auto exposure in us
#ifdef USE_HISTOGRAM
#define DEFAULT_BOX_SIZEX       500
#define DEFAULT_BOX_SIZEY       500
int histogramBoxSizeX         = DEFAULT_BOX_SIZEX;     // 500 px x 500 px box.  Must be a multiple of 2.
int histogramBoxSizeY         = DEFAULT_BOX_SIZEY;
#define DEFAULT_BOX_FROM_LEFT   0.5
#define DEFAULT_BOX_FROM_TOP    0.5
// % from left/top side that the center of the box is.  0.5 == the center of the image's X/Y axis
float histogramBoxPercentFromLeft = DEFAULT_BOX_FROM_LEFT;
float histogramBoxPercentFromTop = DEFAULT_BOX_FROM_TOP;
#endif	// USE_HISTOGRAM

//char debug_text[500];		// buffer to hold debug messages displayed by displayDebugText()
//char debug_text2[100];		// buffer to hold additional message
//int debugLevel = 0;

// main
    bool endOfNight       = false;
    ASI_ERROR_CODE asiRetCode;  // used for return code from ASI functions.

    // Some settings have both day and night versions, some have only one version that applies to both,
    // and some have either a day OR night version but not both.
    // For settings with both versions we keep a "current" variable (e.g., "currentBin") that's either the day
    // or night version so the code doesn't always have to check if it's day or night.
    // The settings have either "day" or "night" in the name.
    // In theory, almost every setting could have both day and night versions (e.g., width & height),
    // but the chances of someone wanting different versions.

    // #define the defaults so we can use the same value in the help message.

#define DEFAULT_LOCALE           "en_US.UTF-8"
const char *locale = DEFAULT_LOCALE;
    // All the font settings apply to both day and night.
#define DEFAULT_FONTNUMBER       0
    int fontnumber             = DEFAULT_FONTNUMBER;
#define DEFAULT_ITEXTX           15
#define DEFAULT_ITEXTY           25
    int iTextX                 = DEFAULT_ITEXTX;
    int iTextY                 = DEFAULT_ITEXTY;
#define DEFAULT_ITEXTLINEHEIGHT  30
    int iTextLineHeight        = DEFAULT_ITEXTLINEHEIGHT;
    char const *ImgText        = "";
    char const *ImgExtraText   = "";
    int extraFileAge           = 0;   // 0 disables it
#define DEFAULT_FONTSIZE         7
    double fontsize            = DEFAULT_FONTSIZE;
#define SMALLFONTSIZE_MULTIPLIER 0.08
#define DEFAULT_LINEWIDTH        1
    int linewidth              = DEFAULT_LINEWIDTH;
#define DEFAULT_OUTLINEFONT      0
    int outlinefont            = DEFAULT_OUTLINEFONT;
    int fontcolor[3]           = { 255, 0, 0 };
    int smallFontcolor[3]      = { 0, 0, 255 };
    int linetype[3]            = { cv::LINE_AA, 8, 4 };
#define DEFAULT_LINENUMBER       0
    int linenumber             = DEFAULT_LINENUMBER;

#define DEFAULT_WIDTH            0
#define DEFAULT_HEIGHT           0
    int width                  = DEFAULT_WIDTH;		int originalWidth  = width;
    int height                 = DEFAULT_HEIGHT;	int originalHeight = height;

#define DEFAULT_DAYBIN           1  // binning during the day probably isn't too useful...
#define DEFAULT_NIGHTBIN         1
    int dayBin                 = DEFAULT_DAYBIN;
    int nightBin               = DEFAULT_NIGHTBIN;
    int currentBin             = NOT_SET;

#define DEFAULT_IMAGE_TYPE       1
#define AUTO_IMAGE_TYPE         99	// needs to match what's in the camera_settings.json file
    int Image_type             = DEFAULT_IMAGE_TYPE;

#define DEFAULT_ASIBANDWIDTH    40
    int asiBandwidth           = DEFAULT_ASIBANDWIDTH;
    int asiAutoBandwidth       = 0;	// is Auto Bandwidth on or off?

    // There is no max day autoexposure since daylight exposures are always pretty short.
#define DEFAULT_ASINIGHTEXPOSURE (5 * US_IN_SEC)	// 5 seconds
    long asi_night_exposure_us = DEFAULT_ASINIGHTEXPOSURE;
#define DEFAULT_NIGHTAUTOEXPOSURE 1
    int asiNightAutoExposure   = DEFAULT_NIGHTAUTOEXPOSURE;	// is it on or off for nighttime?
    // currentAutoExposure is global so is defined outside of main()

#define DEFAULT_ASIDAYGHTGAIN    0
    int asiDayGain             = DEFAULT_ASIDAYGHTGAIN;
    int asiDayAutoGain         = 0;	// is Auto Gain on or off for daytime?
#define DEFAULT_ASINIGHTGAIN     150
    int asiNightGain           = DEFAULT_ASINIGHTGAIN;
#define DEFAULT_NIGHTAUTOGAIN    0
    int asiNightAutoGain       = DEFAULT_NIGHTAUTOGAIN;	// is Auto Gain on or off for nighttime?
#define DEFAULT_ASINIGHTMAXGAIN  200
    int asiNightMaxGain        = DEFAULT_ASINIGHTMAXGAIN;
    ASI_BOOL currentAutoGain   = ASI_FALSE;

    int currentDelay           = NOT_SET;

#define DEFAULT_ASIWBR           65
    int asiWBR                 = DEFAULT_ASIWBR;
#define DEFAULT_ASIWBB           85
    int asiWBB                 = DEFAULT_ASIWBB;
#define DEFAULT_AUTOWHITEBALANCE 0
    int asiAutoWhiteBalance    = DEFAULT_AUTOWHITEBALANCE;	// is Auto White Balance on or off?

#define DEFAULT_ASIGAMMA         50		// not supported by all cameras
    int asiGamma               = DEFAULT_ASIGAMMA;

#define DEFAULT_BRIGHTNESS 50
    int asiDayBrightness       = DEFAULT_BRIGHTNESS;
    int asiNightBrightness     = DEFAULT_BRIGHTNESS;
    int currentBrightness      = NOT_SET;

#define DEFAULT_LATITUDE         "60.7N" //GPS Coordinates of Whitehorse, Yukon where the code was created
    char const *latitude       = DEFAULT_LATITUDE;
#define DEFAULT_LONGITUDE        "135.05W"
    char const *longitude      = DEFAULT_LONGITUDE;
#define DEFAULT_ANGLE            "-6"
    // angle of the sun with the horizon
    // (0=sunset, -6=civil twilight, -12=nautical twilight, -18=astronomical twilight)
    char const *angle          = DEFAULT_ANGLE;

    int preview                = 0;
#define DEFAULT_SHOWTIME 1
    int showTime               = DEFAULT_SHOWTIME;
    char const *tempType       = "C";	// Celsius

    int showDetails            = 0;
        // Allow for more granularity than showDetails, which shows everything:
        int showTemp           = 0;
        int showExposure       = 0;
        int showGain           = 0;
        int showBrightness     = 0;
#ifdef USE_HISTOGRAM
        int showHistogram      = 0;
    int maxHistogramAttempts   = 15;	// max number of times we'll try for a better histogram mean
    int showHistogramBox       = 0;
#endif
#define DEFAULT_DAYTIMECAPTURE   0
    int daytimeCapture         = DEFAULT_DAYTIMECAPTURE;  // are we capturing daytime pictures?

    int quality                = NOT_SET;
    int asiFlip                = 0;
    int asiCoolerEnabled       = 0;
    long asiTargetTemp         = 0;
    char bufTime[128]     = { 0 };
    char bufTemp[128]     = { 0 };
    char bufTemp2[50]     = { 0 };
    char const *bayer[4]   = { "RG", "BG", "GR", "GB" };


    void init(int argc, char *argv[]);
    void help();
    void main();

    //
    static void IntHandle(int i);
    static void *SaveImgThd(void *para);
    static void *Display(void *params);

    //
    unsigned long createRGB(int r, int g, int b);
    void cvText(cv::Mat &img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname,
    int fontcolor[], int imgtype, int outlinefont);
    char* formatTime(timeval t, char const *tf);
    ASI_ERROR_CODE setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto);
    timeval getTimeval();
    char *getTime(char const *tf);
    std::string exec(const char *cmd);
    double time_diff_us(int64 start, int64 end);
    int roundTo(int n, int roundTo);
    int bytesPerPixel(ASI_IMG_TYPE imageType);
    void flush_buffered_image(int cameraId, void *buf, size_t size);
    void closeUp(int e);
    char *getRetCode(ASI_ERROR_CODE code);
    ASI_ERROR_CODE takeOneExposure(
        int cameraId,
        long exposure_time_ms,
        unsigned char *imageBuffer, long width, long height,  // where to put image and its size
        ASI_IMG_TYPE imageType,
        int *histogram,
        int *mean
    );

    void waitToFix(char const *msg);
    void calculateDayOrNight(const char *latitude, const char *longitude, const char *angle);
    int calculateTimeToNightTime(const char *latitude, const char *longitude, const char *angle);
    void writeToLog(int val);
    void writeTemperatureToFile(float val);
    char const *yesNo(int flag);
    bool resetGainTransitionVariables(int dayGain, int nightGain);
    void displayDebugText(const char * text, int requiredLevel); 
    int determineGainChange(int dayGain, int nightGain);
    char *length_in_units(float ms);
};

// Derived class
class CameraRPi : public Camera {
  public:
    void help();
};

// Derived class
class CameraRPiHQ : public CameraRPi {
  public:
    void help();
};

// Derived class
class CameraZWO : public Camera {
  public:
    void help();
};

// Derived class
class CameraZWOASI071MCPro : public CameraZWO {
  public:
    void help();
};

#endif