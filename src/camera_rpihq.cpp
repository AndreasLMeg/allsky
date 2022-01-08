#include "allsky.h"
#include "camera_rpihq.h"
#include "modeMean.h"

#include <string.h>
#include <string>
#include <cstring>
#include <iomanip>
#include <unistd.h>

using namespace std;

void CameraRPi::kill() 
{
	// Ensure no process is still running.
	// Define command line.
	string command;
	if (settings.camera.is_libcamera) command = "libcamera-still";
	else command = "raspistill";

	// Include "--" so we only find the command, not a different program with the command
	// on its command line.
	string kill = "ps -ef | grep '" + command + " --' | grep -v color | awk '{print $2}' | xargs kill -9 1> /dev/null 2>&1";
	char kcmd[kill.length() + 1];		// Define char variable
	strcpy(kcmd, kill.c_str());			// Convert command to character variable

	Allsky::Trace(" > Kill command: %s\n", kcmd);
	system(kcmd);						// Stop any currently running process
}

void CameraRPi::initCamera()
{
	// nothing to do
}

void CameraRPi::setupForCapture() 
{
	// Define command line.
	string command;
	if (settings.camera.is_libcamera) command = "libcamera-still";
	else command = "raspistill";

	stringstream ss;
	ss << settings.image.fileName;
	command += " --output '" + ss.str() + "'";
	if (settings.camera.is_libcamera)
		// xxx TODO: does this do anything?
		command += " --tuning-file /usr/share/libcamera/ipa/raspberrypi/imx477.json";
	else
		command += " --thumb none --burst -st";
}

// Build capture command to capture the image from the HQ camera
int CameraRPi::capture() 
{
		return RPiHQcapture(Allsky::currentAutoExposure, &Allsky::currentExposure_us, Allsky::currentAutoGain, settings.camera.asiAutoAWB, Allsky::currentGain, Allsky::currentBin, settings.camera.asiWBR, settings.camera.asiWBB, settings.image.asiRotation, settings.image.asiFlip, settings.camera.saturation, Allsky::currentBrightness, settings.image.quality, settings.image.fileName, Allsky::showTime, Allsky::ImgText, settings.fonts.fontsize, settings.fonts.fontcolor, Allsky::background, settings.taking_dark_frames, settings.preview, settings.image.width, settings.image.height, settings.camera.is_libcamera);
}

