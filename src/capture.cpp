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

#define USE_HISTOGRAM                     // use the histogram code as a workaround to ZWO's bug

// Forward definitions
char *getRetCode(ASI_ERROR_CODE);

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


std::vector<int> compression_parameters;
// In version 0.8 we introduced a different way to take exposures.  Instead of turning video mode on at
// the beginning of the program and off at the end (which kept the camera running all the time, heating it up),
// version 0.8 turned video mode on, then took a picture, then turned it off.  This helps cool the camera,
// but some users (seems hit or miss) get ASI_ERROR_TIMEOUTs when taking exposures.
// So, we added the ability for them to use the 0.7 video-always-on method, or the 0.8 "new exposure" method.
bool bMain = true, bDisplay = false;
std::string dayOrNight;

bool bSaveRun = false, bSavingImg = false;
pthread_mutex_t mtx_SaveImg;
pthread_cond_t cond_SatrtSave;

// These are global so they can be used by other routines.
#define NOT_SET -1	// signifies something isn't set yet
ASI_CONTROL_CAPS ControlCaps;
void *retval;
int numErrors              = 0;	// Number of errors in a row.
int gotSignal              = 0;	// did we get a SIGINT (from keyboard) or SIGTERM (from service)?
int iNumOfCtrl             = 0;
int CamNum                 = 0;
pthread_t thread_display   = 0;
pthread_t hthdSave         = 0;
int numExposures           = 0;	// how many valid pictures have we taken so far?
long camera_max_autoexposure_us= NOT_SET;	// camera's max auto-exposure
long camera_min_exposure_us= 100;	// camera's minimum exposure
long current_exposure_us   = NOT_SET;

// Some command-line and other option definitions needed outside of main():



#ifdef USE_HISTOGRAM
#define DEFAULT_BOX_SIZEX       500     // Must be a multiple of 2
#define DEFAULT_BOX_SIZEY       500     // Must be a multiple of 2
int current_histogramBoxSizeX = NOT_SET;
int current_histogramBoxSizeY = NOT_SET;
#define DEFAULT_BOX_FROM_LEFT   0.5
#define DEFAULT_BOX_FROM_TOP    0.5
// % from left/top side that the center of the box is.  0.5 == the center of the image's X/Y axis
float histogramBoxPercentFromLeft = DEFAULT_BOX_FROM_LEFT;
float histogramBoxPercentFromTop = DEFAULT_BOX_FROM_TOP;
#endif	// USE_HISTOGRAM



// Make sure we don't try to update a non-updateable control, and check for errors.
ASI_ERROR_CODE setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto)
{
	ASI_ERROR_CODE ret = ASI_SUCCESS;
	int i;
	for (i = 0; i < iNumOfCtrl && i <= control; i++)    // controls are sorted 1 to n
	{
		ret = ASIGetControlCaps(CamNum, i, &ControlCaps);
		if (ret != ASI_SUCCESS)
		{
			Allsky::Log(0, "WARNING: ASIGetControlCaps() for control %d failed: %s\n", i, getRetCode(ret));
			return(ret);
		}

		if (ControlCaps.ControlType == control)
		{
			if (ControlCaps.IsWritable)
			{
				if (value > ControlCaps.MaxValue)
				{
					printf("WARNING: Value of %ld greater than max value allowed (%ld) for control '%s' (#%d).\n", value, ControlCaps.MaxValue, ControlCaps.Name, ControlCaps.ControlType);
					value = ControlCaps.MaxValue;
				} else if (value < ControlCaps.MinValue)
				{
					printf("WARNING: Value of %ld less than min value allowed (%ld) for control '%s' (#%d).\n", value, ControlCaps.MinValue, ControlCaps.Name, ControlCaps.ControlType);
					value = ControlCaps.MinValue;
				}
				if (makeAuto == ASI_TRUE && ControlCaps.IsAutoSupported == ASI_FALSE)
				{
					printf("WARNING: control '%s' (#%d) doesn't support auto mode.\n", ControlCaps.Name, ControlCaps.ControlType);
					makeAuto = ASI_FALSE;
				}
				ret = ASISetControlValue(CamNum, control, value, makeAuto);
				if (ret != ASI_SUCCESS)
				{
					Allsky::Log(0, "WARNING: ASISetControlCaps() for control %d, value=%ld failed: %s\n", control, value, getRetCode(ret));
					return(ret);
				}
			} else {
				printf("ERROR: ControlCap: '%s' (#%d) not writable; not setting to %ld.\n", ControlCaps.Name, ControlCaps.ControlType, value);
				ret = ASI_ERROR_INVALID_MODE;	// this seemed like the closest error
			}
			return ret;
		}
	}
	Allsky::Log(3, "NOTICE: Camera does not support ControlCap # %d; not setting to %ld.\n", control, value);
	return ASI_ERROR_INVALID_CONTROL_TYPE;
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

// Return the numeric time.
timeval getTimeval()
{
	timeval curTime;
	gettimeofday(&curTime, NULL);
	return(curTime);
}

// Format a numeric time as a string.
char *formatTime(timeval t, char const *tf)
{
	static char TimeString[128];
	strftime(TimeString, 80, tf, localtime(&t.tv_sec));
	return(TimeString);
}

// Return the current time as a string.  Uses both functions above.
char *getTime(char const *tf)
{
	return(formatTime(getTimeval(), tf));
}

double time_diff_us(int64 start, int64 end)
{
	double frequency = cv::getTickFrequency();
	return (double)(end - start) / frequency;	// in Microseconds
}

std::string exec(const char *cmd)
{
	std::tr1::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe)
		return "ERROR";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe.get()))
	{
		if (fgets(buffer, 128, pipe.get()) != NULL)
		{
			result += buffer;
		}
	}
	return result;
}

void *Display(void *params)
{
	cv::Mat *pImg = (cv::Mat *)params;
	int w = pImg->cols;
	int h = pImg->rows;
	cv::namedWindow("Preview", cv::WINDOW_AUTOSIZE);
	cv::Mat Img2 = *pImg, *pImg2 = &Img2;

	while (bDisplay)
	{
		// default preview size usually fills whole screen, so shrink.
		cv::resize(*pImg, *pImg2, cv::Size((int)w/2, (int)h/2));
		cv::imshow("Preview", *pImg2);
		cv::waitKey(500);	// TODO: wait for exposure time instead of hard-coding value
	}
	cv::destroyWindow("Preview");
	printf("Display thread over\n");
	return (void *)0;
}

void *SaveImgThd(void *para)
{
	while (bSaveRun)
	{
		pthread_mutex_lock(&mtx_SaveImg);
		pthread_cond_wait(&cond_SatrtSave, &mtx_SaveImg);

		if (gotSignal)
		{
			// we got a signal to exit, so don't save the (probably incomplete) image
			pthread_mutex_unlock(&mtx_SaveImg);
			break;
		}

		bSavingImg = true;

		Allsky::Log(1, "  > Saving %s image '%s'\n", Allsky::taking_dark_frames ? "dark" : dayOrNight.c_str(), Allsky::fileName);
		int64 st, et;

		bool result = false;
		if (Allsky::pRgb.data)
		{
			const char *s;	// TODO: use saveImage.sh
			if (dayOrNight == "NIGHT")
			{
				s = "scripts/saveImageNight.sh";
			}
			else
			{
				s = "scripts/saveImageDay.sh";
			}

			char cmd[100];
			// imwrite() may take several seconds and while it's running, "fileName" could change,
			// so set "cmd" before imwrite().
			// The temperature must be a 2-digit number with an optional "-" sign.
			sprintf(cmd, "%s %s '%s' '%2.0f' %ld &", s, dayOrNight.c_str(), Allsky::fileName, (float) Allsky::actualTemp/10, current_exposure_us);
			st = cv::getTickCount();
			try
			{
				result = imwrite(Allsky::fileName, Allsky::pRgb, compression_parameters);
			}
			catch (const cv::Exception& ex)
			{
				printf("*** ERROR: Exception saving image: %s\n", ex.what());
			}
			et = cv::getTickCount();

			if (result)
				system(cmd);
			else
				printf("*** ERROR: Unable to save image '%s'.\n", Allsky::fileName);

		} else {
			// This can happen if the program is closed before the first picture.
			Allsky::Log(2, "----- SaveImgThd(): pRgb.data is null\n");
		}
		bSavingImg = false;

		if (result)
		{
			static int total_saves = 0;
			static double total_time_ms = 0;
			total_saves++;
			double diff = time_diff_us(st, et) * US_IN_MS;	// we want ms
			total_time_ms += diff;
			char const *x;
			if (diff > 1 * MS_IN_SEC)
			   x = "  > *****\n";	// indicate when it takes a REALLY long time to save
			else
			   x = "";
			Allsky::Log(4, "%s  > Image took %'.1f ms to save (average %'.1f ms).\n%s", x, diff, total_time_ms / total_saves, x);
		}

		pthread_mutex_unlock(&mtx_SaveImg);
	}

	return (void *)0;
}

char retCodeBuffer[100];
int asi_error_timeout_cntr = 0;

// Display ASI errors in human-readable format
char *getRetCode(ASI_ERROR_CODE code)
{
	std::string ret;
	if (code == ASI_SUCCESS) ret = "ASI_SUCCESS";
	else if (code == ASI_ERROR_INVALID_INDEX) ret = "ASI_ERROR_INVALID_INDEX";
	else if (code == ASI_ERROR_INVALID_ID) ret = "ASI_ERROR_INVALID_ID";
	else if (code == ASI_ERROR_INVALID_CONTROL_TYPE) ret = "ASI_ERROR_INVALID_CONTROL_TYPE";
	else if (code == ASI_ERROR_CAMERA_CLOSED) ret = "ASI_ERROR_CAMERA_CLOSED";
	else if (code == ASI_ERROR_CAMERA_REMOVED) ret = "ASI_ERROR_CAMERA_REMOVED";
	else if (code == ASI_ERROR_INVALID_PATH) ret = "ASI_ERROR_INVALID_PATH";
	else if (code == ASI_ERROR_INVALID_FILEFORMAT) ret = "ASI_ERROR_INVALID_FILEFORMAT";
	else if (code == ASI_ERROR_INVALID_SIZE) ret = "ASI_ERROR_INVALID_SIZE";
	else if (code == ASI_ERROR_INVALID_IMGTYPE) ret = "ASI_ERROR_INVALID_IMGTYPE";
	else if (code == ASI_ERROR_OUTOF_BOUNDARY) ret = "ASI_ERROR_OUTOF_BOUNDARY";
	else if (code == ASI_ERROR_TIMEOUT)
	{
		// To aid in debugging these errors, keep track of how many we see.
		asi_error_timeout_cntr += 1;
		ret = "ASI_ERROR_TIMEOUT #" + std::to_string(asi_error_timeout_cntr) +
			  " (with 0.8 exposure = " + ((Allsky::use_new_exposure_algorithm)?("YES"):("NO")) + ")";
	}
	else if (code == ASI_ERROR_INVALID_SEQUENCE) ret = "ASI_ERROR_INVALID_SEQUENCE";
	else if (code == ASI_ERROR_BUFFER_TOO_SMALL) ret = "ASI_ERROR_BUFFER_TOO_SMALL";
	else if (code == ASI_ERROR_VIDEO_MODE_ACTIVE) ret = "ASI_ERROR_VIDEO_MODE_ACTIVE";
	else if (code == ASI_ERROR_EXPOSURE_IN_PROGRESS) ret = "ASI_ERROR_EXPOSURE_IN_PROGRESS";
	else if (code == ASI_ERROR_GENERAL_ERROR) ret = "ASI_ERROR_GENERAL_ERROR";
	else if (code == ASI_ERROR_END) ret = "ASI_ERROR_END";
	else if (code == -1) ret = "Non-ASI ERROR";
	else ret = "UNKNOWN ASI ERROR";

	sprintf(retCodeBuffer, "%s (%d)", ret.c_str(), (int) code);
	return(retCodeBuffer);
}

