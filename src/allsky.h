#ifndef ALLSKY_H
#define ALLSKY_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
// new includes (MEAN)
#include "include/RPiHQ_raspistill.h"
#include "include/mode_RPiHQ_mean.h"

#define NOT_SET				  -1		// signifies something isn't set yet

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#ifndef ASICAMERA2_H
#define ASI_TRUE true
#define ASI_FALSE false
#define ASI_IMG_RAW8	0
#define ASI_IMG_RGB24	1
#define ASI_IMG_RAW16	2
#define ASI_IMG_Y8		3
#endif

#define US_IN_MS 1000                     // microseconds in a millisecond
#define MS_IN_SEC 1000                    // milliseconds in a second
#define US_IN_SEC (US_IN_MS * MS_IN_SEC)  // microseconds in a second

#define DEFAULT_SHOWTIME         1
#define DEFAULT_ITEXTX      15
#define DEFAULT_ITEXTY      25
#define DEFAULT_LINEWIDTH   1
#define DEFAULT_LINENUMBER       0
#define DEFAULT_FONTNUMBER  0
#define AUTO_IMAGE_TYPE     99	// needs to match what's in the camera_settings.json file
#define DEFAULT_IMAGE_TYPE       AUTO_IMAGE_TYPE
#define SMALLFONTSIZE_MULTIPLIER 0.08
#define DEFAULT_OUTLINEFONT 0
#define DEFAULT_ITEXTLINEHEIGHT  30
#define DEFAULT_NOTIFICATIONIMAGES 1
#define DEFAULT_TIMEFORMAT   "%Y%m%d %H:%M:%S"	// format the time should be displayed in
#define DEFAULT_AUTOWHITEBALANCE 0
#define DEFAULT_LOCALE           "en_US.UTF-8"
#define DEFAULT_ANGLE            "-6"
#define DEFAULT_LATITUDE         "60.7N" //GPS Coordinates of Whitehorse, Yukon where the code was created
#define DEFAULT_LONGITUDE        "135.05W"
#define DEFAULT_FILENAME     "image.jpg"
#define DEFAULT_WIDTH            0
#define DEFAULT_HEIGHT           0
#define DEFAULT_DAYBIN           1  // binning during the day probably isn't too useful...
#define DEFAULT_NIGHTBIN         1
#define DEFAULT_DAYSKIPFRAMES    5
#define DEFAULT_NIGHTSKIPFRAMES  1
#define DEFAULT_DAYDELAY     (5 * MS_IN_SEC)	// 5 seconds
#define DEFAULT_NIGHTDELAY   (10 * MS_IN_SEC)	// 10 seconds
#define DEFAULT_DAYTIMECAPTURE   0
// todo: some values are different (ASI / RPiHQ)
#ifdef CAM_RPIHQ
	#define DEFAULT_FONTSIZE    32  
	#define DEFAULT_BRIGHTNESS       50
	#define DEFAULT_BRIGHTNESS_LIBCAMERA 0
	#define DEFAULT_NIGHTAUTOGAIN    4
	#define DEFAULT_DAYAUTOEXPOSURE  0
	#define DEFAULT_NIGHTAUTOEXPOSURE 0
#else
	#define DEFAULT_FONTSIZE    7
	#define DEFAULT_BRIGHTNESS       50
	#define DEFAULT_ASIBANDWIDTH    40
	#define DEFAULT_ASIWBR           65
	#define DEFAULT_ASIWBB           85
	#define DEFAULT_AGGRESSION       50
	#define DEFAULT_GAIN_TRANSITION_TIME 5		// user specifies minutes
	#define DEFAULT_ASINIGHTMAXGAIN  200
	#define DEFAULT_NIGHTAUTOGAIN    0
	#define DEFAULT_ASINIGHTGAIN     150
	#define DEFAULT_ASINIGHTMAXAUTOEXPOSURE_MS  (20 * MS_IN_SEC)	// 20 seconds
	#define DEFAULT_ASIDAYEXPOSURE   500	// microseconds - good starting point for daytime exposures
	#define DEFAULT_ASIDAYMAXAUTOEXPOSURE_MS  (60 * MS_IN_SEC)	// 60 seconds
	#define DEFAULT_DAYAUTOEXPOSURE  1
	#define DEFAULT_ASINIGHTEXPOSURE (5 * US_IN_SEC)	// 5 seconds
	#define DEFAULT_NIGHTAUTOEXPOSURE 1
	#define DEFAULT_ASIGAMMA         50		// not supported by all cameras
#endif

