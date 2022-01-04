#include "camera_zwo.h"
#include "allsky.h"
#include "include/ASICamera2.h"

#include <string.h>
#include <string>
#include <cstring>
#include <iomanip>
#include <unistd.h>
#include <fstream>


using namespace std;
int maxErrors       = 5;    // Max number of errors in a row before we exit
char const *bayer[]   = { "RG", "BG", "GR", "GB" };
//int i;
int iNumOfCtrl = 0;
char retCodeBuffer[100];
int asi_error_timeout_cntr = 0;
ASI_BOOL bAuto = ASI_FALSE;		// "auto" flag returned by ASIGetControlValue, when we don't care what it is
long actualGain = 0;			// actual gain used, per the camera
long reported_exposure_us = 0;	// exposure reported by the camera, either actual exposure or suggested next one
long last_exposure_us = 0;		// last exposure taken
int numErrors              = 0;	// Number of errors in a row.
ASI_BOOL wasAutoExposure = ASI_FALSE;
ASI_ERROR_CODE asiRetCode;  // used for return code from ASI functions.


//#ifdef USE_HISTOGRAM
int mean = 0;
int attempts = 0;
int histogram[256];
//#endif



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
Allsky::Debug("  > [Cleared buffer frame, next exposure: %'ld, auto=%s]\n", us, b==ASI_TRUE ? "yes" : "no");
	}

// xxxxxxxxxx For now, display message above for each one rather than a summary.
return;
	if (num_cleared > 0)
	{
		Allsky::Debug("  > [Cleared %d buffer frame%s]\n", num_cleared, num_cleared > 1 ? "s" : "");
	}
}



void CameraZWO::kill() 
{
// todo
}