long roundTo(long n, int roundTo)
{
	long a = (n / roundTo) * roundTo;	// Smaller multiple
	long b = a + roundTo;				// Larger multiple
	return (n - a > b - n)? b : a;		// Return of closest of two
}

int bytesPerPixel(ASI_IMG_TYPE imageType) {
	switch (imageType) {
		case ASI_IMG_RGB24:
			return 3;
			break;
		case ASI_IMG_RAW16:
			return 2;
			break;
		case ASI_IMG_RAW8:
		case ASI_IMG_Y8:
		default:
			return 1;
	}
}

#ifdef USE_HISTOGRAM
// As of July 2021, ZWO's SDK (version 1.9) has a bug where autoexposure daylight shots'
// exposures jump all over the place.  One is way too dark and the next way too light, etc.
// As a workaround, our histogram code replaces ZWO's code auto-exposure mechanism.
// We look at the mean brightness of an X by X rectangle in image, and adjust exposure based on that.

// FIXME prevent this from misbehaving when unreasonable settings are given,
// eg. box size 0x0, box size WxW, box crosses image edge, ... basically
// anything that would read/write out-of-bounds

int computeHistogram(unsigned char *imageBuffer, int width, int height, ASI_IMG_TYPE imageType, int *histogram)
{
	int h, i;
	unsigned char *buf = imageBuffer;

	// Clear the histogram array.
	for (h = 0; h < 256; h++) {
		histogram[h] = 0;
	}

	// Different image types have a different number of bytes per pixel.
	int bpp = bytesPerPixel(imageType);
	width *= bpp;
	int roiX1 = (width * histogramBoxPercentFromLeft) - (current_histogramBoxSizeX * bpp / 2);
	int roiX2 = roiX1 + (bpp * current_histogramBoxSizeX);
	int roiY1 = (height * histogramBoxPercentFromTop) - (current_histogramBoxSizeY / 2);
	int roiY2 = roiY1 + current_histogramBoxSizeY;

	// Start off and end on a logical pixel boundries.
	roiX1 = (roiX1 / bpp) * bpp;
	roiX2 = (roiX2 / bpp) * bpp;

	// For RGB24, data for each pixel is stored in 3 consecutive bytes: blue, green, red.
	// For all image types, each row in the image contains one row of pixels.
	// bpp doesn't apply to rows, just columns.
	switch (imageType) {
	case ASI_IMG_RGB24:
	case ASI_IMG_RAW8:
	case ASI_IMG_Y8:
		for (int y = roiY1; y < roiY2; y++) {
			for (int x = roiX1; x < roiX2; x+=bpp) {
				i = (width * y) + x;
				int total = 0;
				for (int z = 0; z < bpp; z++)
				{
					// For RGB24 this averages the blue, green, and red pixels.
					total += buf[i+z];
				}
				int avg = total / bpp;
				histogram[avg]++;
			}
		}
		break;
	case ASI_IMG_RAW16:
		for (int y = roiY1; y < roiY2; y++) {
			for (int x = roiX1; x < roiX2; x+=bpp) {
				i = (width * y) + x;
				int pixelValue;
				// This assumes the image data is laid out in big endian format.
				// We are going to grab the most significant byte
				// and use that for the histogram value ignoring the
				// least significant byte so we can use the 256 value histogram array.
				// If it's acutally little endian then add a +1 to the array subscript for buf[i].
				pixelValue = buf[i];
				histogram[pixelValue]++;
			}
		}
		break;
	case ASI_IMG_END:
		break;
	}

	// Now calculate the mean.
	int meanBin = 0;
	int a = 0, b = 0;
	for (int h = 0; h < 256; h++) {
		a += (h+1) * histogram[h];
		b += histogram[h];
	}

	if (b == 0)
	{
		// This is one heck of a dark picture!
		return(0);
	}

	meanBin = a/b - 1;
	return meanBin;
}
#endif

// This is based on code from PHD2.
// Camera has 2 internal frame buffers we need to clear.
// The camera and/or driver will buffer frames and return the oldest one which
// could be very old. Read out all the buffered frames so the frame we get is current.
static void flush_buffered_image(int cameraId, void *buf, size_t size)
{
	enum { NUM_IMAGE_BUFFERS = 2 };

	int num_cleared;
	for (num_cleared = 0; num_cleared < NUM_IMAGE_BUFFERS; num_cleared++)
	{
		ASI_ERROR_CODE status = ASIGetVideoData(cameraId, (unsigned char *) buf, size, 0);
		if (status != ASI_SUCCESS)
			break; // no more buffered frames
long us;
ASI_BOOL b;
ASIGetControlValue(cameraId, ASI_EXPOSURE, &us, &b);
Allsky::Log(3, "  > [Cleared buffer frame, next exposure: %'ld, auto=%s]\n", us, b==ASI_TRUE ? "yes" : "no");
	}

// xxxxxxxxxx For now, display message above for each one rather than a summary.
return;
	if (num_cleared > 0)
	{
		Allsky::Log(3, "  > [Cleared %d buffer frame%s]\n", num_cleared, num_cleared > 1 ? "s" : "");
	}
}


long last_exposure_us = 0;		// last exposure taken
long reported_exposure_us = 0;	// exposure reported by the camera, either actual exposure or suggested next one
long actualGain = 0;			// actual gain used, per the camera
ASI_BOOL bAuto = ASI_FALSE;		// "auto" flag returned by ASIGetControlValue, when we don't care what it is

ASI_BOOL wasAutoExposure = ASI_FALSE;
long bufferSize = NOT_SET;

ASI_ERROR_CODE takeOneExposure(
		int cameraId,
		long exposure_time_us,
		unsigned char *imageBuffer, long width, long height,  // where to put image and its size
		ASI_IMG_TYPE imageType,
		int *histogram,
		int *mean
)
{
	if (imageBuffer == NULL) {
		return (ASI_ERROR_CODE) -1;
	}

	ASI_ERROR_CODE status;
	// ZWO recommends timeout = (exposure*2) + 500 ms
	// After some discussion, we're doing +5000ms to account for delays induced by
	// USB contention, such as that caused by heavy USB disk IO
	long timeout = ((exposure_time_us * 2) / US_IN_MS) + 5000;	// timeout is in ms

	if (Allsky::currentAutoExposure && exposure_time_us > Allsky::current_max_autoexposure_us)
	{
		// If we call length_in_units() twice in same command line they both return the last value.
		char x[100];
		snprintf(x, sizeof(x), "%s", Allsky::length_in_units(exposure_time_us, true));
		Allsky::Log(0, "*** WARNING: exposure_time_us requested [%s] > current_max_autoexposure_us [%s]\n", x, Allsky::length_in_units(Allsky::current_max_autoexposure_us, true));
		exposure_time_us = Allsky::current_max_autoexposure_us;
	}

	// This debug message isn't typcally needed since we already displayed a message about
	// starting a new exposure, and below we display the result when the exposure is done.
	Allsky::Log(4, "  > %s to %s, timeout: %'ld ms\n",
		wasAutoExposure == ASI_TRUE ? "Camera set auto-exposure" : "Exposure set",
		Allsky::length_in_units(exposure_time_us, true), timeout);

	setControl(cameraId, ASI_EXPOSURE, exposure_time_us, (ASI_BOOL)Allsky::currentAutoExposure);

	flush_buffered_image(cameraId, imageBuffer, bufferSize);

	if (Allsky::use_new_exposure_algorithm)
	{
		status = ASIStartVideoCapture(cameraId);
	} else {
		status = ASI_SUCCESS;
	}

	if (status == ASI_SUCCESS) {
		status = ASIGetVideoData(cameraId, imageBuffer, bufferSize, timeout);
		if (status != ASI_SUCCESS)
		{
			Allsky::Log(0, "  > ERROR: Failed getting image: %s\n", getRetCode(status));
		}
		else
		{
			numErrors = 0;
			Allsky::debugText[0] = '\0';
#ifdef USE_HISTOGRAM
			if (histogram != NULL && mean != NULL)
			{
				*mean = computeHistogram(imageBuffer, width, height, imageType, histogram);
				sprintf(Allsky::debugText, " @ mean %d", *mean);
			}
#endif
			last_exposure_us = exposure_time_us;
			// Per ZWO, when in manual-exposure mode, the returned exposure length should always
			// be equal to the requested length; in fact, "there's no need to call ASIGetControlValue()".
			// When in auto-exposure mode, the returned exposure length is what the driver thinks the
			// next exposure should be, and will eventually converge on the correct exposure.
			ASIGetControlValue(cameraId, ASI_EXPOSURE, &reported_exposure_us, &wasAutoExposure);
			Allsky::Log(3, "  > Got image%s.  Reported exposure: %s, auto=%s\n", Allsky::debugText, Allsky::length_in_units(reported_exposure_us, true), wasAutoExposure == ASI_TRUE ? "yes" : "no");

			// If this was a manual exposure, make sure it took the correct exposure.
			// Per ZWO, this should never happen.
			if (wasAutoExposure == ASI_FALSE && exposure_time_us != reported_exposure_us)
			{
				Allsky::Log(0, "  > WARNING: not correct exposure (requested: %'ld us, reported: %'ld us, diff: %'ld)\n", exposure_time_us, reported_exposure_us, reported_exposure_us - exposure_time_us);
			}
			ASIGetControlValue(cameraId, ASI_GAIN, &actualGain, &bAuto);
			ASIGetControlValue(cameraId, ASI_TEMPERATURE, &Allsky::actualTemp, &bAuto);
		}

		if (Allsky::use_new_exposure_algorithm)
			ASIStopVideoCapture(cameraId);

	}
	else {
		Allsky::Log(0, "  > ERROR: Not fetching exposure data because status is %s\n", getRetCode(status));
	}

	return status;
}

