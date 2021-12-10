#ifndef ALLSKY_H
#define ALLSKY_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
// new includes (MEAN)
#include "include/RPiHQ_raspistill.h"
#include "include/mode_RPiHQ_mean.h"

#define NOT_SET				  -1		// signifies something isn't set yet

#ifndef ASI_IMG_RAW8
#define ASI_IMG_RAW8	0
#define ASI_IMG_RGB24	1
#define ASI_IMG_RAW16	2
#define ASI_IMG_Y8		3
#endif

#ifndef ASI_TRUE
#define ASI_TRUE true
#define ASI_FALSE false
#endif

#define US_IN_MS 1000                     // microseconds in a millisecond
#define MS_IN_SEC 1000                    // milliseconds in a second
#define US_IN_SEC (US_IN_MS * MS_IN_SEC)  // microseconds in a second

#define DEFAULT_SHOWTIME         1
#define DEFAULT_ITEXTX      15
#define DEFAULT_ITEXTY      25
#define DEFAULT_FONTSIZE    32
#define DEFAULT_LINEWIDTH   1
#define DEFAULT_LINENUMBER       0
#define DEFAULT_FONTNUMBER  0
#define AUTO_IMAGE_TYPE     99	// needs to match what's in the camera_settings.json file
#define DEFAULT_IMAGE_TYPE       AUTO_IMAGE_TYPE
#define SMALLFONTSIZE_MULTIPLIER 0.08
#define DEFAULT_OUTLINEFONT 0
#define DEFAULT_ITEXTLINEHEIGHT  30



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






    static modeMeanSetting myModeMeanSetting;
    static raspistillSetting myRaspistillSetting;



    static void overlayText();
    static void cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname,
            int fontcolor[], int imgtype, int outlinefont);
    static unsigned long createRGB(int r, int g, int b);
    static void Log(int required_level, const char *fmt, ...);
};

#endif