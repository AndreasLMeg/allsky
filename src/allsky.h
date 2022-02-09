#pragma once

#include "allsky_common.h"
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/freetype.hpp>

#include "allsky_helper.h"
#include "log.h"
#include "modeMean.h"

#if defined CAM_RPIHQ
// new includes (MEAN)
#include "include/RPiHQ_raspistill.h"
//#include "include/mode_RPiHQ_mean.h"
#elif defined CAM_ZWO
#include "include/ASICamera2.h"
#elif defined CAM_NEW
#endif

#define NOT_SET				  -1		// signifies something isn't set yet

#ifndef ASICAMERA2_H
#define ASI_TRUE true
#define ASI_FALSE false
#define ASI_IMG_RAW8	0
#define ASI_IMG_RGB24	1
#define ASI_IMG_RAW16	2
#define ASI_IMG_Y8		3
#endif


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
#define DEFAULT_DAYDELAY     (30 * MS_IN_SEC)	// 30 seconds
#define DEFAULT_NIGHTDELAY   (30 * MS_IN_SEC)	// 30 seconds
#define DEFAULT_DAYTIMECAPTURE   0
#define DEFAULT_BRIGHTNESS       50
// todo: some values are different (ASI / RPiHQ)
#if defined CAM_RPIHQ
	#define DEFAULT_FONTSIZE    32  
	#define DEFAULT_BRIGHTNESS_LIBCAMERA 0
	#define DEFAULT_NIGHTAUTOGAIN    4
	#define DEFAULT_DAYAUTOEXPOSURE  0
	#define DEFAULT_NIGHTAUTOEXPOSURE 0
#elif defined CAM_ZWO
	#define DEFAULT_FONTSIZE    7
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
	#define DEFAULT_ASINIGHTEXPOSURE (5 * US_IN_SEC)	// 5 seconds
	#define DEFAULT_ASIGAMMA         50		// not supported by all cameras
	#define DEFAULT_BOX_SIZEX       500     // Must be a multiple of 2
	#define DEFAULT_BOX_SIZEY       500     // Must be a multiple of 2
	#define DEFAULT_BOX_FROM_LEFT   0.5
	#define DEFAULT_BOX_FROM_TOP    0.5
	#define DEFAULT_PERCENTCHANGE 10.0	// percent of ORIGINAL difference
	#define DEFAULT_ASIDAYGHTGAIN    0
	#define DEFAULT_DAYAUTOEXPOSURE  1
	#define DEFAULT_NIGHTAUTOEXPOSURE 1
#elif defined CAM_NEW
	#define DEFAULT_FONTSIZE    7
	#define DEFAULT_NIGHTAUTOGAIN    4 // factor 4
	#define DEFAULT_DAYAUTOEXPOSURE  1
	#define DEFAULT_NIGHTAUTOEXPOSURE 1
#endif


class AllskyInterface {

	public:
		virtual char const *c(char const *color) = 0;
 		virtual std::string exec(const char *cmd) = 0;
};

class AllskyExternalsInterface {
public:
		virtual int system(const char *__command) = 0;
		virtual unsigned int s_sleep(unsigned int __seconds) = 0;

		//##############################################
 		virtual std::string exec(const char *cmd) = 0;
		virtual	char *getTime(char const *tf) = 0;
		virtual char *formatTime(timeval t, char const *tf) = 0;
		virtual timeval getTimeval() = 0;
		virtual void cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname, int fontcolor[], int imgtype, int outlinefont) = 0;
		virtual unsigned long createRGB(int r, int g, int b) = 0;


    virtual ~AllskyExternalsInterface() = default;
};

class AllskyExternals : public AllskyExternalsInterface {
public:

    void _cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname, int fontcolor[], int imgtype, int outlinefont);
		virtual unsigned long createRGB(int r, int g, int b);

		virtual int system(const char *__command) {
    	return std::system(__command);
		}

		virtual unsigned int s_sleep(unsigned int __seconds) {
    	return sleep(__seconds);
		}

 		virtual std::string exec(const char *cmd) {
      //std::cout << "AllskyExternals exec" << std::endl;
    	return ("todo implement and run externals.exec !");
  	}
		// Return the numeric time.
		virtual timeval getTimeval() override	{
			timeval curTime;
			gettimeofday(&curTime, NULL);
			return(curTime);
		}
		// Format a numeric time as a string.
		virtual char *formatTime(timeval t, char const *tf) override {
			static char TimeString[128];
			strftime(TimeString, 80, tf, localtime(&t.tv_sec));
			return(TimeString);
		}
		virtual char *getTime(char const *tf) override {
    	return(formatTime(getTimeval(), tf));
  	}
		virtual void cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname, int fontcolor[], int imgtype, int outlinefont) override {
			_cvText(img, text, x, y, fontsize, linewidth, linetype, fontname, fontcolor, imgtype, outlinefont);
		} 
};
class Allsky: public AllskyInterface, public AllskyHelper, public Log, public ModeMean {
	//enums
	public:
		enum Status
		{
			StatusUndefined = 0,
			StatusInit,
			StatusLoop,
			StatusCloseup
		};

	//variables
	public:
		AllskyExternalsInterface *externals = nullptr;
		struct Allsky_runtime {
			Status status;
			bool endOfNight;
			bool gotSignal;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
			std::string dayOrNight;
			std::string lastDayOrNight;
			//Camera* myCam;
		  char exposureStart[128];
		} static runtime;

		struct Allsky_valuesCapture {
			int exposure_us;
			double gain;
			int gain_dB;
		} static valuesCapture;

		struct Allsky_current {
			int currentDelay_ms;
		} static current;