void CameraZWO::initCamera()
{
const char *imagetype = "";
	const char *ext = strrchr(settings.image.fileName, '.');
	if (strcasecmp(ext + 1, "jpg") == 0 || strcasecmp(ext + 1, "jpeg") == 0)
	{
		if (settings.image.Image_type == ASI_IMG_RAW16)
		{
			Allsky::waitToFix("*** ERROR: RAW16 images only work with .png files; either change the Image Type or the Filename.\n");
			exit(2);
		}

		imagetype = "jpg";
		Allsky::compression_parameters.push_back(cv::IMWRITE_JPEG_QUALITY);
		// want dark frames to be at highest quality
		if (settings.image.quality > 100 || settings.taking_dark_frames)
		{
			settings.image.quality = 100;
		}
		else if (settings.image.quality == NOT_SET)
		{
			settings.image.quality = 95;
		}
	}
	else if (strcasecmp(ext + 1, "png") == 0)
	{
		imagetype = "png";
		Allsky::compression_parameters.push_back(cv::IMWRITE_PNG_COMPRESSION);
		if (settings.taking_dark_frames)
		{
			settings.image.quality = 0;	// actually, it's PNG compression - 0 is highest quality
		}
		else if (settings.image.quality > 9)
		{
			settings.image.quality = 9;
		}
		else if (settings.image.quality == NOT_SET)
		{
			settings.image.quality = 3;
		}
	}
	else
	{
		sprintf(Allsky::debugText, "*** ERROR: Unsupported image extension (%s); only .jpg and .png are supported.\n", ext);
		Allsky::waitToFix(Allsky::debugText);
		exit(100);
	}
	Allsky::compression_parameters.push_back(settings.image.quality);

	if (settings.taking_dark_frames)
	{
		// To avoid overwriting the optional notification inage with the dark image,
		// during dark frames we use a different file name.
		static char darkFilename[200];
		sprintf(darkFilename, "dark.%s", imagetype);
		settings.image.fileName = darkFilename;
	}

	int numDevices = ASIGetNumOfConnectedCameras();
	if (numDevices <= 0)
	{
		printf("*** ERROR: No Connected Camera...\n");
		// Don't wait here since it's possible the camera is physically connected
		// but the software doesn't see it and the USB bus needs to be reset.
		Allsky::closeUp(1);   // If there are no cameras we can't do anything.
	}

	if (numDevices > 1)
	{
		printf("\nAttached Cameras%s:\n", numDevices == 1 ? "" : " (using first one)");
		for (int i = 0; i < numDevices; i++)
		{
			ASIGetCameraProperty(&Allsky::ASICameraInfo, i);
			printf("  - %d %s\n", i, Allsky::ASICameraInfo.Name);
		}
	}
	ASIGetCameraProperty(&Allsky::ASICameraInfo, 0);	// want info on 1st camera

	asiRetCode = ASIOpenCamera(Allsky::CamNum);
	if (asiRetCode != ASI_SUCCESS)
	{
		printf("*** ERROR opening camera, check that you have root permissions! (%s)\n", getRetCode(asiRetCode));
		Allsky::closeUp(1);      // Can't do anything so might as well exit.
	}

	int iMaxWidth, iMaxHeight;
	double pixelSize;
	iMaxWidth  = Allsky::ASICameraInfo.MaxWidth;
	iMaxHeight = Allsky::ASICameraInfo.MaxHeight;
	pixelSize  = Allsky::ASICameraInfo.PixelSize;
	if (settings.image.width == 0 || settings.image.height == 0)
	{
		settings.image.width  = iMaxWidth;
		settings.image.height = iMaxHeight;
	}
	Allsky::originalWidth = settings.image.width;
	Allsky::originalHeight = settings.image.height;

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
		centerX = settings.image.width * histogramBoxPercentFromLeft;
		centerY = settings.image.height * histogramBoxPercentFromTop;
		left_of_box = centerX - (histogramBoxSizeX / 2);
		right_of_box = centerX + (histogramBoxSizeX / 2);
		top_of_box = centerY - (histogramBoxSizeY / 2);
		bottom_of_box = centerY + (histogramBoxSizeY / 2);

		if (left_of_box < 0 || right_of_box >= settings.image.width || top_of_box < 0 || bottom_of_box >= settings.image.height)
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
	if (Allsky::ASICameraInfo.IsUSB3Camera == ASI_TRUE && ASIGetID(Allsky::CamNum, &cameraID) == ASI_SUCCESS)
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
	asiRetCode = ASIGetSerialNumber(Allsky::CamNum, &serialNumber);
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

	asiRetCode = ASIInitCamera(Allsky::CamNum);
	if (asiRetCode != ASI_SUCCESS)
	{
		printf("*** ERROR: Unable to initialise camera: %s\n", getRetCode(asiRetCode));
		Allsky::closeUp(1);      // Can't do anything so might as well exit.
	}

	// Get a few values from the camera that we need elsewhere.
	ASIGetNumOfControls(Allsky::CamNum, &iNumOfCtrl);
	if (settings.debugLevel >= 4)
		printf("Control Caps:\n");
	for (int i = 0; i < iNumOfCtrl; i++)
	{
		ASIGetControlCaps(Allsky::CamNum, i, &Allsky::ControlCaps);
		switch (Allsky::ControlCaps.ControlType) {
		case ASI_EXPOSURE:
			Allsky::camera_min_exposure_us = Allsky::ControlCaps.MinValue;
			break;
#ifdef USE_HISTOGRAM
		case ASI_AUTO_MAX_EXP:
			// Keep track of the camera's max auto-exposure so we don't try to exceed it.
			// MaxValue is in MS so convert to microseconds
			camera_max_autoexposure_us = Allsky::ControlCaps.MaxValue * US_IN_MS;
			break;
		default:	// needed to keep compiler quiet
			break;
#endif
		}
		if (settings.debugLevel >= 4)
		{
			printf("- %s:\n", Allsky::ControlCaps.Name);
			printf("   - Description = %s\n", Allsky::ControlCaps.Description);
			printf("   - MinValue = %'ld\n", Allsky::ControlCaps.MinValue);
			printf("   - MaxValue = %'ld\n", Allsky::ControlCaps.MaxValue);
			printf("   - DefaultValue = %'ld\n", Allsky::ControlCaps.DefaultValue);
			printf("   - IsAutoSupported = %d\n", Allsky::ControlCaps.IsAutoSupported);
			printf("   - IsWritable = %d\n", Allsky::ControlCaps.IsWritable);
			printf("   - ControlType = %d\n", Allsky::ControlCaps.ControlType);
		}
	}

	if (Allsky::asi_day_exposure_us < Allsky::camera_min_exposure_us)
	{
	   	fprintf(stderr, "*** WARNING: daytime exposure %'ld us less than camera minimum of %'ld us; setting to minimum\n", Allsky::asi_day_exposure_us, Allsky::camera_min_exposure_us);
	   	Allsky::asi_day_exposure_us = Allsky::camera_min_exposure_us;
	}
	else if (settings.camera.asiDayAutoExposure && Allsky::asi_day_exposure_us > Allsky::camera_max_autoexposure_us)
	{
	   	fprintf(stderr, "*** WARNING: daytime exposure %'ld us greater than camera maximum of %'ld us; setting to maximum\n", Allsky::asi_day_exposure_us, Allsky::camera_max_autoexposure_us);
	   	Allsky::asi_day_exposure_us = Allsky::camera_max_autoexposure_us;
	}
	if (Allsky::asi_night_exposure_us < Allsky::camera_min_exposure_us)
	{
	   	fprintf(stderr, "*** WARNING: nighttime exposure %'ld us less than camera minimum of %'ld us; setting to minimum\n", Allsky::asi_night_exposure_us, Allsky::camera_min_exposure_us);
	   	Allsky::asi_night_exposure_us = Allsky::camera_min_exposure_us;
	}
	else if (settings.camera.asiNightAutoExposure && Allsky::asi_night_exposure_us > Allsky::camera_max_autoexposure_us)
	{
	   	fprintf(stderr, "*** WARNING: nighttime exposure %'ld us greater than camera maximum of %'ld us; setting to maximum\n", Allsky::asi_night_exposure_us, Allsky::camera_max_autoexposure_us);
	   	Allsky::asi_night_exposure_us = Allsky::camera_max_autoexposure_us;
	}

	if (settings.debugLevel >= 4)
	{
		printf("Supported video formats:\n");
		for (int i = 0; i < 8; i++)
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

	ASIGetControlValue(Allsky::CamNum, ASI_TEMPERATURE, &Allsky::actualTemp, &bAuto);
	printf("- Sensor temperature: %0.2f\n", (float)Allsky::actualTemp / 10.0);

	// Handle "auto" Image_type.
	if (settings.image.Image_type == AUTO_IMAGE_TYPE)
	{
		// If it's a color camera, create color pictures.
		// If it's a mono camera use RAW16 if the image file is a .png, otherwise use RAW8.
		// There is no good way to handle Y8 automatically so it has to be set manually.
		if (Allsky::ASICameraInfo.IsColorCam)
			settings.image.Image_type = ASI_IMG_RGB24;
		else if (strcmp(imagetype, "png") == 0)
			settings.image.Image_type = ASI_IMG_RAW16;
		else // jpg
			settings.image.Image_type = ASI_IMG_RAW8;
	}

	
	if (settings.image.Image_type == ASI_IMG_RAW16)
	{
		Allsky::sType = "ASI_IMG_RAW16";
	}
	else if (settings.image.Image_type == ASI_IMG_RGB24)
	{
		Allsky::sType = "ASI_IMG_RGB24";
	}
	else if (settings.image.Image_type == ASI_IMG_RAW8)
	{
		// Color cameras should use Y8 instead of RAW8.  Y8 is the mono mode for color cameras.
		if (Allsky::ASICameraInfo.IsColorCam)
		{
			settings.image.Image_type = ASI_IMG_Y8;
			Allsky::sType = "ASI_IMG_Y8 (not RAW8 for color cameras)";
		}
		else
		{
			Allsky::sType = "ASI_IMG_RAW8";
		}
	}
	else if (settings.image.Image_type == ASI_IMG_RAW8)
	{
		Allsky::sType = "ASI_IMG_Y8";
	}
	else
	{
		sprintf(Allsky::debugText, "*** ERROR: ASI_IMG_TYPE: %d\n", settings.image.Image_type);
		Allsky::waitToFix(Allsky::debugText);
		exit(100);
	}

		//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	// These configurations apply to both day and night.
	// Other calls to setControl() are done after we know if we're in daytime or nighttime.
	setControl(Allsky::CamNum, ASI_BANDWIDTHOVERLOAD, Allsky::asiBandwidth, Allsky::asiAutoBandwidth == 1 ? ASI_TRUE : ASI_FALSE);
	setControl(Allsky::CamNum, ASI_HIGH_SPEED_MODE, 0, ASI_FALSE);  // ZWO sets this in their program
	if (Allsky::ASICameraInfo.IsColorCam)
	{
		setControl(Allsky::CamNum, ASI_WB_R, settings.camera.asiWBR, settings.camera.asiAutoAWB == 1 ? ASI_TRUE : ASI_FALSE);
		setControl(Allsky::CamNum, ASI_WB_B, settings.camera.asiWBB, settings.camera.asiAutoAWB == 1 ? ASI_TRUE : ASI_FALSE);
	}
	setControl(Allsky::CamNum, ASI_GAMMA, Allsky::asiGamma, ASI_FALSE);
	setControl(Allsky::CamNum, ASI_FLIP, settings.image.asiFlip, ASI_FALSE);

	if (Allsky::ASICameraInfo.IsCoolerCam)
	{
		asiRetCode = setControl(Allsky::CamNum, ASI_COOLER_ON, Allsky::asiCoolerEnabled == 1 ? ASI_TRUE : ASI_FALSE, ASI_FALSE);
		if (asiRetCode != ASI_SUCCESS)
		{
			printf("%s", Allsky::c(KRED));
			printf(" WARNING: Could not enable cooler: %s, but continuing without it.\n", getRetCode(asiRetCode));
			printf("%s", Allsky::c(KNRM));
		}
		asiRetCode = setControl(Allsky::CamNum, ASI_TARGET_TEMP, Allsky::asiTargetTemp, ASI_FALSE);
		if (asiRetCode != ASI_SUCCESS)
		{
			printf("%s", Allsky::c(KRED));
			printf(" WARNING: Could not set cooler temperature: %s, but continuing without it.\n", getRetCode(asiRetCode));
			printf("%s", Allsky::c(KNRM));
		}
	}

	if (! Allsky::bSaveRun && pthread_create(&Allsky::hthdSave, 0, SaveImgThd, 0) == 0)
	{
		Allsky::bSaveRun = true;
	}

	// Display one-time messages.

	// If autogain is on, our adjustments to gain will get overwritten by the camera
	// so don't transition.
	// gainTransitionTime of 0 means don't adjust gain.
	// No need to adjust gain if day and night gain are the same.
	if (Allsky::asiDayAutoGain == 1 || settings.camera.asiNightAutoGain == 1 || Allsky::gainTransitionTime == 0 || settings.camera.asiDayGain == settings.camera.asiNightGain || settings.taking_dark_frames == 1)
	{
		Allsky::adjustGain = false;
		Allsky::Debug("Will NOT adjust gain at transitions\n");
	}
	else
	{
		Allsky::adjustGain = true;
		Allsky::Debug("Will adjust gain at transitions\n");
	}

	if (Allsky::ImgExtraText[0] != '\0' && Allsky::extraFileAge > 0) {
		Allsky::Warning("Extra Text File Age Disabled So Displaying Anyway\n");
	}

	if (settings.tty)
		printf("*** Press Ctrl+C to stop ***\n\n");
	else
		printf("*** Stop the allsky service to end this process. ***\n\n");


	// Start taking pictures

	if (! Allsky::use_new_exposure_algorithm)
	{
		asiRetCode = ASIStartVideoCapture(Allsky::CamNum);
		if (asiRetCode != ASI_SUCCESS)
		{
			printf("*** ERROR: Unable to start video capture: %s\n", getRetCode(asiRetCode));
			Allsky::closeUp(2);
		}
	}

}



void CameraZWO::setupForCapture() 
{
// todo
}

// Build capture command to capture the image from the camera
int CameraZWO::capture() 
{
	ASI_ERROR_CODE asiRetCode;
	asiRetCode = takeOneExposure(Allsky::CamNum, Allsky::current_exposure_us, settings.image.pRgb.data, settings.image.width, settings.image.height, (ASI_IMG_TYPE) settings.image.Image_type, histogram, &mean);
	if (asiRetCode == ASI_SUCCESS) {
				numErrors = 0;
				Allsky::numExposures++;

				if (Allsky::numExposures == 0 && settings.preview == 1)
				{
					// Start the preview thread at the last possible moment.
					Allsky::bDisplay = 1;
					pthread_create(&Allsky::thread_display, NULL, Display, (void *)&settings.image.pRgb);
				}

#ifdef USE_HISTOGRAM
				int usedHistogram = 0;	// did we use the histogram method?

				// We don't use this at night since the ZWO bug is only when it's light outside.
				if (runtime.dayOrNight == "DAY" && settings.camera.asiDayAutoExposure && ! settings.taking_dark_frames)
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
					long hist_min_exposure_us = Allsky::camera_min_exposure_us ? Allsky::camera_min_exposure_us : 100;
					long temp_min_exposure_us = hist_min_exposure_us;
					long temp_max_exposure_us = Allsky::current_max_autoexposure_us;

					if (settings.camera.asiDayBrightness != DEFAULT_BRIGHTNESS)
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
							numMultiples = (float)(settings.camera.asiDayBrightness - DEFAULT_BRIGHTNESS) / DEFAULT_BRIGHTNESS;
							exposureAdjustment = 1 + (numMultiples * adjustmentAmountPerMultiple);
							Allsky::Debug("  > >>> Adjusting exposure x %.2f (%.1f%%) for daybrightness\n", exposureAdjustment, (exposureAdjustment - 1) * 100);
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
						Allsky::Debug("  > !!! Adjusting %sAcceptableMean by %d to %d\n",
						   adjustment < 0 ? "min" : "max",
						   adjustment,
						   adjustment < 0 ? minAcceptableMean : maxAcceptableMean);
					}

					while ((mean < minAcceptableMean || mean > maxAcceptableMean) && ++attempts <= maxHistogramAttempts && Allsky::current_exposure_us <= Allsky::current_max_autoexposure_us)
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
							 new_exposure_us = Allsky::current_exposure_us * 25;
							 why = "< (minAcceptableMean * 0.04)";
							 num = minAcceptableMean * 0.04;
						 }
						 else if (mean < (minAcceptableMean * 0.1))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 7;
							 why = "< (minAcceptableMean * 0.1)";
							 num = minAcceptableMean * 0.1;
						 }
						 else if (mean < (minAcceptableMean * 0.3))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 4;
							 why = "< (minAcceptableMean * 0.3)";
							 num = minAcceptableMean * 0.3;
						 }
						 else if (mean < (minAcceptableMean * 0.6))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 2.5;
							 why = "< (minAcceptableMean * 0.6)";
							 num = minAcceptableMean * 0.6;
						 }
						 else if (mean < (minAcceptableMean * 0.8))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 1.8;
							 why = "< (minAcceptableMean * 0.8)";
							 num = minAcceptableMean * 0.8;
						 }
						 else if (mean < (minAcceptableMean * 1.0))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 1.05;
							 why = "< minAcceptableMean";
							 num = minAcceptableMean * 1.0;
						 }


						 else if (mean > (maxAcceptableMean * 1.89))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 0.4;
							 why = "> (maxAcceptableMean * 1.89)";
							 num = (maxAcceptableMean * 1.89);
						 }
						 else if (mean > (maxAcceptableMean * 1.6))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 0.7;
							 why = "> (maxAcceptableMean * 1.6)";
							 num = (maxAcceptableMean * 1.6);
						 }
						 else if (mean > (maxAcceptableMean * 1.3))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 0.85;
							 why = "> (maxAcceptableMean * 1.3)";
							 num = (maxAcceptableMean * 1.3);
						 }
						 else if (mean > (maxAcceptableMean * 1.0))
						 {
							 new_exposure_us = Allsky::current_exposure_us * 0.9;
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
e_us = Allsky::current_exposure_us;
e_us = last_exposure_us;
if (Allsky::current_exposure_us != last_exposure_us) printf("xxxxxxxxxxx current_exposure_us %'ld != last_exposure_us %'ld\n", Allsky::current_exposure_us, last_exposure_us);
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
							-last_mean_diff, minAcceptableMean, mean, Allsky::current_exposure_us);
						} 
						else
						{
							if (prior_mean_diff < 0 && last_mean_diff > 0)
							{
							// OK to set upper limit since we know it's too high.
printf(" >xxx mean was %d and went from %d below min of %d to %d above max of %d, is now at %d; OK to set temp max to current_exposure_us of %'ld\n",
								prior_mean, -prior_mean_diff, minAcceptableMean,
								last_mean_diff, maxAcceptableMean, mean, Allsky::current_exposure_us);
							}

							if (mean < minAcceptableMean)
							{
								temp_min_exposure_us = Allsky::current_exposure_us;
							} 
							else if (mean > maxAcceptableMean)
							{
								temp_max_exposure_us = Allsky::current_exposure_us;
							} 
						} 

						 new_exposure_us = roundTo(new_exposure_us, roundToMe);
						 new_exposure_us = std::max(temp_min_exposure_us, new_exposure_us);
						 new_exposure_us = std::min(temp_max_exposure_us, new_exposure_us);
						 new_exposure_us = std::min(Allsky::current_max_autoexposure_us, new_exposure_us);

						 if (new_exposure_us == Allsky::current_exposure_us)
						 {
							 break;
						 }

						 Allsky::current_exposure_us = new_exposure_us;
						 if (Allsky::current_exposure_us > Allsky::current_max_autoexposure_us)
						 {
							 break;
						 }

						 Allsky::Debug("  >> Retry %i @ %'ld us, min=%'ld us, max=%'ld us: mean (%d) %s (%d)\n", attempts, new_exposure_us, temp_min_exposure_us, temp_max_exposure_us, mean, why.c_str(), num);

						 prior_mean = mean;
						 prior_mean_diff = last_mean_diff;

						 asiRetCode = takeOneExposure(Allsky::CamNum, Allsky::current_exposure_us, settings.image.pRgb.data, settings.image.width, settings.image.height, (ASI_IMG_TYPE) settings.image.Image_type, histogram, &mean);
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
							if (! check_max_errors(&Allsky::exitCode, maxErrors))
							{
								Allsky::closeUp(Allsky::exitCode);
							}
							break;
						 }
					} // end of "Retry" loop

					if (asiRetCode != ASI_SUCCESS)
					{
						Allsky::Info("  > Sleeping %s from failed exposure\n", Allsky::length_in_units(Allsky::currentDelay_ms * US_IN_MS, false));
						usleep(Allsky::currentDelay_ms * US_IN_MS);
						// Don't save the file or do anything below.
						continue;
					}

					if (mean >= minAcceptableMean && mean <= maxAcceptableMean)
					{
						// +++ at end makes it easier to see in log file
						Allsky::Debug("  > Good image: mean within range of %d to %d ++++++++++, mean %d\n", minAcceptableMean, maxAcceptableMean, mean);
					}
					else if (attempts > maxHistogramAttempts)
					{
						 Allsky::Debug("  > max attempts reached - using exposure of %s us with mean %d\n", Allsky::length_in_units(Allsky::current_exposure_us, true), mean);
					}
					else if (attempts >= 1)
					{
						 if (Allsky::current_exposure_us > Allsky::current_max_autoexposure_us)
						 {
							 Allsky::Debug("  > Stopped trying: new exposure of %s would be over max of %s\n", Allsky::length_in_units(Allsky::current_exposure_us, false), Allsky::length_in_units(Allsky::current_max_autoexposure_us, false));

							 long diff = (long)((float)Allsky::current_exposure_us * (1/(float)percent_change));
							 Allsky::current_exposure_us -= diff;
							 Allsky::Debug("  > Decreasing next exposure by %d%% (%'ld us) to %'ld\n", percent_change, diff, Allsky::current_exposure_us);
						 }
						 else if (Allsky::current_exposure_us == Allsky::current_max_autoexposure_us)
						 {
							 Allsky::Debug("  > Stopped trying: hit max exposure limit of %s, mean %d\n", Allsky::length_in_units(Allsky::current_max_autoexposure_us, false), mean);
							 // If current_exposure_us causes too high of a mean, decrease exposure
							 // so on the next loop we'll adjust it.
							 if (mean > maxAcceptableMean)
								 Allsky::current_exposure_us--;
						 }
						 else if (new_exposure_us == Allsky::current_exposure_us)
						 {
							 Allsky::Debug("  > Stopped trying: new_exposure_us == current_exposure_us == %s\n", Allsky::length_in_units(Allsky::current_exposure_us, false));
						 }
						 else
						 {
							 Allsky::Debug("  > Stopped trying, using exposure of %s us with mean %d, min=%d, max=%d\n", Allsky::length_in_units(Allsky::current_exposure_us, false), mean, minAcceptableMean, maxAcceptableMean);
						 }
					}
					else if (Allsky::current_exposure_us == Allsky::current_max_autoexposure_us)
					{
						 Allsky::Debug("  > Did not make any additional attempts - at max exposure limit of %s, mean %d\n", Allsky::length_in_units(Allsky::current_max_autoexposure_us, false), mean);
					}
					// xxxx TODO: this was "actual_exposure_us = ..."    reported_exposure_us = current_exposure_us;

				} else {
					// Didn't use histogram method.
					// If we used auto-exposure, set the next exposure to the last reported exposure, which is what.
					// the camera driver thinks the next exposure should be.
					if (Allsky::currentAutoExposure == ASI_TRUE)
						Allsky::current_exposure_us = reported_exposure_us;
					else
						Allsky::current_exposure_us = last_exposure_us;
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
						Allsky::Info("  >>>> Skipping this frame\n");
						Allsky::current_skip_frames--;
						// Do not save this frame or sleep after it.
						// We just started taking images so no need to check if DAY or NIGHT changed
						return(0); // todo check this
					}
				}

				// Write temperature to file
				writeTemperatureToFile((float)Allsky::actualTemp / 10.0);

				// If taking_dark_frames is off, add overlay text to the image
				if (! settings.taking_dark_frames)
				{
					int iYOffset = 0;
					Allsky::overlayText(iYOffset);

					if (Allsky::currentAdjustGain)
					{
						// Determine if we need to change the gain on the next image.
						// This must come AFTER the "showGain" above.
						Allsky::gainChange = determineGainChange(settings.camera.asiDayGain, settings.camera.asiNightGain);
						setControl(Allsky::CamNum, ASI_GAIN, Allsky::currentGain + Allsky::gainChange, (ASI_BOOL)Allsky::currentAutoGain);
					}

#ifdef USE_HISTOGRAM
					if (showHistogram)
					{
						sprintf(Allsky::bufTemp, "Mean: %d", mean);
						Allsky::cvText(settings.image.pRgb, Allsky::bufTemp, Allsky::iTextX, Allsky::iTextY + (iYOffset / Allsky::currentBin),
							settings.fonts.fontsize * SMALLFONTSIZE_MULTIPLIER, Allsky::linewidth,
							Allsky::linetype[Allsky::linenumber], Allsky::fontname[settings.fonts.fontnumber], Allsky::smallFontcolor,
							settings.image.Image_type, Allsky::outlinefont);
						iYOffset += Allsky::iTextLineHeight;
					}
					if (showHistogramBox && usedHistogram)
					{
						// Draw a rectangle where the histogram box is.
						// Put a black and white line one next to each other so they
						// can be seen in light and dark images.
						int lt = cv::LINE_AA, thickness = 2;
						int X1 = (settings.image.width * histogramBoxPercentFromLeft) - (histogramBoxSizeX / 2);
						int X2 = X1 + histogramBoxSizeX;
						int Y1 = (settings.image.height * histogramBoxPercentFromTop) - (histogramBoxSizeY / 2);
						int Y2 = Y1 + histogramBoxSizeY;
						cv::Scalar outer_line, inner_line;
// xxxxxxx  TODO: can we use Scalar(x,y,z) for both?
						if (1 || settings.image.Image_type == ASI_IMG_RAW16)
						{
							outer_line = cv::Scalar(0,0,0);
							inner_line = cv::Scalar(255,255,255);
						}
						else
						{
							outer_line = cv::Scalar(0,0,0, 255);
							inner_line = cv::Scalar(255,255,255, 255);
						}
						cv::rectangle(settings.image.pRgb, cv::Point(X1, Y1), cv::Point(X2, Y2), outer_line,  thickness, lt, 0);
						cv::rectangle(settings.image.pRgb, cv::Point(X1+thickness, Y1+thickness), cv::Point(X2-thickness, Y2-thickness), inner_line,  thickness, lt, 0);
					}
#endif
				}