int CameraRPi::RPiHQcapture(int auto_exposure, int *exposure_us, int auto_gain, int auto_AWB, double gain, int bin, double WBR, double WBB, int rotation, int flip, float saturation, int currentBrightness, int quality, const char* fileName, int time, const char* ImgText, int fontsize, int *fontcolor, int background, int taking_dark_frames, int preview, int width, int height, bool libcamera)
{
	// Define command line.
	string command;
	if (libcamera) command = "libcamera-still";
	else command = "raspistill";
	Allsky::Info("RPiHQcapture:0\n");

	// Ensure no process is still running.
	// Include "--" so we only find the command, not a different program with the command
	// on its command line.
	string kill = "ps -ef | grep '" + command + " --' | grep -v color | awk '{print $2}' | xargs kill -9 1> /dev/null 2>&1";
	char kcmd[kill.length() + 1];		// Define char variable
	strcpy(kcmd, kill.c_str());			// Convert command to character variable

	Allsky::Trace(" > Kill command: %s\n", kcmd);
	Allsky::Info("RPiHQcapture:1\n");
	system(kcmd);						// Stop any currently running process
	Allsky::Info("RPiHQcapture:2\n");

	stringstream ss;

	ss << settings.image.fileName;
	command += " --output '" + ss.str() + "'";
	if (libcamera)
		// xxx TODO: does this do anything?
		command += " --tuning-file /usr/share/libcamera/ipa/raspberrypi/imx477.json";
	else
		command += " --thumb none --burst -st";

	// --timeout (in MS) determines how long the video will run before it takes a picture.
	if (settings.preview)
	{
		stringstream wh;
		wh << settings.image.width << "," << settings.image.height;
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
			if (mode_mean)
			{
				// We do our own auto-exposure so no need to wait at all.
				ss << 1;
			}
			else if (runtime.dayOrNight == "DAY")
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

	Allsky::Info("RPiHQcapture:3\n");

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

	Allsky::Info("RPiHQcapture:4\n");

	if (mode_mean)
		*exposure_us = Allsky::valuesCapture.exposure_us;

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
		if (mode_mean) {
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

	Allsky::Info("RPiHQcapture:5\n");

	// Check if auto gain is selected
	if (auto_gain)
	{
		if (mode_mean)
		{
			ss.str("");
			ss << valuesCapture.gain;
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

	Allsky::Info("RPiHQcapture:6\n");

	if (mode_mean) {
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
	if (mode_mean) {
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

	if (settings.camera.saturation < Allsky::min_saturation)
		settings.camera.saturation = Allsky::min_saturation;
	else if (settings.camera.saturation > Allsky::max_saturation)
		settings.camera.saturation = Allsky::max_saturation;
	ss.str("");
	ss << settings.camera.saturation;
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

	ss.str("");
	ss << settings.image.quality;
	command += " --quality " + ss.str();

	if (!settings.taking_dark_frames) {
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
			if (settings.debugLevel > 1) {
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

	Allsky::Info("RPiHQcapture:7\n");

if (! libcamera)	// xxxx libcamera doesn't have fontsize, color, or background.
{
		if (settings.fonts.fontsize < 6)
			settings.fonts.fontsize = 6;
		else if (settings.fonts.fontsize > 160)
			settings.fonts.fontsize = 160;

		ss.str("");
		ss << settings.fonts.fontsize;

		// xxxxxxxx use all 3
		if (settings.fonts.fontcolor[0] < 0)
			settings.fonts.fontcolor[0] = 0;
		else if (settings.fonts.fontcolor[0] > 255)
			settings.fonts.fontcolor[0] = 255;

		std::stringstream C;
		C  << std::setfill ('0') << std::setw(2) << std::hex << settings.fonts.fontcolor[0];

		if (Allsky::background < 0)
			Allsky::background = 0;
		else if (Allsky::background > 255)
			Allsky::background = 255;

		std::stringstream B;
		B  << std::setfill ('0') << std::setw(2) << std::hex << Allsky::background;

		command += " -ae " + ss.str() + ",0x" + C.str() + ",0x8080" + B.str();
}
	}

	Allsky::Info("RPiHQcapture:8\n");

	if (libcamera)
	{
		// gets rid of a bunch of libcamera verbose messages
		command = "LIBCAMERA_LOG_LEVELS='ERROR,FATAL' " + command;
	}

	if (libcamera)
		command += " 2> /dev/null";	// gets rid of a bunch of libcamera verbose messages

	command += "; sync";

	Allsky::Info("RPiHQcapture:9\n");

	// Define char variable
	char cmd[command.length() + 1];

	Allsky::Info("RPiHQcapture:10\n");

	// Convert command to character variable
	strcpy(cmd, command.c_str());

	Allsky::Info("RPiHQcapture:11\n");

	Allsky::Debug("> Capture command: %s\n", cmd);

	system("sync;");
	// Execute the command.
	int ret = system(cmd);

	Allsky::Info("RPiHQcapture:12 ret=%d\n", ret);
	if (ret == 0)
	{
		Allsky::Info("RPiHQcapture:13\n");
		settings.image.pRgb.release();
		Allsky::Info("RPiHQcapture:13a\n");
		settings.image.pRgb = cv::imread(settings.image.fileName, cv::IMREAD_UNCHANGED);
		Allsky::Info("RPiHQcapture:14\n");
		if (! settings.image.pRgb.data) {
			Allsky::Info("RPiHQcapture:15\n");
			printf("WARNING: Error re-reading file '%s'; skipping further processing.\n", basename(settings.image.fileName));
		}
	}
	Allsky::Info("RPiHQcapture:16\n");
	return(ret);
}

void  CameraRPi::postCapture(void)
{
				Allsky::numExposures++;

				// If taking_dark_frames is off, add overlay text to the image
				if (! settings.taking_dark_frames)
				{

					Allsky::lastGain = Allsky::currentGain;	// to be compatible with ZWO - ZWO gain=0.1 dB , RPiHQ gain=factor
					if (mode_mean)
						Allsky::lastGain = valuesCapture.gain;

					if (mode_mean)
					{
						//Allsky::lastMean = RPiHQcalcMean(settings.image.fileName, Allsky::asiNightExposure_us, settings.camera.asiNightGain, Allsky::myRaspistillSetting, Allsky::myModeMeanSetting);
						
						calculateNextExposureSettings();
						Allsky::lastMean = ModeMean::values_ModeMean.current_mean;
						Allsky::myRaspistillSetting.shutter_us = Allsky::valuesCapture.exposure_us;
						Allsky::myRaspistillSetting.analoggain = Allsky::valuesCapture.gain;

						Allsky::Debug("  > exposure: %d shutter: %1.4f s quickstart: %d\n", Allsky::asiNightExposure_us, (double) Allsky::myRaspistillSetting.shutter_us / US_IN_SEC, quickstart);
					}

 					int iYOffset = 0;
					Allsky::overlayText(iYOffset);

					if (iYOffset > 0)	// if we added anything to overlay, write the file out
					{
						bool result = cv::imwrite(settings.image.fileName, settings.image.pRgb, Allsky::compression_params);
						if (! result) 
							Allsky::Error("Unable to write to '%s'\n", settings.image.fileName);
						else
							Allsky::Debug("'%s' saved with extratext\n", settings.image.fileName);
					}
				}
}

void CameraRPi::waitForNextCapture(void)
{
			long us;
			if (mode_mean && quickstart)
			{
				us = 1 * US_IN_SEC;
			}
			else if ((runtime.dayOrNight == "NIGHT"))
			{
				us = (Allsky::asiNightExposure_us - Allsky::myRaspistillSetting.shutter_us) + (settings.night.nightDelay_ms * US_IN_MS);
			}
			else
			{
				us = current.currentDelay_ms * US_IN_MS;
			}
			Allsky::Info("Sleeping %.1f seconds...\n", (float)us / US_IN_SEC);
			usleep(us);
}

void CameraRPi::prepareForDayOrNight(void) 
{
	// Have we displayed "not taking picture during day" message, if applicable?
	int displayedNoDaytimeMsg = 0;
	
	// Find out if it is currently DAY or NIGHT
	calculateDayOrNight();
	runtime.lastDayOrNight = runtime.dayOrNight;

		if (mode_mean && numExposures > 0) {
// TODO: Is this needed?  We also call RPiHQcalcMean() after the exposure.

// TODO: xxxxx shouldn't this be "currentExposure_us" instead of "asiNightExposure_us" ?
// xxxxxx and "currentGain" instead of "asiNightGain"?
				//RPiHQcalcMean(settings.image.fileName, asiNightExposure_us, settings.camera.asiNightGain, Allsky::myRaspistillSetting, Allsky::myModeMeanSetting);
		}

		if (settings.taking_dark_frames) {
			// We're doing dark frames so turn off autoexposure and autogain, and use
			// nightime gain, delay, exposure, and brightness to mimic a nightime shot.
			Allsky::currentAutoExposure = 0;
			Allsky::currentAutoGain = 0;
			Allsky::currentGain = settings.camera.asiNightGain;
			current.currentDelay_ms = settings.night.nightDelay_ms;
			Allsky::currentExposure_us = asiNightExposure_us;
			Allsky::currentBrightness = settings.camera.asiNightBrightness;
			Allsky::currentBin = settings.camera.nightBin;

 			Allsky::Info("Taking dark frames...\n");
			if (settings.notificationImages) {
				system("scripts/copy_notification_image.sh DarkFrames &");
			}
		}

		else if (runtime.dayOrNight == "DAY")
		{
			if (runtime.endOfNight)		// Execute end of night script
			{
 			  Info("starting endOfNight.sh\n");
				system("scripts/endOfNight.sh &");

				// Reset end of night indicator
				runtime.endOfNight = false;

				displayedNoDaytimeMsg = 0;
			}

			// Check if images should not be captured during day-time
			if (settings.day.daytimeCapture != 1)
			{
				// Only display messages once a day.
				if (displayedNoDaytimeMsg == 0) {
					if (settings.notificationImages) {
						system("scripts/copy_notification_image.sh CameraOffDuringDay &");
					}
					Allsky::Info("It's daytime... we're not saving images.\n%s\n",
						settings.tty ? "Press Ctrl+C to stop" : "Stop the allsky service to end this process.");
					displayedNoDaytimeMsg = 1;

					// sleep until almost nighttime, then wake up and sleep a short time
					int secsTillNight = Allsky::calculateTimeToNightTime();
					sleep(secsTillNight - 10);
				}
				else
				{
					// Shouldn't need to sleep more than a few times before nighttime.
					sleep(5);
				}

				// No need to do any of the code below so go back to the main loop.
				return;
			}

			// Images should be captured during day-time
			else
			{
				Allsky::Info("============= Starting daytime capture =============\n");

								// If we went from Night to Day, then currentExposure_us will be the last night
								// exposure so leave it if we're using auto-exposure so there's a seamless change from
								// Night to Day, i.e., if the exposure was fine a minute ago it will likely be fine now.
								// On the other hand, if this program just started or we're using manual exposures,
								// use what the user specified.
								if (numExposures == 0 || ! settings.camera.asiDayAutoExposure)
								{
									Allsky::currentExposure_us = asiDayExposure_us;
									Allsky::myRaspistillSetting.shutter_us = Allsky::currentExposure_us;
								}
								else
								{
										Allsky::Debug("Using the last night exposure of %'ld\n", Allsky::currentExposure_us);
								}
				Allsky::currentAutoExposure = settings.camera.asiDayAutoExposure;
				Allsky::currentBrightness = settings.camera.asiDayBrightness;
				current.currentDelay_ms = settings.day.dayDelay_ms;
				Allsky::currentBin = settings.camera.dayBin;
				Allsky::currentGain = settings.camera.asiDayGain;
				Allsky::currentAutoGain = asiDayAutoGain;
			}
		}

		else	// NIGHT
		{
			Allsky::Info("============= Starting nighttime capture =============\n");

			// Setup the night time capture parameters
			if (numExposures == 0 || ! settings.camera.asiNightAutoExposure)
			{
				Allsky::currentExposure_us = asiNightExposure_us;
				Allsky::Debug("Using night exposure (%'ld)\n", asiNightExposure_us);
				Allsky::myRaspistillSetting.shutter_us = Allsky::currentExposure_us;
			}
			Allsky::currentAutoExposure = settings.camera.asiNightAutoExposure;
			Allsky::currentBrightness = settings.camera.asiNightBrightness;
			current.currentDelay_ms = settings.night.nightDelay_ms;
			Allsky::currentBin = settings.camera.nightBin;
			Allsky::currentGain = settings.camera.asiNightGain;
			Allsky::currentAutoGain = settings.camera.asiNightAutoGain;
		}

		// Adjusting variables for chosen binning
		// TODO: too much ????
		/*
		settings.image.height    = Allsky::originalHeight / Allsky::currentBin;
		settings.image.width     = Allsky::originalWidth / Allsky::currentBin;
		Allsky::iTextX    = originalITextY /Allsky::currentBin;
		Allsky::iTextY    = originalITextY / Allsky::currentBin;
		settings.fonts.fontsize  = originalFontsize / Allsky::currentBin;
		Allsky::linewidth = originalLinewidth / Allsky::currentBin;
		*/

		// TODO: if not the first time, should we free the old settings.image.pRgb?
		settings.image.pRgb.release();
		if (settings.image.Image_type == ASI_IMG_RAW16)
		{
			settings.image.pRgb.create(cv::Size(settings.image.width, settings.image.height), CV_16UC1);
		}
		else if (settings.image.Image_type == ASI_IMG_RGB24)
		{
			settings.image.pRgb.create(cv::Size(settings.image.width, settings.image.height), CV_8UC3);
		}
		else // RAW8 and Y8
		{
			settings.image.pRgb.create(cv::Size(settings.image.width, settings.image.height), CV_8UC1);
		}

		if (settings.tty)
			printf("Press Ctrl+Z to stop\n\n");	// xxx ECC: Ctrl-Z stops a process, it doesn't kill it
		else
			printf("Stop the allsky service to end this process.\n\n");
}
