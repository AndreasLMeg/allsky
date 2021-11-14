#ifndef ALLSKY_H
#define ALLSKY_H

#include "include/ASICamera2.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/viz.hpp>


#define US_IN_MS 1000                     // microseconds in a millisecond
#define MS_IN_SEC 1000                    // milliseconds in a second
#define US_IN_SEC (US_IN_MS * MS_IN_SEC)  // microseconds in a second

#define DEFAULT_FILENAME     "image.jpg"

// Base class
class Allsky {
  public:
    static int debugLevel;
    static int iNumOfCtrl;
    static ASI_CONTROL_CAPS ControlCaps;
    static char debug_text[500];		// buffer to hold debug messages displayed by displayDebugText()
    static char debug_text2[100];		// buffer to hold additional message
    static bool bDisplay;
    static bool bSaveRun;
    static pthread_mutex_t mtx_SaveImg;
    static pthread_cond_t cond_SatrtSave;
    static int gotSignal;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
    static bool bSavingImg;
    static int taking_dark_frames;
    static std::string dayOrNight;
    static char const *fileName;
    static cv::Mat pRgb;
    static long actualTemp;	// actual sensor temp, per the camera
    static long current_exposure_us;
    static std::vector<int> compression_parameters;


    static void displayDebugText(const char * text, int requiredLevel);
    static unsigned long createRGB(int r, int g, int b);
    static void cvText(cv::Mat &img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname,
            int fontcolor[], int imgtype, int outlinefont);
    static timeval getTimeval();
    static char *formatTime(timeval t, char const *tf);
    static char *getTime(char const *tf);
    static double time_diff_us(int64 start, int64 end);
    static std::string exec(const char *cmd);
    static void *Display(void *params);
    static void *SaveImgThd(void *para);


    static ASI_ERROR_CODE setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto);

};

#endif