#ifndef USE_HISTOGRAM
				if (Allsky::currentAutoExposure == ASI_TRUE)
				{
					// Retrieve the current Exposure for smooth transition to night time
					// as long as auto-exposure is enabled during night time
					Allsky::current_exposure_us = last_exposure_us;
				}
#endif

				// Save the image
				if (! Allsky::bSavingImg)
				{
					pthread_mutex_lock(&Allsky::mtx_SaveImg);
					pthread_cond_signal(&Allsky::cond_SatrtSave);
					pthread_mutex_unlock(&Allsky::mtx_SaveImg);
				}
				else
				{
					// Hopefully the user can use the time it took to save a file to disk
					// to help determine why they are getting this warning.
					// Perhaps their disk is very slow or their delay is too short.
					Allsky::Warning("  > WARNING: currently saving an image; can't save new one at %s.\n", Allsky::exposureStart);

					// TODO: wait for the prior image to finish saving.
				}

				if (settings.camera.asiNightAutoGain == 1 && runtime.dayOrNight == "NIGHT" && ! settings.taking_dark_frames)
				{
					ASIGetControlValue(Allsky::CamNum, ASI_GAIN, &actualGain, &bAuto);
					Allsky::Warning("  > Auto Gain value: %ld\n", actualGain);
				}

				if (Allsky::currentAutoExposure == ASI_TRUE)
				{
#ifndef USE_HISTOGRAM

					if (runtime.dayOrNight == "DAY")
					{
						Allsky::current_exposure_us = last_exposure_us;
					}
#endif

					// Delay applied before next exposure
					if (runtime.dayOrNight == "NIGHT" && settings.camera.asiNightAutoExposure == 1 && last_exposure_us < (Allsky::asi_night_max_autoexposure_ms * US_IN_MS) && ! settings.taking_dark_frames)
					{
						// If using auto-exposure and the actual exposure is less than the max,
						// we still wait until we reach maxexposure, then wait for the delay period.
						// This is important for a constant frame rate during timelapse generation.
						// This doesn't apply during the day since we don't have a max time then.
						long s_us = (Allsky::asi_night_max_autoexposure_ms * US_IN_MS) - last_exposure_us; // to get to max
						s_us += Allsky::currentDelay_ms * US_IN_MS;   // Add standard delay amount
						Allsky::Debug("  > Sleeping: %s\n", Allsky::length_in_units(s_us, false));
						usleep(s_us);	// usleep() is in us (microseconds)
					}
					else
					{
						// Sleep even if taking dark frames so the sensor can cool between shots like it would
						// do on a normal night.  With no delay the sensor may get hotter than it would at night.
						Allsky::Debug("  > Sleeping %s from %s exposure\n", Allsky::length_in_units(Allsky::currentDelay_ms * US_IN_MS, false), settings.taking_dark_frames ? "dark frame" : "auto");
						usleep(Allsky::currentDelay_ms * US_IN_MS);
					}
				}
				else
				{
					std::string s;
				   if (settings.taking_dark_frames)
						s = "dark frame";
				   else
						s = "manual";
#ifdef USE_HISTOGRAM
					if (usedHistogram == 1)
						s = "histogram";
#endif
					Allsky::Debug("  > Sleeping %s from %s exposure\n", Allsky::length_in_units(Allsky::currentDelay_ms * US_IN_MS, false), s.c_str());
					usleep(Allsky::currentDelay_ms * US_IN_MS);
				}
				calculateDayOrNight();

	} else {
		return 1;
	}
	// todo maxerrors, retcode
	return (asiRetCode == ASI_SUCCESS)?0:1;
}