		struct Allsky_settings {
			int debugLevel;
			bool tty;
			int notificationImages;
			char const *angle;
			char const *latitude;
			char const *longitude;
			int taking_dark_frames;
			int preview;
			struct day{
				int daytimeCapture;  // are we capturing daytime pictures?
				int dayDelay_ms;	// Delay in milliseconds.
			} day;
			struct night{
				int nightDelay_ms;	// Delay in milliseconds.
			} night;
			struct image{
				//   - image-capture
				cv::Mat pRgb;	// the image
				int asiFlip;
				#if defined CAM_RPIHQ
				int asiRotation;
				#endif
				//   - image-destination
				char const *fileName;
				int Image_type;
				int width;		
				int height;	
				int quality;
			} image;
			struct camera{
				char const *cameraName;
				#if defined CAM_RPIHQ
				bool is_libcamera;
				#endif
				int asiNightBrightness;
				int asiDayBrightness;
				int asiNightAutoExposure;	// is it on or off for nighttime?
				int asiDayAutoExposure;	// is it on or off for daylight?
				int asiAutoAWB;	// is Auto White Balance on or off?
				int asiNightAutoGain;	// is Auto Gain on or off for nighttime?
				#if defined CAM_RPIHQ
				float saturation;
				double asiWBR;
				double asiWBB;
				double asiNightGain;
				double asiDayGain;
				#elif defined CAM_ZWO
				int asiWBR;
				int asiWBB;
				int asiDayGain;
				int asiNightGain;
				#elif defined CAM_NEW
				double asiWBR;
				double asiWBB;
				double asiNightGain;
				double asiDayGain;
				#endif
				int dayBin;
				int nightBin;
			} camera;
			struct fonts{
				double fontsize;
				int fontnumber;
				int fontcolor[3];
				int smallFontcolor[3];
				int outlinefont;
			} fonts;
		} static settings;
		static int showTime;
		static char bufTime[128];
		static int iTextX;
		static int iTextY;
		static int currentBin;
		static int linewidth;
	 	static int linetype[3];
		static int linenumber; // todo: change the name
		static int fontname[];
		static char const *fontnames[];
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
		static char const *ImgExtraText;
		static int extraFileAge;   // 0 disables it
		static char const *timeFormat;
		static int showDetails;
		static const char *locale;
	// angle of the sun with the horizon
	// (0=sunset, -6=civil twilight, -12=nautical twilight, -18=astronomical twilight)
		static int originalWidth;
		static int originalHeight;
		static int numExposures;	// how many valid pictures have we taken so far?
		static int asiDayAutoGain;
		static int asiDayExposure_us;
		static int asiNightExposure_us;
		static std::vector<int> compression_params;
#if defined CAM_RPIHQ
		//static modeMeanSetting myModeMeanSetting;
		static raspistillSetting myRaspistillSetting;
		static int background;
		static int min_brightness;					// what user enters on command line
		static int max_brightness;
		static int default_brightness;
		static float min_saturation;				// produces black and white
		static float max_saturation;
		static float default_saturation;
#elif defined CAM_ZWO
		static const char *sType;		// displayed in output
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
		static ASI_ERROR_CODE asiRetCode;  // used for return code from ASI functions.
		static long bufferSize;
		#ifdef USE_HISTOGRAM
		static float histogramBoxPercentFromLeft;
		static float histogramBoxPercentFromTop;
		static int showHistogram;
		static int maxHistogramAttempts;	// max number of times we'll try for a better histogram mean
		static int showHistogramBox;
		static const int percent_change;
		#endif	// USE_HISTOGRAM
		static int current_histogramBoxSizeX;
		static int current_histogramBoxSizeY;
		static int histogramBoxSizeX;
		static int histogramBoxSizeY;
		static int originalITextX;
		static int originalITextY;
		static int originalFontsize;
		static int originalLinewidth;
		static long current_exposure_us;
		static int gainChange;			// how much to change gain up or down
		static long camera_max_autoexposure_us;	// camera's max auto-exposure
		static long camera_min_exposure_us;	// camera's minimum exposure
		static int numGainChanges;		// This is reset at every day/night and night/day transition.
		static bool adjustGain;	// Should we adjust the gain?  Set by user on command line.
		static bool currentAdjustGain;	// Adjusting it right now?
		static int totalAdjustGain;	// The total amount to adjust gain.
		static int perImageAdjustGain;	// Amount of gain to adjust each image
		static int gainTransitionImages;
		static int displayedNoDaytimeMsg;
		static int exitCode;
		static std::vector<int> compression_parameters; // todo needed ????
		static bool bSavingImg;
		static ASI_CONTROL_CAPS ControlCaps;

#endif

		Allsky ();
		Allsky (int argc, char *argv[]);
   	Allsky(AllskyExternalsInterface *Externals)
    : externals(Externals) {
        // If the dependency was not defined, throw an exception.
        if(externals == nullptr){
            throw std::invalid_argument("service must not be null");
        }
    }
		// camera API
		/* all camara depending things before the capture */
		virtual void setupForCapture(void) = 0;
		/* Camera captures one image */
		virtual int capture(void) = 0;
		/* all camara depending things after the capture */
		virtual void postCapture(void) = 0;
		virtual void waitForNextCapture(void) = 0;
		virtual void prepareForDayOrNight(void) = 0;

		// main functions
		void init(int argc, char *argv[]);
		void info(void);
		int run(void);
		void preCapture(void);
		void deliverImage(void);
		static void closeUp(int e);
		// some other functions
		void overlayText(int &);
		//static void cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname, int fontcolor[], int imgtype, int outlinefont);
		static bool dayOrNightNotChanged(void);
		void waitToFix(char const *msg);
		static char const *_c(char const *color);
		static std::string _exec(const char *cmd);
		static void IntHandle(int i);
		void calculateDayOrNight(void);
		int calculateTimeToNightTime(void);

		// interface
 		virtual char const *c(char const *color) override {
    	return _c(color);
  	}

 		virtual std::string exec(const char *cmd) override {
    	return (_exec(cmd));
  	}


		// depended function:
		
};
