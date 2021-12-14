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


bool bMain					= true;
int numExposures			= 0;	// how many valid pictures have we taken so far?

CameraRPi myCam;

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

// Build capture command to capture the image from the HQ camera
int RPiHQcapture(int auto_exposure, int *exposure_us, int auto_gain, int auto_AWB, double gain, int bin, double WBR, double WBB, int rotation, int flip, float saturation, int currentBrightness, int quality, const char* fileName, int time, const char* ImgText, int fontsize, int *fontcolor, int background, int taking_dark_frames, int preview, int width, int height, bool libcamera, cv::Mat *image)
{
	// Define command line.
	string command;
	if (libcamera) command = "libcamera-still";
	else command = "raspistill";

	// Ensure no process is still running.
	// Include "--" so we only find the command, not a different program with the command
	// on its command line.
	string kill = "ps -ef | grep '" + command + " --' | grep -v color | awk '{print $2}' | xargs kill -9 1> /dev/null 2>&1";
	char kcmd[kill.length() + 1];		// Define char variable
	strcpy(kcmd, kill.c_str());			// Convert command to character variable

	Allsky::Log(4, " > Kill command: %s\n", kcmd);
	system(kcmd);						// Stop any currently running process

	stringstream ss;

	ss << Allsky::fileName;
	command += " --output '" + ss.str() + "'";
	if (libcamera)
		// xxx TODO: does this do anything?
		command += " --tuning-file /usr/share/libcamera/ipa/raspberrypi/imx477.json";
	else
		command += " --thumb none --burst -st";

	// --timeout (in MS) determines how long the video will run before it takes a picture.
	if (Allsky::preview)
	{
		stringstream wh;
		wh << Allsky::width << "," << Allsky::height;
		command += " --timeout 5000";
		command += " --preview '0,0," + wh.str() + "'";	// x,y,width,height
	}
	else
	{
		ss.str("");
		// Daytime auto-exposure pictures don't need a very long --timeout since the exposures are
		// normally short so the camera can home in on the correct exposure quickly.
		if (auto_exposure)
		{
			if (Allsky::myModeMeanSetting.mode_mean)
			{
				// We do our own auto-exposure so no need to wait at all.
				ss << 1;
			}
			else if (Allsky::dayOrNight == "DAY")
			{
				ss << 1000;
			}
			else	// NIGHT
			{
				// really could use longer but then it'll take forever for pictures to appear
				ss << 10000;
			}
		}
		else
		{
			// Manual exposure shots don't need any time to home in since we're specifying the time.
			ss << 10;
		}
		command += " --timeout " + ss.str();
		command += " --nopreview";
		if (libcamera)
			command += "=1";
	}

	if (bin > 3) 	{
		bin = 3;
	}
	else if (bin < 1) 	{
		bin = 1;
	}

//xxxx not sure if this still applies for libcamera
	// https://www.raspberrypi.com/documentation/accessories/camera.html#raspistill
	// Mode   Size         Aspect Ratio  Frame rates  FOV      Binning/Scaling
	// 0      automatic selection
	// 1      2028x1080    169:90        0.1-50fps    Partial  2x2 binned
	// 2      2028x1520    4:3           0.1-50fps    Full     2x2 binned      <<< bin==2
	// 3      4056x3040    4:3           0.005-10fps  Full     None            <<< bin==1
	// 4      1332x990     74:55         50.1-120fps  Partial  2x2 binned      <<< else 
	//
	// TODO: please change gui description !

//xxx libcamera doesn't have --mode; it only uses width and height
	if (libcamera)
	{
		if (bin==1)	{
			command += " --width 4060 --height 3056";
		}
		else if (bin==2) 	{
			command += " --width 2028 --height 1520";
		}
		else 	{
			command += " --width 1012 --height 760";
		}
	}
	else
	{
		if (bin==1)	{
			command += " --mode 3";
		}
		else if (bin==2) 	{
			command += " --mode 2  --width 2028 --height 1520";
		}
		else 	{
			command += " --mode 4 --width 1012 --height 760";
		}
	}

	if (Allsky::myModeMeanSetting.mode_mean)
		*exposure_us = Allsky::myRaspistillSetting.shutter_us;

	if (*exposure_us < 1)
	{
		*exposure_us = 1;
	}
	else if (*exposure_us > 200 * US_IN_SEC)
	{
		// https://www.raspberrypi.org/documentation/raspbian/applications/camera.md : HQ (IMX477) 	200s
		*exposure_us = 200 * US_IN_SEC;
	}

	// Check if automatic determined exposure time is selected
	if (auto_exposure)
	{
		if (Allsky::myModeMeanSetting.mode_mean) {
			ss.str("");
			ss << *exposure_us;
			if (! libcamera)
				command += " --exposure off";
			command += " --shutter " + ss.str();
		} else {
			// libcamera doesn't use "exposure off/auto".  For auto-exposure set shutter to 0.
			if (libcamera)
				command += " --shutter 0";
			else
				command += " --exposure auto";
		}
	}
	else if (*exposure_us)		// manual exposure
	{
		ss.str("");
		ss << *exposure_us;
		if (! libcamera)
			command += " --exposure off";
		command += " --shutter " + ss.str();
	}

	// Check if auto gain is selected
	if (auto_gain)
	{
		if (Allsky::myModeMeanSetting.mode_mean)
		{
			ss.str("");
			ss << Allsky::myRaspistillSetting.analoggain;
			if (libcamera)
				command += " --gain " + ss.str();
			else
				command += " --analoggain " + ss.str();

//xxxx libcamera just has "gain".  If it's higher than what the camera supports,
// the excess is the "digital" gain.
if (! libcamera) { // TODO: need to fix this for libcamera
			if (Allsky::myRaspistillSetting.digitalgain > 1.0) {
				ss.str("");
				ss << Allsky::myRaspistillSetting.digitalgain;
				command += " --digitalgain " + ss.str();
			}
}
		}
		else
		{
			if (libcamera)
				command += " --gain 1";	// 1 effectively makes it autogain
			else
				command += " --analoggain 1";	// 1 effectively makes it autogain
		}
	}
	else	// Is manual gain
	{
		// xxx what are libcamera limits?
		if (gain < 1.0) {
			gain = 1.0;
		}
		else if (gain > 16.0) {
			gain = 16.0;
		}
		ss.str("");
		ss << gain;
		if (libcamera)
			command += " --gain " + ss.str();
		else
			command += " --analoggain " + ss.str();
	}

	if (Allsky::myModeMeanSetting.mode_mean) {
		 	stringstream Str_ExposureTime;
	 		stringstream Str_Reinforcement;
	 		Str_ExposureTime <<  Allsky::myRaspistillSetting.shutter_us;
		Str_Reinforcement << Allsky::myRaspistillSetting.analoggain;
		
	 		command += " --exif IFD0.Artist=li_" + Str_ExposureTime.str() + "_" + Str_Reinforcement.str();
	}

	// White balance
	if (WBR < 0.1) {
		WBR = 0.1;
	}
	else if (WBR > 10) {
		WBR = 10;
	}
	if (WBB < 0.1) {
		WBB = 0.1;
	}
	else if (WBB > 10) {
		WBB = 10;
	}

//xxx libcamera: if the red and blue numbers are given it turns off AWB.
//xxx I don't think the check for myModeMeanSetting.mode_mean is needed anymore.
	// Check if R and B component are given
	if (Allsky::myModeMeanSetting.mode_mean) {
		if (auto_AWB) {
				command += " --awb auto";
		}
		else {
			ss.str("");
			ss << WBR << "," << WBB;
			if (! libcamera)
				command += " --awb off";
			command += " --awbgains " + ss.str();
		}
	}
	else if (! auto_AWB) {
		ss.str("");
		ss << WBR << "," << WBB;
		if (! libcamera)
			command += " --awb off";
		command += " --awbgains " + ss.str();
	}
	// Use automatic white balance
	else {
		command += " --awb auto";
	}

//xxx libcamera only supports 0 and 180 degree rotation
	if (rotation != 0 && rotation != 90 && (! libcamera && rotation != 180 && rotation != 270))
	{
		rotation = 0;
	}

	// check if rotation is needed
	if (rotation != 0) {
		ss.str("");
		ss << rotation;
		command += " --rotation "  + ss.str();
	}

	// Check if flip is selected
	if (flip == 1 || flip == 3)
	{
		// Set horizontal flip
		command += " --hflip";
	}
	if (flip == 2 || flip == 3)
	{
		// Set vertical flip
		command += " --vflip";
	}

	if (Allsky::saturation < Allsky::min_saturation)
		Allsky::saturation = Allsky::min_saturation;
	else if (Allsky::saturation > Allsky::max_saturation)
		Allsky::saturation = Allsky::max_saturation;
	ss.str("");
	ss << Allsky::saturation;
	command += " --saturation "+ ss.str();

	ss.str("");
	if (Allsky::currentBrightness < Allsky::min_brightness)
	{
		Allsky::currentBrightness = Allsky::min_brightness;
	}
	else if (Allsky::currentBrightness > Allsky::max_brightness)
	{
		Allsky::currentBrightness = Allsky::max_brightness;
	}
	if (libcamera)
	{
		// User enters -100 to 100.  Convert to -1.0 to 1.0.
		ss << (float) Allsky::currentBrightness / 100;
	}
	else
	{
		ss << Allsky::currentBrightness;
	}
	command += " --brightness " + ss.str();

	if (Allsky::quality < 0)
	{
		Allsky::quality = 0;
	}
	else if (Allsky::quality > 100)
	{
		Allsky::quality = 100;
	}
	ss.str("");
	ss << Allsky::quality;
	command += " --quality " + ss.str();

	if (!Allsky::taking_dark_frames) {
		string info_text = "";

		if (Allsky::showDetails)
		{
			if (libcamera)
				info_text += " Exposure: %exp, Gain: %ag, Focus: %focus, red: %rg, blue: %bg";
			else
				info_text += " -a 1104";
		}

		if (time==1)
		{
			if (libcamera)
			{
				ss.str("");
				ss << Allsky::timeFormat;
				info_text += " Time: " + ss.str();
			}
			else
				info_text += " -a 1036";
		}

		if (strcmp(ImgText, "") != 0) {
			ss.str("");
			ss << " " << ImgText; 
			if (Allsky::debugLevel > 1) {
				ss << " (li-" << __TIMESTAMP__ 
				<< ") br:" << Allsky::myRaspistillSetting.brightness 
				<< " WBR:" << WBR 
				<< " WBB:" << WBB;
			}
			if (libcamera)
				info_text += ss.str();
			else
				info_text += " -a \"" + ss.str() + "\"";
		}
		if (info_text != "")
		{
// xxxxxxxxxxx libcamera: this only sets text on title bar of preview window, so don't bother.
			if (! libcamera)
				command += " " + info_text;
		}

if (! libcamera)	// xxxx libcamera doesn't have fontsize, color, or background.
{
		if (fontsize < 6)
			fontsize = 6;
		else if (fontsize > 160)
			fontsize = 160;

		ss.str("");
		ss << fontsize;

		// xxxxxxxx use all 3
		if (Allsky::fontcolor[0] < 0)
			Allsky::fontcolor[0] = 0;
		else if (Allsky::fontcolor[0] > 255)
			Allsky::fontcolor[0] = 255;

		std::stringstream C;
		C  << std::setfill ('0') << std::setw(2) << std::hex << Allsky::fontcolor[0];

		if (Allsky::background < 0)
			Allsky::background = 0;
		else if (Allsky::background > 255)
			Allsky::background = 255;

		std::stringstream B;
		B  << std::setfill ('0') << std::setw(2) << std::hex << Allsky::background;

		command += " -ae " + ss.str() + ",0x" + C.str() + ",0x8080" + B.str();
}
	}


	if (libcamera)
	{
		// gets rid of a bunch of libcamera verbose messages
		command = "LIBCAMERA_LOG_LEVELS='ERROR,FATAL' " + command;
	}

	if (libcamera)
		command += " 2> /dev/null";	// gets rid of a bunch of libcamera verbose messages

	// Define char variable
	char cmd[command.length() + 1];

	// Convert command to character variable
	strcpy(cmd, command.c_str());

	Allsky::Log(1, "  > Capture command: %s\n", cmd);

	// Execute the command.
	int ret = system(cmd);

	if (ret == 0)
	{
		*image = cv::imread(Allsky::fileName, cv::IMREAD_UNCHANGED);
		if (! image->data) {
			printf("WARNING: Error re-reading file '%s'; skipping further processing.\n", basename(Allsky::fileName));
		}
	}
	return(ret);
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{

	int asiDayExposure_us = 32;
	int asiNightExposure_us= 60 * US_IN_SEC;
	int asiDayAutoGain    = 0;
	int currentDelay_ms   = NOT_SET;
	int retCode;

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

	// Initialization
	int originalITextX = Allsky::iTextX;
	int originalITextY = Allsky::iTextY;
	int originalFontsize = Allsky::fontsize;
	int originalLinewidth = Allsky::linewidth;
	// Have we displayed "not taking picture during day" message, if applicable?
	int displayedNoDaytimeMsg = 0;

	while (bMain)
	{
		std::string lastDayOrNight;

		// Find out if it is currently DAY or NIGHT
		Allsky::calculateTimeToNightTime(Allsky::latitude, Allsky::longitude, Allsky::angle);

// Next line is present for testing purposes
// dayOrNight.assign("NIGHT");

		lastDayOrNight = Allsky::dayOrNight;

		if (Allsky::myModeMeanSetting.mode_mean && numExposures > 0) {
// TODO: Is this needed?  We also call RPiHQcalcMean() after the exposure.

// TODO: xxxxx shouldn't this be "currentExposure_us" instead of "asiNightExposure_us" ?
// xxxxxx and "currentGain" instead of "asiNightGain"?
				RPiHQcalcMean(Allsky::fileName, asiNightExposure_us, Allsky::asiNightGain, Allsky::myRaspistillSetting, Allsky::myModeMeanSetting);
		}

		if (Allsky::taking_dark_frames) {
			// We're doing dark frames so turn off autoexposure and autogain, and use
			// nightime gain, delay, exposure, and brightness to mimic a nightime shot.
			Allsky::currentAutoExposure = 0;
			Allsky::currentAutoGain = 0;
			Allsky::currentGain = Allsky::asiNightGain;
			currentDelay_ms = Allsky::nightDelay_ms;
			Allsky::currentExposure_us = asiNightExposure_us;
			Allsky::currentBrightness = Allsky::asiNightBrightness;
			Allsky::currentBin = Allsky::nightBin;

 			Allsky::Log(0, "Taking dark frames...\n");
			if (Allsky::notificationImages) {
				system("scripts/copy_notification_image.sh DarkFrames &");
			}
		}

		else if (Allsky::dayOrNight == "DAY")
		{
			if (Allsky::endOfNight == true)		// Execute end of night script
			{
				system("scripts/endOfNight.sh &");

				// Reset end of night indicator
				Allsky::endOfNight = false;

				displayedNoDaytimeMsg = 0;
			}

			// Check if images should not be captured during day-time
			if (Allsky::daytimeCapture != 1)
			{
				// Only display messages once a day.
				if (displayedNoDaytimeMsg == 0) {
					if (Allsky::notificationImages) {
						system("scripts/copy_notification_image.sh CameraOffDuringDay &");
					}
					Allsky::Log(0, "It's daytime... we're not saving images.\n%s\n",
						Allsky::tty ? "Press Ctrl+C to stop" : "Stop the allsky service to end this process.");
					displayedNoDaytimeMsg = 1;

					// sleep until almost nighttime, then wake up and sleep a short time
					int secsTillNight = Allsky::calculateTimeToNightTime(Allsky::latitude, Allsky::longitude, Allsky::angle);
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

			// Images should be captured during day-time
			else
			{
				Allsky::Log(0, "==========\n=== Starting daytime capture ===\n==========\n");

								// If we went from Night to Day, then currentExposure_us will be the last night
								// exposure so leave it if we're using auto-exposure so there's a seamless change from
								// Night to Day, i.e., if the exposure was fine a minute ago it will likely be fine now.
								// On the other hand, if this program just started or we're using manual exposures,
								// use what the user specified.
								if (numExposures == 0 || ! Allsky::asiDayAutoExposure)
								{
					Allsky::currentExposure_us = asiDayExposure_us;
					Allsky::myRaspistillSetting.shutter_us = Allsky::currentExposure_us;
								}
								else
								{
										Allsky::Log(3, "Using the last night exposure of %'ld\n", Allsky::currentExposure_us);
								}
				Allsky::currentAutoExposure = Allsky::asiDayAutoExposure;
				Allsky::currentBrightness = Allsky::asiDayBrightness;
				currentDelay_ms = Allsky::dayDelay_ms;
				Allsky::currentBin = Allsky::dayBin;
				Allsky::currentGain = Allsky::asiDayGain;
				Allsky::currentAutoGain = asiDayAutoGain;
			}
		}

		else	// NIGHT
		{
			Allsky::Log(0, "==========\n=== Starting nighttime capture ===\n==========\n");

			// Setup the night time capture parameters
			if (numExposures == 0 || ! Allsky::asiNightAutoExposure)
			{
				Allsky::currentExposure_us = asiNightExposure_us;
				Allsky::Log(3, "Using night exposure (%'ld)\n", asiNightExposure_us);
				Allsky::myRaspistillSetting.shutter_us = Allsky::currentExposure_us;
			}
			Allsky::currentAutoExposure = Allsky::asiNightAutoExposure;
			Allsky::currentBrightness = Allsky::asiNightBrightness;
			currentDelay_ms = Allsky::nightDelay_ms;
			Allsky::currentBin = Allsky::nightBin;
			Allsky::currentGain = Allsky::asiNightGain;
			Allsky::currentAutoGain = Allsky::asiNightAutoGain;
		}

		// Adjusting variables for chosen binning
		Allsky::height    = Allsky::originalHeight / Allsky::currentBin;
		Allsky::width     = Allsky::originalWidth / Allsky::currentBin;
		Allsky::iTextX    = originalITextX / Allsky::currentBin;
		Allsky::iTextY    = originalITextY / Allsky::currentBin;
		Allsky::fontsize  = originalFontsize / Allsky::currentBin;
		Allsky::linewidth = originalLinewidth / Allsky::currentBin;

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

		if (Allsky::tty)
			printf("Press Ctrl+Z to stop\n\n");	// xxx ECC: Ctrl-Z stops a process, it doesn't kill it
		else
			printf("Stop the allsky service to end this process.\n\n");

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
			Allsky::Log(0, "STARTING EXPOSURE at: %s   @ %s\n", exposureStart, Allsky::length_in_units(Allsky::currentExposure_us, true));

			// Get start time for overlay.  Make sure it has the same time as exposureStart.
			if (Allsky::showTime == 1)
				sprintf(Allsky::bufTime, "%s", Allsky::formatTime(t, Allsky::timeFormat));

			myCam.setup();
			// Capture and save image
			retCode = RPiHQcapture(Allsky::currentAutoExposure, &Allsky::currentExposure_us, Allsky::currentAutoGain, Allsky::asiAutoAWB, Allsky::currentGain, Allsky::currentBin, Allsky::asiWBR, Allsky::asiWBB, Allsky::asiRotation, Allsky::asiFlip, Allsky::saturation, Allsky::currentBrightness, Allsky::quality, Allsky::fileName, Allsky::showTime, Allsky::ImgText, Allsky::fontsize, Allsky::fontcolor, Allsky::background, Allsky::taking_dark_frames, Allsky::preview, Allsky::width, Allsky::height, Allsky::is_libcamera, &Allsky::pRgb);
			if (retCode == 0)
			{
				numExposures++;

				// If taking_dark_frames is off, add overlay text to the image
				if (! Allsky::taking_dark_frames)
				{

					Allsky::lastGain = Allsky::currentGain;	// to be compatible with ZWO - ZWO gain=0.1 dB , RPiHQ gain=factor
					if (Allsky::myModeMeanSetting.mode_mean)
						Allsky::lastGain =  Allsky::myRaspistillSetting.analoggain;

					if (Allsky::myModeMeanSetting.mode_mean)
					{
						Allsky::lastMean = RPiHQcalcMean(Allsky::fileName, asiNightExposure_us, Allsky::asiNightGain, Allsky::myRaspistillSetting, Allsky::myModeMeanSetting);
						Allsky::Log(2, "  > exposure: %d shutter: %1.4f s quickstart: %d\n", asiNightExposure_us, (double) Allsky::myRaspistillSetting.shutter_us / US_IN_SEC, Allsky::myModeMeanSetting.quickstart);
					}

 					int iYOffset = 0;
					Allsky::overlayText(iYOffset);

					if (iYOffset > 0)	// if we added anything to overlay, write the file out
					{
						bool result = cv::imwrite(Allsky::fileName, Allsky::pRgb, Allsky::compression_params);
						if (! result) printf("*** ERROR: Unable to write to '%s'\n", Allsky::fileName);
					}
				}
			}
			else
			{
				printf(" >>> Unable to take picture, return code=%d\n", (retCode >> 8));
				Allsky::Log(1, "  > Sleeping from failed exposure: %.1f seconds\n", (float)currentDelay_ms / MS_IN_SEC);
				usleep(currentDelay_ms * US_IN_MS);
				continue;
			}

			// Check for night time
			if (Allsky::dayOrNight == "NIGHT")
			{
				// Preserve image during night time
				system("scripts/saveImageNight.sh &");
			}
			else
			{
				// Upload and resize image when configured
				system("scripts/saveImageDay.sh &");
			}

			long s;
			if (Allsky::myModeMeanSetting.mode_mean && Allsky::myModeMeanSetting.quickstart)
			{
				s = 1 * US_IN_SEC;
			}
			else if ((Allsky::dayOrNight == "NIGHT"))
			{
				s = (asiNightExposure_us - Allsky::myRaspistillSetting.shutter_us) + (Allsky::nightDelay_ms * US_IN_MS);
			}
			else
			{
				s = currentDelay_ms * US_IN_MS;
			}
			Allsky::Log(0, "Sleeping %.1f seconds...\n", (float)s / US_IN_SEC);
			usleep(s);

			// Check for day or night based on location and angle
			Allsky::calculateTimeToNightTime(Allsky::latitude, Allsky::longitude, Allsky::angle);
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