ASI_ERROR_CODE CameraZWO::takeOneExposure(
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
		Allsky::Warning("*** WARNING: exposure_time_us requested [%s] > current_max_autoexposure_us [%s]\n", x, Allsky::length_in_units(Allsky::current_max_autoexposure_us, true));
		exposure_time_us = Allsky::current_max_autoexposure_us;
	}

	// This debug message isn't typcally needed since we already displayed a message about
	// starting a new exposure, and below we display the result when the exposure is done.
	Allsky::Trace("  > %s to %s, timeout: %'ld ms\n",
		wasAutoExposure == ASI_TRUE ? "Camera set auto-exposure" : "Exposure set",
		Allsky::length_in_units(exposure_time_us, true), timeout);

	setControl(cameraId, ASI_EXPOSURE, exposure_time_us, (ASI_BOOL)Allsky::currentAutoExposure);

	flush_buffered_image(cameraId, imageBuffer, Allsky::bufferSize);

	if (Allsky::use_new_exposure_algorithm)
	{
		status = ASIStartVideoCapture(cameraId);
	} else {
		status = ASI_SUCCESS;
	}

	if (status == ASI_SUCCESS) {
		status = ASIGetVideoData(cameraId, imageBuffer, Allsky::bufferSize, timeout);
		if (status != ASI_SUCCESS)
		{
			Allsky::Error("  > ERROR: Failed getting image: %s\n", getRetCode(status));
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
			Allsky::Debug("  > Got image%s.  Reported exposure: %s, auto=%s\n", Allsky::debugText, Allsky::length_in_units(reported_exposure_us, true), wasAutoExposure == ASI_TRUE ? "yes" : "no");

			// If this was a manual exposure, make sure it took the correct exposure.
			// Per ZWO, this should never happen.
			if (wasAutoExposure == ASI_FALSE && exposure_time_us != reported_exposure_us)
			{
				Allsky::Warning("  > WARNING: not correct exposure (requested: %'ld us, reported: %'ld us, diff: %'ld)\n", exposure_time_us, reported_exposure_us, reported_exposure_us - exposure_time_us);
			}
			ASIGetControlValue(cameraId, ASI_GAIN, &actualGain, &bAuto);
			ASIGetControlValue(cameraId, ASI_TEMPERATURE, &Allsky::actualTemp, &bAuto);
		}

		if (Allsky::use_new_exposure_algorithm)
			ASIStopVideoCapture(cameraId);

	}
	else {
		Allsky::Error("  > ERROR: Not fetching exposure data because status is %s\n", getRetCode(status));
	}

	return status;
}