// Exit the program gracefully.
void closeUp(int e)
{
	static int closingUp = 0;		// indicates if we're in the process of exiting.
	// For whatever reason, we're sometimes called twice, but we should only execute once.
	if (closingUp) return;

	closingUp = 1;

	ASIStopVideoCapture(CamNum);

	// Seems to hang on ASICloseCamera() if taking a picture when the signal is sent,
	// until the exposure finishes, then it never returns so the remaining code doesn't
	// get executed.  Don't know a way around that, so don't bother closing the camera.
	// Prior versions of allsky didn't do any cleanup, so it should be ok not to close the camera.
	//    ASICloseCamera(CamNum);

	if (bDisplay)
	{
		bDisplay = 0;
		pthread_join(thread_display, &retval);
	}

	if (bSaveRun)
	{
		bSaveRun = false;
		pthread_mutex_lock(&mtx_SaveImg);
		pthread_cond_signal(&cond_SatrtSave);
		pthread_mutex_unlock(&mtx_SaveImg);
		pthread_join(hthdSave, 0);
	}

	// If we're not on a tty assume we were started by the service.
	// Unfortunately we don't know if the service is stopping us, or restarting us.
	// If it was restarting there'd be no reason to copy a notification image since it
	// will soon be overwritten.  Since we don't know, always copy it.
	if (Allsky::notificationImages) {
		system("scripts/copy_notification_image.sh NotRunning &");
		// Sleep to give it a chance to print any messages so they (hopefully) get printed
		// before the one below.  This is only so it looks nicer in the log file.
		sleep(3);
	}

	printf("     ***** Stopping AllSky *****\n");
	exit(e);
}

void IntHandle(int i)
{
	gotSignal = 1;
	closeUp(0);
}

// Calculate if it is day or night
void calculateDayOrNight(const char *latitude, const char *longitude, const char *angle)
{
	char sunwaitCommand[128];
	sprintf(sunwaitCommand, "sunwait poll angle %s %s %s", angle, latitude, longitude);
	dayOrNight = exec(sunwaitCommand);
	dayOrNight.erase(std::remove(dayOrNight.begin(), dayOrNight.end(), '\n'), dayOrNight.end());

	if (dayOrNight != "DAY" && dayOrNight != "NIGHT")
	{
		sprintf(Allsky::debugText, "*** ERROR: dayOrNight isn't DAY or NIGHT, it's '%s'\n", dayOrNight == "" ? "[empty]" : dayOrNight.c_str());
		Allsky::waitToFix(Allsky::debugText);
		closeUp(2);
	}
}

// Calculate how long until nighttime.
int calculateTimeToNightTime(const char *latitude, const char *longitude, const char *angle)
{
	std::string t;
	char sunwaitCommand[128];	// returns "hh:mm, hh:mm" (sunrise, sunset)
	sprintf(sunwaitCommand, "sunwait list angle %s %s %s | awk '{print $2}'", angle, latitude, longitude);
	t = exec(sunwaitCommand);
	t.erase(std::remove(t.begin(), t.end(), '\n'), t.end());

	int h=0, m=0, secs;
// xxxx TODO: Check - this might be getting time to DAY, not NIGHT
	sscanf(t.c_str(), "%d:%d", &h, &m);
	secs = (h*60*60) + (m*60);

	char *now = getTime("%H:%M");
	int hNow=0, mNow=0, secsNow;
	sscanf(now, "%d:%d", &hNow, &mNow);
	secsNow = (hNow*60*60) + (mNow*60);

	// Handle the (probably rare) case where nighttime is tomorrow
	if (secsNow > secs)
	{
		return(secs + (60*60*24) - secsNow);
	}
	else
	{
		return(secs - secsNow);
	}
}

void writeTemperatureToFile(float val)
{
	std::ofstream outfile;
	outfile.open("temperature.txt", std::ios_base::trunc);
	outfile << val;
	outfile << "\n";
}


bool adjustGain = false;	// Should we adjust the gain?  Set by user on command line.
bool currentAdjustGain = false;	// Adjusting it right now?
int totalAdjustGain = 0;	// The total amount to adjust gain.
int perImageAdjustGain = 0;	// Amount of gain to adjust each image
int gainTransitionImages = 0;
int numGainChanges = 0;		// This is reset at every day/night and night/day transition.

// Reset the gain transition variables for the first transition image.
// This is called when the program first starts and at the beginning of every day/night transition.
// "dayOrNight" is the new value, e.g., if we just transitioned from day to night, it's "NIGHT".
bool resetGainTransitionVariables(int dayGain, int nightGain)
{
	// Many of the "xxx" messages below will go away once we're sure gain transition works.
	Allsky::Log(4, "xxx resetGainTransitionVariables(%d, %d) called at %s\n", dayGain, nightGain, dayOrNight.c_str());

	if (adjustGain == false)
	{
		// determineGainChange() will never be called so no need to set any variables.
		Allsky::Log(5, "xxx will not adjust gain - adjustGain == false\n");
		return(false);
	}

	if (numExposures == 0)
	{
		// we don't adjust when the program first starts since there's nothing to transition from
		Allsky::Log(5, "xxx will not adjust gain right now - numExposures == 0\n");
		return(false);
	}

	// Determine the amount to adjust gain per image.
	// Do this once per day/night or night/day transition (i.e., numGainChanges == 0).
	// First determine how long an exposure and delay is, in seconds.
	// The user specifies the transition period in seconds,
	// but day exposure is in microseconds, night max is in milliseconds,
	// and delays are in milliseconds, so convert to seconds.
	float totalTimeInSec;
	if (dayOrNight == "DAY")
	{
		totalTimeInSec = (Allsky::asi_day_exposure_us / US_IN_SEC) + (Allsky::dayDelay_ms / MS_IN_SEC);
		Allsky::Log(4, "xxx totalTimeInSec=%.1fs, asi_day_exposure_us=%'ldus , daydelay_ms=%'dms\n", totalTimeInSec, Allsky::asi_day_exposure_us, Allsky::dayDelay_ms);
	}
	else	// NIGHT
	{
		// At nightime if the exposure is less than the max, we wait until max has expired,
		// so use it instead of the exposure time.
		totalTimeInSec = (Allsky::asi_night_max_autoexposure_ms / MS_IN_SEC) + (Allsky::nightDelay_ms / MS_IN_SEC);
		Allsky::Log(4, "xxx totalTimeInSec=%.1fs, asi_night_max_autoexposure_ms=%'dms, nightDelay_ms=%'dms\n", totalTimeInSec, Allsky::asi_night_max_autoexposure_ms, Allsky::nightDelay_ms);
	}

	gainTransitionImages = ceil(Allsky::gainTransitionTime / totalTimeInSec);
	if (gainTransitionImages == 0)
	{
		Allsky::Log(0, "*** INFORMATION: Not adjusting gain - your 'gaintransitiontime' (%d seconds) is less than the time to take one image plus its delay (%.1f seconds).\n", Allsky::gainTransitionTime, totalTimeInSec);
		return(false);
	}

	totalAdjustGain = nightGain - dayGain;
	perImageAdjustGain = ceil(totalAdjustGain / gainTransitionImages);	// spread evenly
	if (perImageAdjustGain == 0)
		perImageAdjustGain = totalAdjustGain;
	else
	{
		// Since we can't adust gain by fractions, see if there's any "left over" after gainTransitionImages.
		// For example, if totalAdjustGain is 7 and we're adjusting by 3 each of 2 times,
		// we need an extra transition to get the remaining 1 ((7 - (3 * 2)) == 1).
		if (gainTransitionImages * perImageAdjustGain < totalAdjustGain)
			gainTransitionImages++;		// this one will get the remaining amount
	}

	Allsky::Log(4, "xxx gainTransitionImages=%d, gainTransitionTime=%ds, perImageAdjustGain=%d, totalAdjustGain=%d\n",
		gainTransitionImages, Allsky::gainTransitionTime, perImageAdjustGain, totalAdjustGain);

	return(true);
}

// Determine the change in gain needed for smooth transitions between night and day.
// Gain during the day is usually 0 and at night is usually > 0.
// If auto-exposure is on for both, the first several night frames may be too bright at night
// because of the sudden (often large) increase in gain, or too dark at the night-to-day
// transition.
// Try to mitigate that by changing the gain over several images at each transition.

int determineGainChange(int dayGain, int nightGain)
{
	if (numGainChanges > gainTransitionImages || totalAdjustGain == 0)
	{
		// no more changes needed in this transition
		Allsky::Log(4, "  xxxx No more gain changes needed.\n");
		currentAdjustGain = false;
		return(0);
	}

	numGainChanges++;
	int amt;	// amount to adjust gain on next picture
	if (dayOrNight == "DAY")
	{
		// During DAY, want to start out adding the full gain adjustment minus the increment on the first image,
		// then DECREASE by totalAdjustGain each exposure.
		// This assumes night gain is > day gain.
		amt = totalAdjustGain - (perImageAdjustGain * numGainChanges);
		if (amt < 0)
		{
			amt = 0;
			totalAdjustGain = 0;	// we're done making changes
		}
	}
	else	// NIGHT
	{
		// During NIGHT, want to start out (nightGain-perImageAdjustGain),
		// then DECREASE by perImageAdjustGain each time, until we get to "nightGain".
		// This last image was at dayGain and we wen't to increase each image.
		amt = (perImageAdjustGain * numGainChanges) - totalAdjustGain;
		if (amt > 0)
		{
			amt = 0;
			totalAdjustGain = 0;	// we're done making changes
		}
	}

	Allsky::Log(4, "  xxxx Adjusting %s gain by %d on next picture to %d; will be gain change # %d of %d.\n",
		dayOrNight.c_str(), amt, amt+Allsky::currentGain, numGainChanges, gainTransitionImages);
	return(amt);
}