class Allsky {
	public:
		static int showTime;
		static cv::Mat pRgb;	// the image
		static char bufTime[128];
		static int iTextX;
		static int iTextY;
		static int currentBin;
		static double fontsize;
		static int linewidth;
	 	static int linetype[3];
		static int linenumber;
		static int fontname[];
		static char const *fontnames[];
		static int fontnumber;
		static int fontcolor[3];
		static int smallFontcolor[3];
		static int Image_type;
		static int outlinefont;
		static int iTextLineHeight;
		static char const *ImgText;
		static char bufTemp[128];
		static char bufTemp2[50];
		static int showTemp;
		static int showExposure;
		static int showGain;
		static int showBrightness;
		static int showMean;
		static int showFocus;
		static int currentExposure_us;
	 	static int currentBrightness;
		static int currentAutoGain;
		static double currentGain;
		static double lastGain;
		static float lastMean;
		static int currentAutoExposure;
		static long actualTemp;	// actual sensor temp, per the camera
		static char const *tempType;	// Celsius
		static char debugText[500];		// buffer to hold debug messages
		static int debugLevel;
		static char const *ImgExtraText;
		static int extraFileAge;   // 0 disables it
		static bool tty;	// are we on a tty?
		static int notificationImages;
		static char const *timeFormat;
		static int asiAutoAWB;	// is Auto White Balance on or off?
		static int taking_dark_frames;
		static int preview;
		static int showDetails;
		static const char *locale;
	// angle of the sun with the horizon
	// (0=sunset, -6=civil twilight, -12=nautical twilight, -18=astronomical twilight)
		static char const *angle;
		static char const *latitude;
		static char const *longitude;
		static char const *fileName;
		static int asiFlip;
		static int width;		
		static int originalWidth;
		static int height;
		static int originalHeight;
		static int dayBin;
		static int nightBin;
		static int dayDelay_ms;	// Delay in milliseconds.
		static int nightDelay_ms;	// Delay in milliseconds.
		static int daytimeCapture;  // are we capturing daytime pictures?
		static int quality;
		static const char *sType;		// displayed in output
		static int asiNightBrightness;
		static int asiDayBrightness;
		static int asiNightAutoExposure;	// is it on or off for nighttime?
		static int asiDayAutoExposure;	// is it on or off for daylight?
		static int asiNightAutoGain;	// is Auto Gain on or off for nighttime?
		static int gotSignal;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
		static std::string dayOrNight;


#ifdef CAM_RPIHQ
		static modeMeanSetting myModeMeanSetting;
		static raspistillSetting myRaspistillSetting;
		static double asiWBR;
		static double asiWBB;
		static double asiNightGain;
	  static double asiDayGain;
		static int asiRotation;
		static int background;
		static float saturation;

	
#else
		static int asiWBR;
		static int asiWBB;
		static bool use_new_exposure_algorithm;
		static int asiBandwidth;
		static int asiAutoBandwidth;	// is Auto Bandwidth on or off?
		static ASI_CAMERA_INFO ASICameraInfo;
		static long asiTargetTemp;
		static int asiCoolerEnabled;
		static int aggression; // ala PHD2.  Percent of change made, 1 - 100.
		static int day_skip_frames;
		static int night_skip_frames;
		static int current_skip_frames;
		static int gainTransitionTime;
		static int asiNightMaxGain;
		static int asiNightGain;
		static int asi_night_max_autoexposure_ms;
		static long current_max_autoexposure_us;
		static long asi_day_exposure_us;
		static int asi_day_max_autoexposure_ms;
		static long asi_night_exposure_us;
		static int asiGamma;
		static int CamNum;
		static bool bDisplay;
		static pthread_t thread_display;
		static pthread_t hthdSave;
		static void *retval;
		static bool bSaveRun;
		static pthread_mutex_t mtx_SaveImg;
		static pthread_cond_t cond_SatrtSave;



#endif

		static void overlayText(int &);
		static void cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname, int fontcolor[], int imgtype, int outlinefont);
		static unsigned long createRGB(int r, int g, int b);
		static void Log(int required_level, const char *fmt, ...);
		static char *length_in_units(long us, bool multi);
		static void waitToFix(char const *msg);
		static void init(int argc, char *argv[]);
		static void info(void);
		static char const *yesNo(int flag);
		static char const *c(char const *color);
		static timeval getTimeval();
		static char *formatTime(timeval t, char const *tf);
		static char *getTime(char const *tf);
		static std::string exec(const char *cmd);
		static void IntHandle(int i);
		static void closeUp(int e);
		static void calculateDayOrNight(const char *latitude, const char *longitude, const char *angle);
		static int calculateTimeToNightTime(const char *latitude, const char *longitude, const char *angle);



};

#endif