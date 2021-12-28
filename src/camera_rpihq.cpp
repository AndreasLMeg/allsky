#include "allsky.h"
#include "camera_rpihq.h"

#include <string.h>
#include <string>
#include <cstring>
#include <iomanip>

using namespace std;

void CameraRPi::kill() 
{
	// Ensure no process is still running.
	// Define command line.
	string command;
	if (is_libcamera) command = "libcamera-still";
	else command = "raspistill";

	// Include "--" so we only find the command, not a different program with the command
	// on its command line.
	string kill = "ps -ef | grep '" + command + " --' | grep -v color | awk '{print $2}' | xargs kill -9 1> /dev/null 2>&1";
	char kcmd[kill.length() + 1];		// Define char variable
	strcpy(kcmd, kill.c_str());			// Convert command to character variable

	Allsky::Trace(" > Kill command: %s\n", kcmd);
	system(kcmd);						// Stop any currently running process
}


void CameraRPi::setupCapture() 
{
	// Define command line.
	string command;
	if (is_libcamera) command = "libcamera-still";
	else command = "raspistill";

	stringstream ss;
	ss << Allsky::fileName;
	command += " --output '" + ss.str() + "'";
	if (Allsky::is_libcamera)
		// xxx TODO: does this do anything?
		command += " --tuning-file /usr/share/libcamera/ipa/raspberrypi/imx477.json";
	else
		command += " --thumb none --burst -st";
}

// Build capture command to capture the image from the HQ camera
int CameraRPi::capture() 
{
		return RPiHQcapture(Allsky::currentAutoExposure, &Allsky::currentExposure_us, Allsky::currentAutoGain, Allsky::asiAutoAWB, Allsky::currentGain, Allsky::currentBin, Allsky::asiWBR, Allsky::asiWBB, Allsky::asiRotation, Allsky::asiFlip, Allsky::saturation, Allsky::currentBrightness, Allsky::quality, Allsky::fileName, Allsky::showTime, Allsky::ImgText, Allsky::fontsize, Allsky::fontcolor, Allsky::background, Allsky::taking_dark_frames, Allsky::preview, Allsky::width, Allsky::height, Allsky::is_libcamera, &Allsky::pRgb);
}

int CameraRPi::RPiHQcapture(int auto_exposure, int *exposure_us, int auto_gain, int auto_AWB, double gain, int bin, double WBR, double WBB, int rotation, int flip, float saturation, int currentBrightness, int quality, const char* fileName, int time, const char* ImgText, int fontsize, int *fontcolor, int background, int taking_dark_frames, int preview, int width, int height, bool libcamera, cv::Mat *image)
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

	Allsky::Trace(" > Kill command: %s\n", kcmd);
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

	Allsky::Debug("> Capture command: %s\n", cmd);

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

void  CameraRPi::postCapture(void)
{
				Allsky::numExposures++;

				// If taking_dark_frames is off, add overlay text to the image
				if (! Allsky::taking_dark_frames)
				{

					Allsky::lastGain = Allsky::currentGain;	// to be compatible with ZWO - ZWO gain=0.1 dB , RPiHQ gain=factor
					if (Allsky::myModeMeanSetting.mode_mean)
						Allsky::lastGain =  Allsky::myRaspistillSetting.analoggain;

					if (Allsky::myModeMeanSetting.mode_mean)
					{
						Allsky::lastMean = RPiHQcalcMean(Allsky::fileName, Allsky::asiNightExposure_us, Allsky::asiNightGain, Allsky::myRaspistillSetting, Allsky::myModeMeanSetting);
						Allsky::Debug("  > exposure: %d shutter: %1.4f s quickstart: %d\n", Allsky::asiNightExposure_us, (double) Allsky::myRaspistillSetting.shutter_us / US_IN_SEC, Allsky::myModeMeanSetting.quickstart);
					}

 					int iYOffset = 0;
					Allsky::overlayText(iYOffset);

					if (iYOffset > 0)	// if we added anything to overlay, write the file out
					{
						bool result = cv::imwrite(Allsky::fileName, Allsky::pRgb, Allsky::compression_params);
						if (! result) 
							Allsky::Error("Unable to write to '%s'\n", Allsky::fileName);
						else
							Allsky::Debug("'%s' saved with extratext\n", Allsky::fileName);
					}
				}
}