// Check if the maximum number of consecutive errors has been reached
bool check_max_errors(int *e, int max_errors)
{
	// Once takeOneExposure() fails with a timeout, it seems to always fail,
	// even with extremely large timeout values, so apparently ASI_ERROR_TIMEOUT doesn't
	// necessarily mean it's timing out.  Exit which will cause us to be restarted.
	numErrors++; sleep(2);
	if (numErrors >= max_errors)
	{
		*e = 99;		// exit code - needs to match what's in allsky.sh
		Allsky::Log(0, "*** ERROR: Maximum number of consecutive errors of %d reached; exiting...\n", max_errors);
		return(false);	// gets us out of inner and outer loop
	}
	return(true);
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	Allsky::tty = isatty(fileno(stdout)) ? true : false;
	signal(SIGINT, IntHandle);
	signal(SIGTERM, IntHandle);	// The service sends SIGTERM to end this program.
	pthread_mutex_init(&mtx_SaveImg, 0);
	pthread_cond_init(&cond_SatrtSave, 0);


	char const *bayer[]   = { "RG", "BG", "GR", "GB" };
	bool endOfNight       = false;
	int i;
	ASI_ERROR_CODE asiRetCode;  // used for return code from ASI functions.

	// Some settings have both day and night versions, some have only one version that applies to both,
	// and some have either a day OR night version but not both.
	// For settings with both versions we keep a "current" variable (e.g., "currentBin") that's either the day
	// or night version so the code doesn't always have to check if it's day or night.
	// The settings have either "day" or "night" in the name.
	// In theory, almost every setting could have both day and night versions (e.g., width & height),
	// but the chances of someone wanting different versions.

	// #define the defaults so we can use the same value in the help message.

#define SMALLFONTSIZE_MULTIPLIER 0.08



	// currentAutoExposure is global so is defined outside of main()


#define DEFAULT_ASIDAYGHTGAIN    0
	int asiDayGain             = DEFAULT_ASIDAYGHTGAIN;
	int asiDayAutoGain         = 0;	// is Auto Gain on or off for daytime?

	int currentDelay_ms        = NOT_SET;





#define DEFAULT_SHOWTIME         1
	int showTime               = DEFAULT_SHOWTIME;

#ifdef USE_HISTOGRAM
		int showHistogram      = 0;
	int maxHistogramAttempts   = 15;	// max number of times we'll try for a better histogram mean
	int showHistogramBox       = 0;
	int histogramBoxSizeX      = DEFAULT_BOX_SIZEX;
	int histogramBoxSizeY      = DEFAULT_BOX_SIZEY;

	// If we just transitioned from night to day, it's possible current_exposure_us will
	// be MUCH greater than the daytime max (and will possibly be at the nighttime's max exposure).
	// So, decrease current_exposure_us by a certain amount of the difference between the two so
	// it takes several frames to reach the daytime max (which is now in current_max_autoexposure_us).

	// If we don't do this, we'll be stuck in a loop taking an exposure
	// that's over the max forever.

	// Note that it's likely getting lighter outside with every exposure
	// so the mean will eventually get into the valid range.
#define DEFAULT_PERCENTCHANGE 10.0	// percent of ORIGINAL difference
	const int percent_change = DEFAULT_PERCENTCHANGE;
#endif


	
	//-------------------------------------------------------------------------------------------------------
	Allsky::init(argc, argv);
	//-------------------------------------------------------------------------------------------------------
	const char *imagetype = "";
	const char *ext = strrchr(Allsky::fileName, '.');
	if (strcasecmp(ext + 1, "jpg") == 0 || strcasecmp(ext + 1, "jpeg") == 0)
	{
		if (Allsky::Image_type == ASI_IMG_RAW16)
		{
			Allsky::waitToFix("*** ERROR: RAW16 images only work with .png files; either change the Image Type or the Filename.\n");
			exit(2);
		}

		imagetype = "jpg";
		compression_parameters.push_back(cv::IMWRITE_JPEG_QUALITY);
		// want dark frames to be at highest quality
		if (Allsky::quality > 100 || Allsky::taking_dark_frames)
		{
			Allsky::quality = 100;
		}
		else if (Allsky::quality == NOT_SET)
		{
			Allsky::quality = 95;
		}
	}
	else if (strcasecmp(ext + 1, "png") == 0)
	{
		imagetype = "png";
		compression_parameters.push_back(cv::IMWRITE_PNG_COMPRESSION);
		if (Allsky::taking_dark_frames)
		{
			Allsky::quality = 0;	// actually, it's PNG compression - 0 is highest quality
		}
		else if (Allsky::quality > 9)
		{
			Allsky::quality = 9;
		}
		else if (Allsky::quality == NOT_SET)
		{
			Allsky::quality = 3;
		}
	}
	else
	{
		sprintf(Allsky::debugText, "*** ERROR: Unsupported image extension (%s); only .jpg and .png are supported.\n", ext);
		Allsky::waitToFix(Allsky::debugText);
		exit(100);
	}
	compression_parameters.push_back(Allsky::quality);

	if (Allsky::taking_dark_frames)
	{
		// To avoid overwriting the optional notification inage with the dark image,
		// during dark frames we use a different file name.
		static char darkFilename[200];
		sprintf(darkFilename, "dark.%s", imagetype);
		Allsky::fileName = darkFilename;
	}

	int numDevices = ASIGetNumOfConnectedCameras();
	if (numDevices <= 0)
	{
		printf("*** ERROR: No Connected Camera...\n");
		// Don't wait here since it's possible the camera is physically connected
		// but the software doesn't see it and the USB bus needs to be reset.
		closeUp(1);   // If there are no cameras we can't do anything.
	}

	if (numDevices > 1)
	{
		printf("\nAttached Cameras%s:\n", numDevices == 1 ? "" : " (using first one)");
		for (i = 0; i < numDevices; i++)
		{
			ASIGetCameraProperty(&Allsky::ASICameraInfo, i);
			printf("  - %d %s\n", i, Allsky::ASICameraInfo.Name);
		}
	}
	ASIGetCameraProperty(&Allsky::ASICameraInfo, 0);	// want info on 1st camera

	asiRetCode = ASIOpenCamera(CamNum);
	if (asiRetCode != ASI_SUCCESS)
	{
		printf("*** ERROR opening camera, check that you have root permissions! (%s)\n", getRetCode(asiRetCode));
		closeUp(1);      // Can't do anything so might as well exit.
	}

	int iMaxWidth, iMaxHeight;
	double pixelSize;
	iMaxWidth  = Allsky::ASICameraInfo.MaxWidth;
	iMaxHeight = Allsky::ASICameraInfo.MaxHeight;
	pixelSize  = Allsky::ASICameraInfo.PixelSize;
	if (Allsky::width == 0 || Allsky::height == 0)
	{
		Allsky::width  = iMaxWidth;
		Allsky::height = iMaxHeight;
	}
	Allsky::originalWidth = Allsky::width;
	Allsky::originalHeight = Allsky::height;

#ifdef USE_HISTOGRAM
	int centerX, centerY;
	int left_of_box, right_of_box;
	int top_of_box, bottom_of_box;

	// The histogram box needs to fit on the image.
	// If we're binning we'll decrease the size of the box accordingly.
	bool ok = true;
	if (histogramBoxSizeX < 1 ||  histogramBoxSizeY < 1)
	{
		fprintf(stderr, "%s*** ERROR: Histogram box size must be > 0; you entered X=%d, Y=%d%s\n",
			Allsky::c(KRED), histogramBoxSizeX, histogramBoxSizeY, Allsky::c(KNRM));
		ok = false;
	}
	if (isnan(histogramBoxPercentFromLeft) || isnan(histogramBoxPercentFromTop) || 
		histogramBoxPercentFromLeft < 0.0 || histogramBoxPercentFromTop < 0.0)
	{
		fprintf(stderr, "%s*** ERROR: Bad values for histogram percents; you entered X=%.0f%%, Y=%.0f%%%s\n",
			Allsky::c(KRED), (histogramBoxPercentFromLeft*100.0), (histogramBoxPercentFromTop*100.0), Allsky::c(KNRM));
		ok = false;
	}
	else
	{
		centerX = Allsky::width * histogramBoxPercentFromLeft;
		centerY = Allsky::height * histogramBoxPercentFromTop;
		left_of_box = centerX - (histogramBoxSizeX / 2);
		right_of_box = centerX + (histogramBoxSizeX / 2);
		top_of_box = centerY - (histogramBoxSizeY / 2);
		bottom_of_box = centerY + (histogramBoxSizeY / 2);

		if (left_of_box < 0 || right_of_box >= Allsky::width || top_of_box < 0 || bottom_of_box >= Allsky::height)
		{
			fprintf(stderr, "%s*** ERROR: Histogram box location must fit on image; upper left of box is %dx%d, lower right %dx%d%s\n", Allsky::c(KRED), left_of_box, top_of_box, right_of_box, bottom_of_box, Allsky::c(KNRM));
			ok = false;
		}
	}

	if (! ok)
		exit(100);	// force the user to fix it
#endif

	printf("\n%s Information:\n", Allsky::ASICameraInfo.Name);
	printf("  - Native Resolution: %dx%d\n", iMaxWidth, iMaxHeight);
	printf("  - Pixel Size: %1.1fmicrons\n", pixelSize);
	printf("  - Supported Bins: ");
	for (int i = 0; i < 16; ++i)
	{
		if (Allsky::ASICameraInfo.SupportedBins[i] == 0)
		{
			break;
		}
		printf("%d ", Allsky::ASICameraInfo.SupportedBins[i]);
	}
	printf("\n");

	if (Allsky::ASICameraInfo.IsColorCam)
	{
		printf("  - Color Camera: bayer pattern:%s\n", bayer[Allsky::ASICameraInfo.BayerPattern]);
	}
	else
	{
		printf("  - Mono camera\n");
	}
	if (Allsky::ASICameraInfo.IsCoolerCam)
	{
		printf("  - Camera with cooling capabilities\n");
	}

	printf("\n");
	ASI_ID cameraID;	// USB 3 cameras only
	if (Allsky::ASICameraInfo.IsUSB3Camera == ASI_TRUE && ASIGetID(CamNum, &cameraID) == ASI_SUCCESS)
	{
		printf("  - Camera ID: ");
		if (cameraID.id[0] == '\0')
		{
			printf("[none]");
		} else {
			for (unsigned int i=0; i<sizeof(cameraID.id); i++) printf("%c", cameraID.id[i]);
		}
		printf("\n");
	}
	// To clear the camera ID:
		// cameraID.id[0] = '\0';
		// ASISetID(CamNum, cameraID);
	ASI_SN serialNumber;
	asiRetCode = ASIGetSerialNumber(CamNum, &serialNumber);
	if (asiRetCode != ASI_SUCCESS)
	{
		if (asiRetCode == ASI_ERROR_GENERAL_ERROR)
			printf("Camera does not support serialNumber\n");
		else
			printf("*** WARNING: unable to get serialNumber (%s)\n", getRetCode(asiRetCode));
	}
	else
	{
		printf("  - Camera Serial Number: ");
		if (serialNumber.id[0] == '\0')
		{
			printf("[none]");
		} else {
			for (unsigned int i=0; i<sizeof(serialNumber.id); i++) printf("%02x", serialNumber.id[i]);
		}
		printf("\n");
	}

	asiRetCode = ASIInitCamera(CamNum);
	if (asiRetCode != ASI_SUCCESS)
	{
		printf("*** ERROR: Unable to initialise camera: %s\n", getRetCode(asiRetCode));
		closeUp(1);      // Can't do anything so might as well exit.
	}

	// Get a few values from the camera that we need elsewhere.
	ASIGetNumOfControls(CamNum, &iNumOfCtrl);
	if (Allsky::debugLevel >= 4)
		printf("Control Caps:\n");
	for (i = 0; i < iNumOfCtrl; i++)
	{
		ASIGetControlCaps(CamNum, i, &ControlCaps);
		switch (ControlCaps.ControlType) {
		case ASI_EXPOSURE:
			camera_min_exposure_us = ControlCaps.MinValue;
			break;
#ifdef USE_HISTOGRAM
		case ASI_AUTO_MAX_EXP:
			// Keep track of the camera's max auto-exposure so we don't try to exceed it.
			// MaxValue is in MS so convert to microseconds
			camera_max_autoexposure_us = ControlCaps.MaxValue * US_IN_MS;
			break;
		default:	// needed to keep compiler quiet
			break;
#endif
		}
		if (Allsky::debugLevel >= 4)
		{
			printf("- %s:\n", ControlCaps.Name);
			printf("   - Description = %s\n", ControlCaps.Description);
			printf("   - MinValue = %'ld\n", ControlCaps.MinValue);
			printf("   - MaxValue = %'ld\n", ControlCaps.MaxValue);
			printf("   - DefaultValue = %'ld\n", ControlCaps.DefaultValue);
			printf("   - IsAutoSupported = %d\n", ControlCaps.IsAutoSupported);
			printf("   - IsWritable = %d\n", ControlCaps.IsWritable);
			printf("   - ControlType = %d\n", ControlCaps.ControlType);
		}
	}

	if (Allsky::asi_day_exposure_us < camera_min_exposure_us)
	{
	   	fprintf(stderr, "*** WARNING: daytime exposure %'ld us less than camera minimum of %'ld us; setting to minimum\n", Allsky::asi_day_exposure_us, camera_min_exposure_us);
	   	Allsky::asi_day_exposure_us = camera_min_exposure_us;
	}
	else if (Allsky::asiDayAutoExposure && Allsky::asi_day_exposure_us > camera_max_autoexposure_us)
	{
	   	fprintf(stderr, "*** WARNING: daytime exposure %'ld us greater than camera maximum of %'ld us; setting to maximum\n", Allsky::asi_day_exposure_us, camera_max_autoexposure_us);
	   	Allsky::asi_day_exposure_us = camera_max_autoexposure_us;
	}
	if (Allsky::asi_night_exposure_us < camera_min_exposure_us)
	{
	   	fprintf(stderr, "*** WARNING: nighttime exposure %'ld us less than camera minimum of %'ld us; setting to minimum\n", Allsky::asi_night_exposure_us, camera_min_exposure_us);
	   	Allsky::asi_night_exposure_us = camera_min_exposure_us;
	}
	else if (Allsky::asiNightAutoExposure && Allsky::asi_night_exposure_us > camera_max_autoexposure_us)
	{
	   	fprintf(stderr, "*** WARNING: nighttime exposure %'ld us greater than camera maximum of %'ld us; setting to maximum\n", Allsky::asi_night_exposure_us, camera_max_autoexposure_us);
	   	Allsky::asi_night_exposure_us = camera_max_autoexposure_us;
	}

	if (Allsky::debugLevel >= 4)
	{
		printf("Supported video formats:\n");
		for (i = 0; i < 8; i++)
		{
			ASI_IMG_TYPE it = Allsky::ASICameraInfo.SupportedVideoFormat[i];
			if (it == ASI_IMG_END)
			{
				break;
			}
			printf("   - %s\n",
				it == ASI_IMG_RAW8 ?  "ASI_IMG_RAW8" :
				it == ASI_IMG_RGB24 ?  "ASI_IMG_RGB24" :
				it == ASI_IMG_RAW16 ?  "ASI_IMG_RAW16" :
				it == ASI_IMG_Y8 ?  "ASI_IMG_Y8" :
				"unknown video format");
		}
	}

	ASIGetControlValue(CamNum, ASI_TEMPERATURE, &Allsky::actualTemp, &bAuto);
	printf("- Sensor temperature: %0.2f\n", (float)Allsky::actualTemp / 10.0);

	// Handle "auto" Image_type.
	if (Allsky::Image_type == AUTO_IMAGE_TYPE)
	{
		// If it's a color camera, create color pictures.
		// If it's a mono camera use RAW16 if the image file is a .png, otherwise use RAW8.
		// There is no good way to handle Y8 automatically so it has to be set manually.
		if (Allsky::ASICameraInfo.IsColorCam)
			Allsky::Image_type = ASI_IMG_RGB24;
		else if (strcmp(imagetype, "png") == 0)
			Allsky::Image_type = ASI_IMG_RAW16;
		else // jpg
			Allsky::Image_type = ASI_IMG_RAW8;
	}

	
	if (Allsky::Image_type == ASI_IMG_RAW16)
	{
		Allsky::sType = "ASI_IMG_RAW16";
	}
	else if (Allsky::Image_type == ASI_IMG_RGB24)
	{
		Allsky::sType = "ASI_IMG_RGB24";
	}
	else if (Allsky::Image_type == ASI_IMG_RAW8)
	{
		// Color cameras should use Y8 instead of RAW8.  Y8 is the mono mode for color cameras.
		if (Allsky::ASICameraInfo.IsColorCam)
		{
			Allsky::Image_type = ASI_IMG_Y8;
			Allsky::sType = "ASI_IMG_Y8 (not RAW8 for color cameras)";
		}
		else
		{
			Allsky::sType = "ASI_IMG_RAW8";
		}
	}
	else if (Allsky::Image_type == ASI_IMG_RAW8)
	{
		Allsky::sType = "ASI_IMG_Y8";
	}
	else
	{
		sprintf(Allsky::debugText, "*** ERROR: ASI_IMG_TYPE: %d\n", Allsky::Image_type);
		Allsky::waitToFix(Allsky::debugText);
		exit(100);
	}

	//-------------------------------------------------------------------------------------------------------
	Allsky::info();
	//-------------------------------------------------------------------------------------------------------


	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	// These configurations apply to both day and night.
	// Other calls to setControl() are done after we know if we're in daytime or nighttime.
	setControl(CamNum, ASI_BANDWIDTHOVERLOAD, Allsky::asiBandwidth, Allsky::asiAutoBandwidth == 1 ? ASI_TRUE : ASI_FALSE);
	setControl(CamNum, ASI_HIGH_SPEED_MODE, 0, ASI_FALSE);  // ZWO sets this in their program
	if (Allsky::ASICameraInfo.IsColorCam)
	{
		setControl(CamNum, ASI_WB_R, Allsky::asiWBR, Allsky::asiAutoAWB == 1 ? ASI_TRUE : ASI_FALSE);
		setControl(CamNum, ASI_WB_B, Allsky::asiWBB, Allsky::asiAutoAWB == 1 ? ASI_TRUE : ASI_FALSE);
	}
	setControl(CamNum, ASI_GAMMA, Allsky::asiGamma, ASI_FALSE);
	setControl(CamNum, ASI_FLIP, Allsky::asiFlip, ASI_FALSE);

	if (Allsky::ASICameraInfo.IsCoolerCam)
	{
		asiRetCode = setControl(CamNum, ASI_COOLER_ON, Allsky::asiCoolerEnabled == 1 ? ASI_TRUE : ASI_FALSE, ASI_FALSE);
		if (asiRetCode != ASI_SUCCESS)
		{
			printf("%s", Allsky::c(KRED));
			printf(" WARNING: Could not enable cooler: %s, but continuing without it.\n", getRetCode(asiRetCode));
			printf("%s", Allsky::c(KNRM));
		}
		asiRetCode = setControl(CamNum, ASI_TARGET_TEMP, Allsky::asiTargetTemp, ASI_FALSE);
		if (asiRetCode != ASI_SUCCESS)
		{
			printf("%s", Allsky::c(KRED));
			printf(" WARNING: Could not set cooler temperature: %s, but continuing without it.\n", getRetCode(asiRetCode));
			printf("%s", Allsky::c(KNRM));
		}
	}

	if (! bSaveRun && pthread_create(&hthdSave, 0, SaveImgThd, 0) == 0)
	{
		bSaveRun = true;
	}

	// Initialization
	int exitCode        = 0;    // Exit code for main()
	int maxErrors       = 5;    // Max number of errors in a row before we exit
	int originalITextX = Allsky::iTextX;
	int originalITextY = Allsky::iTextY;
	int originalFontsize = Allsky::fontsize;
	int originalLinewidth = Allsky::linewidth;
	// Have we displayed "not taking picture during day" message, if applicable?
	int displayedNoDaytimeMsg = 0;
	int gainChange = 0;			// how much to change gain up or down

	// Display one-time messages.

	// If autogain is on, our adjustments to gain will get overwritten by the camera
	// so don't transition.
	// gainTransitionTime of 0 means don't adjust gain.
	// No need to adjust gain if day and night gain are the same.
	if (asiDayAutoGain == 1 || Allsky::asiNightAutoGain == 1 || Allsky::gainTransitionTime == 0 || asiDayGain == Allsky::asiNightGain || Allsky::taking_dark_frames == 1)
	{
		adjustGain = false;
		Allsky::Log(3, "Will NOT adjust gain at transitions\n");
	}
	else
	{
		adjustGain = true;
		Allsky::Log(3, "Will adjust gain at transitions\n");
	}

	if (Allsky::ImgExtraText[0] != '\0' && Allsky::extraFileAge > 0) {
		Allsky::Log(1, "Extra Text File Age Disabled So Displaying Anyway\n");
	}

	if (Allsky::tty)
		printf("*** Press Ctrl+C to stop ***\n\n");
	else
		printf("*** Stop the allsky service to end this process. ***\n\n");


	// Start taking pictures

	if (! Allsky::use_new_exposure_algorithm)
	{
		asiRetCode = ASIStartVideoCapture(CamNum);
		if (asiRetCode != ASI_SUCCESS)
		{
			printf("*** ERROR: Unable to start video capture: %s\n", getRetCode(asiRetCode));
			closeUp(2);
		}
	}

	while (bMain)
	{
		std::string lastDayOrNight;

		// Find out if it is currently DAY or NIGHT
		calculateDayOrNight(Allsky::latitude, Allsky::longitude, Allsky::angle);

		if (! Allsky::taking_dark_frames)
			currentAdjustGain = resetGainTransitionVariables(asiDayGain, Allsky::asiNightGain);

		lastDayOrNight = dayOrNight;
		if (Allsky::taking_dark_frames)
		{
				// We're doing dark frames so turn off autoexposure and autogain, and use
				// nightime gain, delay, max exposure, bin, and brightness to mimic a nightime shot.
				Allsky::currentAutoExposure = ASI_FALSE;
				Allsky::asiNightAutoExposure = 0;
				Allsky::currentAutoGain = ASI_FALSE;
				// Don't need to set ASI_AUTO_MAX_GAIN since we're not using auto gain
				Allsky::currentGain = Allsky::asiNightGain;
				gainChange = 0;
				currentDelay_ms = Allsky::nightDelay_ms;
				Allsky::current_max_autoexposure_us = current_exposure_us = Allsky::asi_night_max_autoexposure_ms * US_IN_MS;
				Allsky::currentBin = Allsky::nightBin;
				Allsky::currentBrightness = Allsky::asiNightBrightness;

				Allsky::Log(0, "Taking dark frames...\n");

				if (Allsky::notificationImages) {
					system("scripts/copy_notification_image.sh DarkFrames &");
				}
		}

		else if (dayOrNight == "DAY")
		{
			// Setup the daytime capture parameters
			if (endOfNight == true)	// Execute end of night script
			{
				Allsky::Log(0, "Processing end of night data\n");
				system("scripts/endOfNight.sh &");
				endOfNight = false;
				displayedNoDaytimeMsg = 0;
			}

			if (Allsky::daytimeCapture != 1)
			{
				// Only display messages once a day.
				if (displayedNoDaytimeMsg == 0) {
					if (Allsky::notificationImages) {
						system("scripts/copy_notification_image.sh CameraOffDuringDay &");
					}
					Allsky::Log(0, "It's daytime... we're not saving images.\n*** %s ***\n",
						Allsky::tty ? "Press Ctrl+C to stop" : "Stop the allsky service to end this process.");
					displayedNoDaytimeMsg = 1;

					// sleep until almost nighttime, then wake up and sleep a short time
					int secsTillNight = calculateTimeToNightTime(Allsky::latitude, Allsky::longitude, Allsky::angle);
					sleep(secsTillNight - 10);
				}
				else
				{
					// Shouldn't need to sleep more than a few times before nighttime.
					sleep(5);
				}

				// No need to do any of the code below so go back to the main loop.
				continue;
			}

			else
			{
				Allsky::Log(0, "==========\n=== Starting daytime capture ===\n==========\n");

				// We only skip initial frames if we are starting in daytime and using auto-exposure.
				if (numExposures == 0 && Allsky::asiDayAutoExposure)
					Allsky::current_skip_frames = Allsky::day_skip_frames;

				// If we went from Night to Day, then current_exposure_us will be the last night
				// exposure so leave it if we're using auto-exposure so there's a seamless change from
				// Night to Day, i.e., if the exposure was fine a minute ago it will likely be fine now.
				// On the other hand, if this program just started or we're using manual exposures,
				// use what the user specified.
				if (numExposures == 0 || ! Allsky::asiDayAutoExposure)
				{
					if (Allsky::asiDayAutoExposure && Allsky::asi_day_exposure_us > Allsky::asi_day_max_autoexposure_ms*US_IN_MS)
					{
						snprintf(Allsky::bufTemp, sizeof(Allsky::bufTemp), "%s", Allsky::length_in_units(Allsky::asi_day_exposure_us, true));
						Allsky::Log(0, "*** WARNING: daytime Manual Exposure [%s] > Max Auto-Exposure [%s]; user smaller number.\n", Allsky::bufTemp, Allsky::length_in_units(Allsky::asi_day_max_autoexposure_ms*US_IN_MS, true));
						Allsky::asi_day_exposure_us = Allsky::asi_day_max_autoexposure_ms * US_IN_MS;
					}
					current_exposure_us = Allsky::asi_day_exposure_us;
				}
				else
				{
					Allsky::Log(2, "Using the last night exposure of %s\n", Allsky::length_in_units(current_exposure_us, true));
				}

				Allsky::current_max_autoexposure_us = Allsky::asi_day_max_autoexposure_ms * US_IN_MS;
#ifdef USE_HISTOGRAM
				// Don't use camera auto-exposure since we mimic it ourselves.
				if (Allsky::asiDayAutoExposure)
				{
					Allsky::Log(2, "Turning off daytime auto-exposure to use histogram exposure.\n");
				}
				// With the histogram method we NEVER use auto exposure - either the user said
				// not to, or we turn it off ourselves.
				Allsky::currentAutoExposure = ASI_FALSE;
#else
				Allsky::currentAutoExposure = Allsky::asiDayAutoExposure ? ASI_TRUE : ASI_FALSE;
#endif
				Allsky::currentBrightness = Allsky::asiDayBrightness;
				currentDelay_ms = Allsky::dayDelay_ms;
				Allsky::currentBin = Allsky::dayBin;
				Allsky::currentGain = asiDayGain;	// must come before determineGainChange() below
				if (currentAdjustGain)
				{
					// we did some nightime images so adjust gain
					numGainChanges = 0;
					gainChange = determineGainChange(asiDayGain, Allsky::asiNightGain);
				}
				else
				{
					gainChange = 0;
				}
				Allsky::currentAutoGain = asiDayAutoGain ? ASI_TRUE : ASI_FALSE;
// xxxx TODO: add asiDayMaxGain and currentMaxGain.
// xxxx then can move the setControl further below
				// We don't have a separate asiDayMaxGain, so set to night one
				setControl(CamNum, ASI_AUTO_MAX_GAIN, Allsky::asiNightMaxGain, ASI_FALSE);
			}
		}

		else	// NIGHT
		{
			Allsky::Log(0, "==========\n=== Starting nighttime capture ===\n==========\n");

			// We only skip initial frames if we are starting in nighttime and using auto-exposure.
			if (numExposures == 0 && Allsky::asiNightAutoExposure)
				Allsky::current_skip_frames = Allsky::night_skip_frames;

			// Setup the night time capture parameters
			if (numExposures == 0 || Allsky::asiNightAutoExposure == ASI_FALSE)
			{
				if (Allsky::asiNightAutoExposure && Allsky::asi_night_exposure_us > Allsky::asi_night_max_autoexposure_ms*US_IN_MS)
				{
					snprintf(Allsky::bufTemp, sizeof(Allsky::bufTemp), "%s", Allsky::length_in_units(Allsky::asi_night_exposure_us, true));
					Allsky::Log(0, "*** WARNING: nighttime Manual Exposure [%s] > Max Auto-Exposure [%s]; user smaller number.\n", Allsky::bufTemp, Allsky::length_in_units(Allsky::asi_night_max_autoexposure_ms*US_IN_MS, true));
					Allsky::asi_night_exposure_us = Allsky::asi_night_max_autoexposure_ms * US_IN_MS;
				}
				current_exposure_us = Allsky::asi_night_exposure_us;
				Allsky::Log(4, "Using night exposure (%s)\n", Allsky::length_in_units(Allsky::asi_night_exposure_us, true));
			}

			Allsky::currentAutoExposure = Allsky::asiNightAutoExposure ? ASI_TRUE : ASI_FALSE;
			Allsky::currentBrightness = Allsky::asiNightBrightness;
			currentDelay_ms = Allsky::nightDelay_ms;
			Allsky::currentBin = Allsky::nightBin;
			Allsky::current_max_autoexposure_us = Allsky::asi_night_max_autoexposure_ms * US_IN_MS;
			Allsky::currentGain = Allsky::asiNightGain;	// must come before determineGainChange() below
			if (currentAdjustGain)
			{
				// we did some daytime images so adjust gain
				numGainChanges = 0;
				gainChange = determineGainChange(asiDayGain, Allsky::asiNightGain);
			}
			else
			{
				gainChange = 0;
			}
			Allsky::currentAutoGain = Allsky::asiNightAutoGain ? ASI_TRUE : ASI_FALSE;
			setControl(CamNum, ASI_AUTO_MAX_GAIN, Allsky::asiNightMaxGain, ASI_FALSE);
		}

		// never go over the camera's max auto exposure.  ASI_AUTO_MAX_EXP is in ms so convert
		Allsky::current_max_autoexposure_us = std::min(Allsky::current_max_autoexposure_us, camera_max_autoexposure_us);
		setControl(CamNum, ASI_AUTO_MAX_EXP, Allsky::current_max_autoexposure_us / US_IN_MS, ASI_FALSE);
		setControl(CamNum, ASI_GAIN, Allsky::currentGain + gainChange, (ASI_BOOL)Allsky::currentAutoGain);
		// ASI_BRIGHTNESS is also called ASI_OFFSET
		setControl(CamNum, ASI_BRIGHTNESS, Allsky::currentBrightness, ASI_FALSE);

#ifndef USE_HISTOGRAM
		setControl(CamNum, ASI_EXPOSURE, current_exposure_us, (ASI_BOOL)Allsky::currentAutoExposure);
#endif

		if (numExposures == 0 || Allsky::dayBin != Allsky::nightBin)
		{
			// Adjusting variables for chosen binning.
			// Only need to do at the beginning and if bin changes.
			Allsky::height    = Allsky::originalHeight / Allsky::currentBin;
			Allsky::width     = Allsky::originalWidth / Allsky::currentBin;
			Allsky::iTextX    = originalITextX / Allsky::currentBin;
			Allsky::iTextY    = originalITextY / Allsky::currentBin;
			Allsky::fontsize  = originalFontsize / Allsky::currentBin;
			Allsky::linewidth = originalLinewidth / Allsky::currentBin;
			current_histogramBoxSizeX = histogramBoxSizeX / Allsky::currentBin;
			current_histogramBoxSizeY = histogramBoxSizeY / Allsky::currentBin;

			bufferSize = Allsky::width * Allsky::height * bytesPerPixel((ASI_IMG_TYPE) Allsky::Image_type);
			Allsky::Log(4, "Buffer size: %ld\n", bufferSize);

// TODO: if not the first time, should we free the old pRgb?
			if (Allsky::Image_type == ASI_IMG_RAW16)
			{
				Allsky::pRgb.create(cv::Size(Allsky::width, Allsky::height), CV_16UC1);
			}
			else if (Allsky::Image_type == ASI_IMG_RGB24)
			{
				Allsky::pRgb.create(cv::Size(Allsky::width, Allsky::height), CV_8UC3);
			}
			else // RAW8 and Y8
			{
				Allsky::pRgb.create(cv::Size(Allsky::width, Allsky::height), CV_8UC1);
			}

// TODO: ASISetStartPos(CamNum, from_left_xxx, from_top_xxx);
			asiRetCode = ASISetROIFormat(CamNum, Allsky::width, Allsky::height, Allsky::currentBin, (ASI_IMG_TYPE)Allsky::Image_type);
			if (asiRetCode != ASI_SUCCESS)
			{
				if (asiRetCode == ASI_ERROR_INVALID_SIZE)
				{
					printf("*** ERROR: your camera does not support bin %dx%d.\n", Allsky::currentBin, Allsky::currentBin);
					closeUp(100);	// user needs to fix
				}
				else
				{
					printf("ASISetROIFormat(%d, %dx%d, %d, %d) = %s\n", CamNum, Allsky::width, Allsky::height, Allsky::currentBin, Allsky::Image_type, getRetCode(asiRetCode));
					closeUp(100);
				}
			}
		}

		// Here and below, indent sub-messages with "  > " so it's clear they go with the un-indented line.
		// This simply makes it easier to see things in the log file.

#ifdef USE_HISTOGRAM
		int mean = 0;
		int attempts = 0;
		int histogram[256];
#endif

		while (bMain && lastDayOrNight == dayOrNight)
		{
			// date/time is added to many log entries to make it easier to associate them
			// with an image (which has the date/time in the filename).
			timeval t;
			t = getTimeval();
			char exposureStart[128];
			char f[10] = "%F %T";
			sprintf(exposureStart, "%s", formatTime(t, f));
			Allsky::Log(0, "STARTING EXPOSURE at: %s   @ %s\n", exposureStart, Allsky::length_in_units(current_exposure_us, true));

			// Get start time for overlay.  Make sure it has the same time as exposureStart.
			if (showTime == 1)
				sprintf(Allsky::bufTime, "%s", formatTime(t, Allsky::timeFormat));

			asiRetCode = takeOneExposure(CamNum, current_exposure_us, Allsky::pRgb.data, Allsky::width, Allsky::height, (ASI_IMG_TYPE) Allsky::Image_type, histogram, &mean);
			if (asiRetCode == ASI_SUCCESS)
			{
				numErrors = 0;
				numExposures++;

				if (numExposures == 0 && Allsky::preview == 1)
				{
					// Start the preview thread at the last possible moment.
					bDisplay = 1;
					pthread_create(&thread_display, NULL, Display, (void *)&Allsky::pRgb);
				}

#ifdef USE_HISTOGRAM
				int usedHistogram = 0;	// did we use the histogram method?

				// We don't use this at night since the ZWO bug is only when it's light outside.
				if (dayOrNight == "DAY" && Allsky::asiDayAutoExposure && ! Allsky::taking_dark_frames)
				{
					usedHistogram = 1;	// we are using the histogram code on this exposure
					attempts = 0;

					// Got these by trial and error.  They are more-or-less half the max of 255.
#define MINMEAN 122
#define MAXMEAN 134
					int minAcceptableMean = MINMEAN;
					int maxAcceptableMean = MAXMEAN;
					int roundToMe = 5; // round exposures to this many microseconds

					long new_exposure_us = 0;

					// camera_min_exposure_us is a camera property.
					// hist_min_exposure_us is the min exposure used in the histogram calculation.
// xxxxxxxxx dump hist_min_exposure_us?  Set temp_min_exposure_us = camera_min_exposure_us ? ...
					long hist_min_exposure_us = camera_min_exposure_us ? camera_min_exposure_us : 100;
					long temp_min_exposure_us = hist_min_exposure_us;
					long temp_max_exposure_us = Allsky::current_max_autoexposure_us;

					if (Allsky::asiDayBrightness != DEFAULT_BRIGHTNESS)
					{
						// Adjust brightness based on asiDayBrightness.
						// The default value has no adjustment.
						// The only way we can do this easily is via adjusting the exposure.
						// We could apply a stretch to the image, but that's more difficult.
						// Sure would be nice to see how ZWO handles this variable.
						// We asked but got a useless reply.
						// Values below the default make the image darker; above make it brighter.

						float exposureAdjustment = 1.0;

						// Adjustments of DEFAULT_BRIGHTNESS up or down make the image this much darker/lighter.
						// Don't want the max brightness to give pure white.
						//xxx May have to play with this number, but it seems to work ok.
						// 100 * this number is the percent to change.
						const float adjustmentAmountPerMultiple = 0.12;

						// The amount doesn't change after being set, so only display once.
						static int showedMessage = 0;
						if (showedMessage == 0)
						{
							float numMultiples;

							// Determine the adjustment amount - only done once.
							// See how many multiples we're different.
							// If asiDayBrightnes < DEFAULT_BRIGHTNESS then numMultiples will be negative,
							// which is ok - it just means the multiplier will be less than 1.
							numMultiples = (float)(Allsky::asiDayBrightness - DEFAULT_BRIGHTNESS) / DEFAULT_BRIGHTNESS;
							exposureAdjustment = 1 + (numMultiples * adjustmentAmountPerMultiple);
							Allsky::Log(3, "  > >>> Adjusting exposure x %.2f (%.1f%%) for daybrightness\n", exposureAdjustment, (exposureAdjustment - 1) * 100);
							showedMessage = 1;
						}

						// Now adjust the variables
// xxxxxxxxx TODO: don't adjust hist_min_exposure_us; just histogram numbers.
						hist_min_exposure_us *= exposureAdjustment;
						minAcceptableMean *= exposureAdjustment;
						maxAcceptableMean *= exposureAdjustment;
					}

					std::string why;	// Why did we adjust the exposure?  For debugging

					// Keep track of whether or not we're bouncing around, for example,
					// one exposure is less than the min and the second is greater than the max.
					// When that happens we don't want to set the min to the second exposure
					// or else we'll never get low enough.
					// Negative is below lower limit, positive is above upper limit.
					// Adjust the min or maxAcceptableMean depending on the aggression.
					int prior_mean = mean;
					int prior_mean_diff = 0;
					int adjustment = 0;

					int last_mean_diff = 0;	// like prior_mean_diff but for next exposure

					if (mean < minAcceptableMean)
					{
						prior_mean_diff = mean - minAcceptableMean;
						// If we're skipping frames we want to get to a good exposure as fast as
						// possible so don't set a adjustment.
						if (Allsky::aggression != 100 && Allsky::current_skip_frames <= 0)
						{
							adjustment = prior_mean_diff * (1 - ((float)Allsky::aggression/100));
							if (adjustment < 1)
								minAcceptableMean += adjustment;
						}
					}
					else if (mean > maxAcceptableMean)
					{
						prior_mean_diff = mean - maxAcceptableMean;
#ifdef DO_NOT_USE_OVER_MAX	// xxxxx this allows the image to get too bright
						if (Allsky::aggression != 100 && Allsky::current_skip_frames <= 0)
						{
							adjustment = prior_mean_diff * (1 - ((float)Allsky::aggression/100));
							if (adjustment > 1)
								maxAcceptableMean += adjustment;
						}
#endif
					}
					if (adjustment != 0)
					{
						Allsky::Log(3, "  > !!! Adjusting %sAcceptableMean by %d to %d\n",
						   adjustment < 0 ? "min" : "max",
						   adjustment,
						   adjustment < 0 ? minAcceptableMean : maxAcceptableMean);
					}

					while ((mean < minAcceptableMean || mean > maxAcceptableMean) && ++attempts <= maxHistogramAttempts && current_exposure_us <= Allsky::current_max_autoexposure_us)
					{
						why = "";
						int num = 0;

						//  The code below looks at how far off we are from an acceptable mean.
						//  There's probably a better way to do this, like adjust by some multiple
						//  of how far of we are.  That exercise is left to the reader...
						 if (mean < (minAcceptableMean * 0.04))
						 {
							 // The cameras don't appear linear at this low of a level,
							 // so really crank it up to get into the linear area.
							 new_exposure_us = current_exposure_us * 25;
							 why = "< (minAcceptableMean * 0.04)";
							 num = minAcceptableMean * 0.04;
						 }
						 else if (mean < (minAcceptableMean * 0.1))
						 {
							 new_exposure_us = current_exposure_us * 7;
							 why = "< (minAcceptableMean * 0.1)";
							 num = minAcceptableMean * 0.1;
						 }
						 else if (mean < (minAcceptableMean * 0.3))
						 {
							 new_exposure_us = current_exposure_us * 4;
							 why = "< (minAcceptableMean * 0.3)";
							 num = minAcceptableMean * 0.3;
						 }
						 else if (mean < (minAcceptableMean * 0.6))
						 {
							 new_exposure_us = current_exposure_us * 2.5;
							 why = "< (minAcceptableMean * 0.6)";
							 num = minAcceptableMean * 0.6;
						 }
						 else if (mean < (minAcceptableMean * 0.8))
						 {
							 new_exposure_us = current_exposure_us * 1.8;
							 why = "< (minAcceptableMean * 0.8)";
							 num = minAcceptableMean * 0.8;
						 }
						 else if (mean < (minAcceptableMean * 1.0))
						 {
							 new_exposure_us = current_exposure_us * 1.05;
							 why = "< minAcceptableMean";
							 num = minAcceptableMean * 1.0;
						 }


						 else if (mean > (maxAcceptableMean * 1.89))
						 {
							 new_exposure_us = current_exposure_us * 0.4;
							 why = "> (maxAcceptableMean * 1.89)";
							 num = (maxAcceptableMean * 1.89);
						 }
						 else if (mean > (maxAcceptableMean * 1.6))
						 {
							 new_exposure_us = current_exposure_us * 0.7;
							 why = "> (maxAcceptableMean * 1.6)";
							 num = (maxAcceptableMean * 1.6);
						 }
						 else if (mean > (maxAcceptableMean * 1.3))
						 {
							 new_exposure_us = current_exposure_us * 0.85;
							 why = "> (maxAcceptableMean * 1.3)";
							 num = (maxAcceptableMean * 1.3);
						 }
						 else if (mean > (maxAcceptableMean * 1.0))
						 {
							 new_exposure_us = current_exposure_us * 0.9;
							 why = "> maxAcceptableMean";
							 num = maxAcceptableMean;
						 }

// xxxxxxxxxxxxxxxx test new formula-based method
long new_new_exposure_us;
int acceptable;
float multiplier = 1.10;
const char *acceptable_type;
if (mean < minAcceptableMean) {
	acceptable = minAcceptableMean;
	acceptable_type = "min";
} else {
	acceptable = maxAcceptableMean;
	acceptable_type = "max";
	multiplier = 1 / multiplier;
}
long e_us;
e_us = current_exposure_us;
e_us = last_exposure_us;
if (current_exposure_us != last_exposure_us) printf("xxxxxxxxxxx current_exposure_us %'ld != last_exposure_us %'ld\n", current_exposure_us, last_exposure_us);
// if mean/acceptable is 9/90, it's 1/10th of the way there, so multiple exposure by 90/9 (10).
// ZWO cameras don't appear to be linear so increase the multiplier amount some.
float multiply = ((double)acceptable / mean) * multiplier;
new_new_exposure_us= e_us * multiply;
printf("=== next exposure: old way=%'ld, new way=%'ld (multiply by %.3f) [last_exposure_us=%'ld, %sAcceptable=%d, mean=%d]\n", roundTo(new_exposure_us, roundToMe), new_new_exposure_us, multiply, e_us, acceptable_type, acceptable, mean);
new_exposure_us = new_new_exposure_us;	// use new way

						if (prior_mean_diff > 0 && last_mean_diff < 0)
						{ 
printf(" >xxx mean was %d and went from %d above max of %d to %d below min of %d, is now at %d; should NOT set temp min to current_exposure_us of %'ld\n",
							prior_mean, prior_mean_diff, maxAcceptableMean,
							-last_mean_diff, minAcceptableMean, mean, current_exposure_us);
						} 
						else
						{
							if (prior_mean_diff < 0 && last_mean_diff > 0)
							{
							// OK to set upper limit since we know it's too high.
printf(" >xxx mean was %d and went from %d below min of %d to %d above max of %d, is now at %d; OK to set temp max to current_exposure_us of %'ld\n",
								prior_mean, -prior_mean_diff, minAcceptableMean,
								last_mean_diff, maxAcceptableMean, mean, current_exposure_us);
							}

							if (mean < minAcceptableMean)
							{
								temp_min_exposure_us = current_exposure_us;
							} 
							else if (mean > maxAcceptableMean)
							{
								temp_max_exposure_us = current_exposure_us;
							} 
						} 

						 new_exposure_us = roundTo(new_exposure_us, roundToMe);
						 new_exposure_us = std::max(temp_min_exposure_us, new_exposure_us);
						 new_exposure_us = std::min(temp_max_exposure_us, new_exposure_us);
						 new_exposure_us = std::min(Allsky::current_max_autoexposure_us, new_exposure_us);

						 if (new_exposure_us == current_exposure_us)
						 {
							 break;
						 }

						 current_exposure_us = new_exposure_us;
						 if (current_exposure_us > Allsky::current_max_autoexposure_us)
						 {
							 break;
						 }

						 Allsky::Log(3, "  >> Retry %i @ %'ld us, min=%'ld us, max=%'ld us: mean (%d) %s (%d)\n", attempts, new_exposure_us, temp_min_exposure_us, temp_max_exposure_us, mean, why.c_str(), num);

						 prior_mean = mean;
						 prior_mean_diff = last_mean_diff;

						 asiRetCode = takeOneExposure(CamNum, current_exposure_us, Allsky::pRgb.data, Allsky::width, Allsky::height, (ASI_IMG_TYPE) Allsky::Image_type, histogram, &mean);
						 if (asiRetCode == ASI_SUCCESS)
						 {

							if (mean < minAcceptableMean)
								last_mean_diff = mean - minAcceptableMean;
							else if (mean > maxAcceptableMean)
								last_mean_diff = mean - maxAcceptableMean;
							else
								last_mean_diff = 0;

							continue;
						 }
						 else
						 {
							// Check if we reached the maximum number of consective errors
							if (! check_max_errors(&exitCode, maxErrors))
							{
								closeUp(exitCode);
							}
							break;
						 }
					} // end of "Retry" loop

					if (asiRetCode != ASI_SUCCESS)
					{
						Allsky::Log(2,"  > Sleeping %s from failed exposure\n", Allsky::length_in_units(currentDelay_ms * US_IN_MS, false));
						usleep(currentDelay_ms * US_IN_MS);
						// Don't save the file or do anything below.
						continue;
					}

					if (mean >= minAcceptableMean && mean <= maxAcceptableMean)
					{
						// +++ at end makes it easier to see in log file
						Allsky::Log(3, "  > Good image: mean within range of %d to %d ++++++++++, mean %d\n", minAcceptableMean, maxAcceptableMean, mean);
					}
					else if (attempts > maxHistogramAttempts)
					{
						 Allsky::Log(3, "  > max attempts reached - using exposure of %s us with mean %d\n", Allsky::length_in_units(current_exposure_us, true), mean);
					}
					else if (attempts >= 1)
					{
						 if (current_exposure_us > Allsky::current_max_autoexposure_us)
						 {
							 Allsky::Log(3, "  > Stopped trying: new exposure of %s would be over max of %s\n", Allsky::length_in_units(current_exposure_us, false), Allsky::length_in_units(Allsky::current_max_autoexposure_us, false));

							 long diff = (long)((float)current_exposure_us * (1/(float)percent_change));
							 current_exposure_us -= diff;
							 Allsky::Log(3, "  > Decreasing next exposure by %d%% (%'ld us) to %'ld\n", percent_change, diff, current_exposure_us);
						 }
						 else if (current_exposure_us == Allsky::current_max_autoexposure_us)
						 {
							 Allsky::Log(3, "  > Stopped trying: hit max exposure limit of %s, mean %d\n", Allsky::length_in_units(Allsky::current_max_autoexposure_us, false), mean);
							 // If current_exposure_us causes too high of a mean, decrease exposure
							 // so on the next loop we'll adjust it.
							 if (mean > maxAcceptableMean)
								 current_exposure_us--;
						 }
						 else if (new_exposure_us == current_exposure_us)
						 {
							 Allsky::Log(3, "  > Stopped trying: new_exposure_us == current_exposure_us == %s\n", Allsky::length_in_units(current_exposure_us, false));
						 }
						 else
						 {
							 Allsky::Log(3, "  > Stopped trying, using exposure of %s us with mean %d, min=%d, max=%d\n", Allsky::length_in_units(current_exposure_us, false), mean, minAcceptableMean, maxAcceptableMean);
						 }
					}
					else if (current_exposure_us == Allsky::current_max_autoexposure_us)
					{
						 Allsky::Log(3, "  > Did not make any additional attempts - at max exposure limit of %s, mean %d\n", Allsky::length_in_units(Allsky::current_max_autoexposure_us, false), mean);
					}
					// xxxx TODO: this was "actual_exposure_us = ..."    reported_exposure_us = current_exposure_us;

				} else {
					// Didn't use histogram method.
					// If we used auto-exposure, set the next exposure to the last reported exposure, which is what.
					// the camera driver thinks the next exposure should be.
					if (Allsky::currentAutoExposure == ASI_TRUE)
						current_exposure_us = reported_exposure_us;
					else
						current_exposure_us = last_exposure_us;
				}
#endif
				if (Allsky::current_skip_frames > 0)
				{
#ifdef USE_HISTOGRAM
					// If we're already at a good exposure, or the last exposure was longer
					// than the max, don't skip any more frames.
// xxx TODO: should we have a separate variable to define "too long" instead of current_max_autoexposure_us?
					if ((mean >= MINMEAN && mean <= MAXMEAN) || last_exposure_us > Allsky::current_max_autoexposure_us)
					{
						Allsky::current_skip_frames = 0;
					}
					else
#endif
					{
						Allsky::Log(2, "  >>>> Skipping this frame\n");
						Allsky::current_skip_frames--;
						// Do not save this frame or sleep after it.
						// We just started taking images so no need to check if DAY or NIGHT changed
						continue;
					}
				}

				// Write temperature to file
				writeTemperatureToFile((float)Allsky::actualTemp / 10.0);

				// If taking_dark_frames is off, add overlay text to the image
				if (! Allsky::taking_dark_frames)
				{
					int iYOffset = 0;
					Allsky::overlayText(iYOffset);

					if (currentAdjustGain)
					{
						// Determine if we need to change the gain on the next image.
						// This must come AFTER the "showGain" above.
						gainChange = determineGainChange(asiDayGain, Allsky::asiNightGain);
						setControl(CamNum, ASI_GAIN, Allsky::currentGain + gainChange, (ASI_BOOL)Allsky::currentAutoGain);
					}

#ifdef USE_HISTOGRAM
					if (showHistogram)
					{
						sprintf(Allsky::bufTemp, "Mean: %d", mean);
						Allsky::cvText(Allsky::pRgb, Allsky::bufTemp, Allsky::iTextX, Allsky::iTextY + (iYOffset / Allsky::currentBin),
							Allsky::fontsize * SMALLFONTSIZE_MULTIPLIER, Allsky::linewidth,
							Allsky::linetype[Allsky::linenumber], Allsky::fontname[Allsky::fontnumber], Allsky::smallFontcolor,
							Allsky::Image_type, Allsky::outlinefont);
						iYOffset += Allsky::iTextLineHeight;
					}
					if (showHistogramBox && usedHistogram)
					{
						// Draw a rectangle where the histogram box is.
						// Put a black and white line one next to each other so they
						// can be seen in light and dark images.
						int lt = cv::LINE_AA, thickness = 2;
						int X1 = (Allsky::width * histogramBoxPercentFromLeft) - (histogramBoxSizeX / 2);
						int X2 = X1 + histogramBoxSizeX;
						int Y1 = (Allsky::height * histogramBoxPercentFromTop) - (histogramBoxSizeY / 2);
						int Y2 = Y1 + histogramBoxSizeY;
						cv::Scalar outer_line, inner_line;
// xxxxxxx  TODO: can we use Scalar(x,y,z) for both?
						if (1 || Allsky::Image_type == ASI_IMG_RAW16)
						{
							outer_line = cv::Scalar(0,0,0);
							inner_line = cv::Scalar(255,255,255);
						}
						else
						{
							outer_line = cv::Scalar(0,0,0, 255);
							inner_line = cv::Scalar(255,255,255, 255);
						}
						cv::rectangle(Allsky::pRgb, cv::Point(X1, Y1), cv::Point(X2, Y2), outer_line,  thickness, lt, 0);
						cv::rectangle(Allsky::pRgb, cv::Point(X1+thickness, Y1+thickness), cv::Point(X2-thickness, Y2-thickness), inner_line,  thickness, lt, 0);
					}
#endif
				}

#ifndef USE_HISTOGRAM
				if (Allsky::currentAutoExposure == ASI_TRUE)
				{
					// Retrieve the current Exposure for smooth transition to night time
					// as long as auto-exposure is enabled during night time
					current_exposure_us = last_exposure_us;
				}
#endif

				// Save the image
				if (! bSavingImg)
				{
					pthread_mutex_lock(&mtx_SaveImg);
					pthread_cond_signal(&cond_SatrtSave);
					pthread_mutex_unlock(&mtx_SaveImg);
				}
				else
				{
					// Hopefully the user can use the time it took to save a file to disk
					// to help determine why they are getting this warning.
					// Perhaps their disk is very slow or their delay is too short.
					Allsky::Log(0, "  > WARNING: currently saving an image; can't save new one at %s.\n", exposureStart);

					// TODO: wait for the prior image to finish saving.
				}

				if (Allsky::asiNightAutoGain == 1 && dayOrNight == "NIGHT" && ! Allsky::taking_dark_frames)
				{
					ASIGetControlValue(CamNum, ASI_GAIN, &actualGain, &bAuto);
					Allsky::Log(1, "  > Auto Gain value: %ld\n", actualGain);
				}

				if (Allsky::currentAutoExposure == ASI_TRUE)
				{
#ifndef USE_HISTOGRAM

					if (dayOrNight == "DAY")
					{
						current_exposure_us = last_exposure_us;
					}
#endif

					// Delay applied before next exposure
					if (dayOrNight == "NIGHT" && Allsky::asiNightAutoExposure == 1 && last_exposure_us < (Allsky::asi_night_max_autoexposure_ms * US_IN_MS) && ! Allsky::taking_dark_frames)
					{
						// If using auto-exposure and the actual exposure is less than the max,
						// we still wait until we reach maxexposure, then wait for the delay period.
						// This is important for a constant frame rate during timelapse generation.
						// This doesn't apply during the day since we don't have a max time then.
						long s_us = (Allsky::asi_night_max_autoexposure_ms * US_IN_MS) - last_exposure_us; // to get to max
						s_us += currentDelay_ms * US_IN_MS;   // Add standard delay amount
						Allsky::Log(0, "  > Sleeping: %s\n", Allsky::length_in_units(s_us, false));
						usleep(s_us);	// usleep() is in us (microseconds)
					}
					else
					{
						// Sleep even if taking dark frames so the sensor can cool between shots like it would
						// do on a normal night.  With no delay the sensor may get hotter than it would at night.
						Allsky::Log(0, "  > Sleeping %s from %s exposure\n", Allsky::length_in_units(currentDelay_ms * US_IN_MS, false), Allsky::taking_dark_frames ? "dark frame" : "auto");
						usleep(currentDelay_ms * US_IN_MS);
					}
				}
				else
				{
					std::string s;
				   if (Allsky::taking_dark_frames)
						s = "dark frame";
				   else
						s = "manual";
#ifdef USE_HISTOGRAM
					if (usedHistogram == 1)
						s = "histogram";
#endif
					Allsky::Log(0, "  > Sleeping %s from %s exposure\n", Allsky::length_in_units(currentDelay_ms * US_IN_MS, false), s.c_str());
					usleep(currentDelay_ms * US_IN_MS);
				}
				calculateDayOrNight(Allsky::latitude, Allsky::longitude, Allsky::angle);

			} else {
				// Check if we reached the maximum number of consective errors
				bMain = check_max_errors(&exitCode, maxErrors);
			}
		}
		if (lastDayOrNight == "NIGHT")
		{
			endOfNight = true;
		}
	}

	closeUp(exitCode);
}