void  CameraZWO::postCapture(void)
{
	// todo
}

// Display ASI errors in human-readable format
char * CameraZWO::getRetCode(ASI_ERROR_CODE code)
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

int CameraZWO::bytesPerPixel(ASI_IMG_TYPE imageType) {
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

int CameraZWO::computeHistogram(unsigned char *imageBuffer, int width, int height, ASI_IMG_TYPE imageType, int *histogram)
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

// Make sure we don't try to update a non-updateable control, and check for errors.
ASI_ERROR_CODE CameraZWO::setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto)
{
	ASI_ERROR_CODE ret = ASI_SUCCESS;
	int i;
	for (i = 0; i < iNumOfCtrl && i <= control; i++)    // controls are sorted 1 to n
	{
		ret = ASIGetControlCaps(CamNum, i, &Allsky::ControlCaps);
		if (ret != ASI_SUCCESS)
		{
			Allsky::Warning("WARNING: ASIGetControlCaps() for control %d failed: %s\n", i, getRetCode(ret));
			return(ret);
		}

		if (Allsky::ControlCaps.ControlType == control)
		{
			if (Allsky::ControlCaps.IsWritable)
			{
				if (value > Allsky::ControlCaps.MaxValue)
				{
					printf("WARNING: Value of %ld greater than max value allowed (%ld) for control '%s' (#%d).\n", value, Allsky::ControlCaps.MaxValue, Allsky::ControlCaps.Name, Allsky::ControlCaps.ControlType);
					value = Allsky::ControlCaps.MaxValue;
				} else if (value < Allsky::ControlCaps.MinValue)
				{
					printf("WARNING: Value of %ld less than min value allowed (%ld) for control '%s' (#%d).\n", value, Allsky::ControlCaps.MinValue, Allsky::ControlCaps.Name, Allsky::ControlCaps.ControlType);
					value = Allsky::ControlCaps.MinValue;
				}
				if (makeAuto == ASI_TRUE && Allsky::ControlCaps.IsAutoSupported == ASI_FALSE)
				{
					printf("WARNING: control '%s' (#%d) doesn't support auto mode.\n", Allsky::ControlCaps.Name, Allsky::ControlCaps.ControlType);
					makeAuto = ASI_FALSE;
				}
				ret = ASISetControlValue(CamNum, control, value, makeAuto);
				if (ret != ASI_SUCCESS)
				{
					Allsky::Warning("WARNING: ASISetControlCaps() for control %d, value=%ld failed: %s\n", control, value, getRetCode(ret));
					return(ret);
				}
			} else {
				printf("ERROR: ControlCap: '%s' (#%d) not writable; not setting to %ld.\n", Allsky::ControlCaps.Name, Allsky::ControlCaps.ControlType, value);
				ret = ASI_ERROR_INVALID_MODE;	// this seemed like the closest error
			}
			return ret;
		}
	}
	Allsky::Debug("NOTICE: Camera does not support ControlCap # %d; not setting to %ld.\n", control, value);
	return ASI_ERROR_INVALID_CONTROL_TYPE;
}

// Determine the change in gain needed for smooth transitions between night and day.
// Gain during the day is usually 0 and at night is usually > 0.
// If auto-exposure is on for both, the first several night frames may be too bright at night
// because of the sudden (often large) increase in gain, or too dark at the night-to-day
// transition.
// Try to mitigate that by changing the gain over several images at each transition.

int CameraZWO::determineGainChange(int dayGain, int nightGain)
{
	if (Allsky::numGainChanges > Allsky::gainTransitionImages || Allsky::totalAdjustGain == 0)
	{
		// no more changes needed in this transition
		Allsky::Trace("  xxxx No more gain changes needed.\n");
		Allsky::currentAdjustGain = false;
		return(0);
	}

	Allsky::numGainChanges++;
	int amt;	// amount to adjust gain on next picture
	if (runtime.dayOrNight == "DAY")
	{
		// During DAY, want to start out adding the full gain adjustment minus the increment on the first image,
		// then DECREASE by totalAdjustGain each exposure.
		// This assumes night gain is > day gain.
		amt = Allsky::totalAdjustGain - (Allsky::perImageAdjustGain * Allsky::numGainChanges);
		if (amt < 0)
		{
			amt = 0;
			Allsky::totalAdjustGain = 0;	// we're done making changes
		}
	}
	else	// NIGHT
	{
		// During NIGHT, want to start out (nightGain-perImageAdjustGain),
		// then DECREASE by perImageAdjustGain each time, until we get to "nightGain".
		// This last image was at dayGain and we wen't to increase each image.
		amt = (Allsky::perImageAdjustGain * Allsky::numGainChanges) - Allsky::totalAdjustGain;
		if (amt > 0)
		{
			amt = 0;
			Allsky::totalAdjustGain = 0;	// we're done making changes
		}
	}

	Allsky::Trace("  xxxx Adjusting %s gain by %d on next picture to %d; will be gain change # %d of %d.\n",
		runtime.dayOrNight.c_str(), amt, amt+Allsky::currentGain, Allsky::numGainChanges, Allsky::gainTransitionImages);
	return(amt);
}

// Reset the gain transition variables for the first transition image.
// This is called when the program first starts and at the beginning of every day/night transition.
// "dayOrNight" is the new value, e.g., if we just transitioned from day to night, it's "NIGHT".
bool CameraZWO::resetGainTransitionVariables(int dayGain, int nightGain)
{
	// Many of the "xxx" messages below will go away once we're sure gain transition works.
	Allsky::Trace("xxx resetGainTransitionVariables(%d, %d) called at %s\n", dayGain, nightGain, runtime.dayOrNight.c_str());

	if (Allsky::adjustGain == false)
	{
		// determineGainChange() will never be called so no need to set any variables.
		Allsky::Trace("xxx will not adjust gain - adjustGain == false\n");
		return(false);
	}

	if (Allsky::numExposures == 0)
	{
		// we don't adjust when the program first starts since there's nothing to transition from
		Allsky::Trace("xxx will not adjust gain right now - numExposures == 0\n");
		return(false);
	}

	// Determine the amount to adjust gain per image.
	// Do this once per day/night or night/day transition (i.e., numGainChanges == 0).
	// First determine how long an exposure and delay is, in seconds.
	// The user specifies the transition period in seconds,
	// but day exposure is in microseconds, night max is in milliseconds,
	// and delays are in milliseconds, so convert to seconds.
	float totalTimeInSec;
	if (runtime.dayOrNight == "DAY")
	{
		totalTimeInSec = (Allsky::asi_day_exposure_us / US_IN_SEC) + (settings.day.dayDelay_ms / MS_IN_SEC);
		Allsky::Trace("xxx totalTimeInSec=%.1fs, asi_day_exposure_us=%'ldus , daydelay_ms=%'dms\n", totalTimeInSec, Allsky::asi_day_exposure_us, settings.day.dayDelay_ms);
	}
	else	// NIGHT
	{
		// At nightime if the exposure is less than the max, we wait until max has expired,
		// so use it instead of the exposure time.
		totalTimeInSec = (Allsky::asi_night_max_autoexposure_ms / MS_IN_SEC) + (settings.night.nightDelay_ms / MS_IN_SEC);
		Allsky::Trace("xxx totalTimeInSec=%.1fs, asi_night_max_autoexposure_ms=%'dms, nightDelay_ms=%'dms\n", totalTimeInSec, Allsky::asi_night_max_autoexposure_ms, settings.night.nightDelay_ms);
	}

	Allsky::gainTransitionImages = ceil(Allsky::gainTransitionTime / totalTimeInSec);
	if (Allsky::gainTransitionImages == 0)
	{
		Allsky::Info("*** INFORMATION: Not adjusting gain - your 'gaintransitiontime' (%d seconds) is less than the time to take one image plus its delay (%.1f seconds).\n", Allsky::gainTransitionTime, totalTimeInSec);
		return(false);
	}

	Allsky::totalAdjustGain = nightGain - dayGain;
	Allsky::perImageAdjustGain = ceil(Allsky::totalAdjustGain / Allsky::gainTransitionImages);	// spread evenly
	if (Allsky::perImageAdjustGain == 0)
		Allsky::perImageAdjustGain = Allsky::totalAdjustGain;
	else
	{
		// Since we can't adust gain by fractions, see if there's any "left over" after gainTransitionImages.
		// For example, if totalAdjustGain is 7 and we're adjusting by 3 each of 2 times,
		// we need an extra transition to get the remaining 1 ((7 - (3 * 2)) == 1).
		if (Allsky::gainTransitionImages * Allsky::perImageAdjustGain < Allsky::totalAdjustGain)
			Allsky::gainTransitionImages++;		// this one will get the remaining amount
	}

	Allsky::Trace("xxx gainTransitionImages=%d, gainTransitionTime=%ds, perImageAdjustGain=%d, totalAdjustGain=%d\n",
		Allsky::gainTransitionImages, Allsky::gainTransitionTime, Allsky::perImageAdjustGain,Allsky::totalAdjustGain);

	return(true);
}

void * CameraZWO::SaveImgThd(void *para)
{
	while (Allsky::bSaveRun)
	{
		pthread_mutex_lock(&Allsky::mtx_SaveImg);
		pthread_cond_wait(&Allsky::cond_SatrtSave, &Allsky::mtx_SaveImg);

		if (runtime.gotSignal)
		{
			// we got a signal to exit, so don't save the (probably incomplete) image
			pthread_mutex_unlock(&Allsky::mtx_SaveImg);
			break;
		}

		Allsky::bSavingImg = true;

		Allsky::Warning("  > Saving %s image '%s'\n", settings.taking_dark_frames ? "dark" : runtime.dayOrNight.c_str(), settings.image.fileName);
		int64 st, et;

		bool result = false;
		if (settings.image.pRgb.data)
		{
			const char *s;	// TODO: use saveImage.sh
			if (runtime.dayOrNight == "NIGHT")
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
			sprintf(cmd, "%s %s '%s' '%2.0f' %ld &", s, runtime.dayOrNight.c_str(), settings.image.fileName, (float) Allsky::actualTemp/10, Allsky::current_exposure_us);
			st = cv::getTickCount();
			try
			{
				result = imwrite(settings.image.fileName, settings.image.pRgb, Allsky::compression_parameters);
			}
			catch (const cv::Exception& ex)
			{
				printf("*** ERROR: Exception saving image: %s\n", ex.what());
			}
			et = cv::getTickCount();

			if (result)
				system(cmd);
			else
				printf("*** ERROR: Unable to save image '%s'.\n", settings.image.fileName);

		} else {
			// This can happen if the program is closed before the first picture.
			Allsky::Info("----- SaveImgThd(): pRgb.data is null\n");
		}
		Allsky::bSavingImg = false;

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
			Allsky::Trace("%s  > Image took %'.1f ms to save (average %'.1f ms).\n%s", x, diff, total_time_ms / total_saves, x);
		}

		pthread_mutex_unlock(&Allsky::mtx_SaveImg);
	}

	return (void *)0;
}

double CameraZWO::time_diff_us(int64 start, int64 end)
{
	double frequency = cv::getTickFrequency();
	return (double)(end - start) / frequency;	// in Microseconds
}

// Check if the maximum number of consecutive errors has been reached
bool CameraZWO::check_max_errors(int *e, int max_errors)
{
	// Once takeOneExposure() fails with a timeout, it seems to always fail,
	// even with extremely large timeout values, so apparently ASI_ERROR_TIMEOUT doesn't
	// necessarily mean it's timing out.  Exit which will cause us to be restarted.
	numErrors++; sleep(2);
	if (numErrors >= max_errors)
	{
		*e = 99;		// exit code - needs to match what's in allsky.sh
		Allsky::Error("*** ERROR: Maximum number of consecutive errors of %d reached; exiting...\n", max_errors);
		return(false);	// gets us out of inner and outer loop
	}
	return(true);
}

void CameraZWO::writeTemperatureToFile(float val)
{
	std::ofstream outfile;
	outfile.open("temperature.txt", std::ios_base::trunc);
	outfile << val;
	outfile << "\n";
}

void *CameraZWO::Display(void *params)
{
	cv::Mat *pImg = (cv::Mat *)params;
	int w = pImg->cols;
	int h = pImg->rows;
	cv::namedWindow("Preview", cv::WINDOW_AUTOSIZE);
	cv::Mat Img2 = *pImg, *pImg2 = &Img2;

	while (Allsky::bDisplay)
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

long CameraZWO::roundTo(long n, int roundTo)
{
	long a = (n / roundTo) * roundTo;	// Smaller multiple
	long b = a + roundTo;				// Larger multiple
	return (n - a > b - n)? b : a;		// Return of closest of two
}
