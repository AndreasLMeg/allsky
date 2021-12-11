#include <stdio.h>
#include <sys/time.h>
#include <tr1/memory>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef CAM_RPIHQ
#else
	#include "include/ASICamera2.h"
#endif

#include "allsky.h"


/**
 * Text_overlay
**/
void Allsky::overlayText(int &iYOffset) {

	if (showTime == 1)
	{
		// The time and ImgText are in the larger font; everything else is in smaller font.
		cvText(pRgb, bufTime, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * 0.1, linewidth,
			linetype[linenumber], fontname[fontnumber], fontcolor,
			Image_type, outlinefont);
			iYOffset += iTextLineHeight;
	}

	if (ImgText[0] != '\0')
	{
		cvText(Allsky::pRgb, ImgText, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * 0.1, linewidth,
			linetype[linenumber], fontname[fontnumber], fontcolor,
			Image_type, outlinefont);
		iYOffset+=iTextLineHeight;
	}

	if (showTemp == 1)
	{
		char C[20] = { 0 }, F[20] = { 0 };
		if (strcmp(tempType, "C") == 0 || strcmp(tempType, "B") == 0)
		{
			sprintf(C, "  %.0fC", (float)actualTemp / 10);
		}
		if (strcmp(tempType, "F") == 0 || strcmp(tempType, "B") == 0)
		{
			sprintf(F, "  %.0fF", (((float)actualTemp / 10 * 1.8) + 32));
		}
		sprintf(bufTemp, "Sensor: %s %s", C, F);
		cvText(pRgb, bufTemp, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
			linetype[linenumber], fontname[fontnumber], smallFontcolor,
			Image_type, outlinefont);
		iYOffset += iTextLineHeight;
	}

	if (showExposure == 1)
	{
		// display in seconds if >= 1 second, else in ms
		if (currentExposure_us >= (1 * US_IN_SEC))
			sprintf(bufTemp, "Exposure: %'.2f s%s", (float)currentExposure_us / US_IN_SEC, bufTemp2);
		else
			sprintf(bufTemp, "Exposure: %'.2f ms%s", (float)currentExposure_us / US_IN_MS, bufTemp2);
		// Indicate if in auto-exposure mode.
		if (currentAutoExposure == ASI_TRUE) strcat(bufTemp, " (auto)");
		cvText(pRgb, bufTemp, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
			linetype[linenumber], fontname[fontnumber], smallFontcolor,
			Image_type, outlinefont);
		iYOffset += iTextLineHeight;
	}

	if (showGain == 1)
	{
		sprintf(bufTemp, "Gain: %1.2f", lastGain);
		// Indicate if in auto gain mode.
		if (currentAutoGain == ASI_TRUE) strcat(bufTemp, " (auto)");
		cvText(pRgb, bufTemp, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
			linetype[linenumber], fontname[fontnumber], smallFontcolor,
			Image_type, outlinefont);
		iYOffset += iTextLineHeight;
	}

	if (showBrightness == 1)
	{
		sprintf(bufTemp, "Brightness: %d", currentBrightness);
		cvText(pRgb, bufTemp, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
			linetype[linenumber], fontname[fontnumber], smallFontcolor,
			Image_type, outlinefont);
		iYOffset += iTextLineHeight;
	}

#ifdef CAM_RPIHQ
	if (showMean == 1 && myModeMeanSetting.mode_mean)
	{
		sprintf(bufTemp, "Mean: %.6f", lastMean);
		cvText(pRgb, bufTemp, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
			linetype[linenumber], fontname[fontnumber], smallFontcolor,
			Image_type, outlinefont);
		iYOffset += iTextLineHeight;
	}
#endif

#ifdef CAM_RPIHQ
	if (showFocus == 1)
	{
		sprintf(bufTemp, "Focus: %.2f", get_focus_measure(pRgb, myModeMeanSetting));
		cvText(pRgb, bufTemp, iTextX, iTextY + (iYOffset / currentBin),
			fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
			linetype[linenumber], fontname[fontnumber], smallFontcolor,
			Image_type, outlinefont);
		iYOffset += iTextLineHeight;
	}
#endif

	/**
	* Optionally display extra text which is read from the provided file. If the
	* age of the file exceeds the specified limit then ignore the file.
	* This prevents situations where the program updating the file stops working.
	**/
	if (ImgExtraText[0] != '\0') {
		// Display these messages every time, since it's possible the user will
		// correct the issue while we're running.
		if (access(ImgExtraText, F_OK ) == -1 ) {
			Log(1, "  > *** WARNING: Extra Text File Does Not Exist So Ignoring It\n");
		} else if (access(ImgExtraText, R_OK ) == -1 ) {
			Log(1, "  > *** WARNING: Cannot Read From Extra Text File So Ignoring It\n");
		} else {
			FILE *fp = fopen(ImgExtraText, "r");

			if (fp != NULL) {
				bool bAddExtra = false;
				if (extraFileAge > 0) {
					struct stat buffer;
					if (stat(ImgExtraText, &buffer) == 0) {
						struct tm modifiedTime = *localtime(&buffer.st_mtime);

						time_t now = time(NULL);
						double ageInSeconds = difftime(now, mktime(&modifiedTime));
						Log(4, "  > Extra Text File (%s) Modified %.1f seconds ago", ImgExtraText, ageInSeconds);
						if (ageInSeconds < extraFileAge) {
							Log(1, ", so Using It\n");
							bAddExtra = true;
						} else {
							Log(1, ", so Ignoring\n");
						}
					} else {
						Log(0, "  > *** ERROR: Stat Of Extra Text File Failed !\n");
					}
				} else {
					bAddExtra = true;
				}

				if (bAddExtra) {
					char *line = NULL;
					size_t len = 0;
					int slen = 0;
					while (getline(&line, &len, fp) != -1) {
						slen = strlen(line);
						if (slen >= 2 && (line[slen-2] == 10 || line[slen-2] == 13)) {  // LF, CR
							line[slen-2] = '\0';
						} else if (slen >= 1 && (line[slen-1] == 10 || line[slen-1] == 13)) {
							line[slen-1] = '\0';
						}

						cvText(pRgb, line, iTextX, iTextY + (iYOffset / currentBin),
							fontsize * SMALLFONTSIZE_MULTIPLIER, linewidth,
							linetype[linenumber], fontname[fontnumber],
							smallFontcolor, Image_type, outlinefont);
						iYOffset += iTextLineHeight;
					}
				}
				fclose(fp);
			} else {
				Log(0, "  > *** WARNING: Failed To Open Extra Text File\n");
			}
		}
	}
}

void Allsky::cvText(cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname,
						int fontcolor[], int imgtype, int outlinefont)
{
	int outline_size = std::max(2.0, (fontsize/4));	// need smaller outline when font size is smaller
	if (imgtype == ASI_IMG_RAW16)
	{
		unsigned long fontcolor16 = createRGB(fontcolor[2], fontcolor[1], fontcolor[0]);
		if (outlinefont)
			cv::putText(img, text, cv::Point(x, y), fontname, fontsize, cv::Scalar(0,0,0), linewidth+outline_size, linetype);
		cv::putText(img, text, cv::Point(x, y), fontname, fontsize, fontcolor16, linewidth, linetype);
	}
	else
	{
		if (outlinefont)
			cv::putText(img, text, cv::Point(x, y), fontname, fontsize, cv::Scalar(0,0,0, 255), linewidth+outline_size, linetype);
		cv::putText(img, text, cv::Point(x, y), fontname, fontsize,
			cv::Scalar(fontcolor[0], fontcolor[1], fontcolor[2], 255), linewidth, linetype);
	}
}

// Create Hex value from RGB
unsigned long Allsky::createRGB(int r, int g, int b)
{
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

/**
 * Helper function to display debug info
**/
// [[gnu::format(printf, 2, 3)]]
void Allsky::Log(int required_level, const char *fmt, ...)
{
	if (debugLevel >= required_level) {
		char msg[8192];
		snprintf(msg, sizeof(msg), "%s", fmt);
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, msg, va);
		va_end(va);
	}
}

// Display a length of time in different units, depending on the length's value.
// If the "multi" flag is set, display in multiple units if appropriate.
char *Allsky::length_in_units(long us, bool multi)	// microseconds
{
	const int l = 50;
	static char length[l];
	if (us == 0)
	{
		snprintf(length, l, "0 us");
	}
	else
	{
		double us_in_ms = (double)us / US_IN_MS;
		// The boundaries on when to display one or two units are really a matter of taste.
		if (us_in_ms < 0.5)						// less than 0.5 ms
		{
			snprintf(length, l, "%'ld us", us);
		}
		else if (us_in_ms < 1.5)				// between 0.5 and 1.5 ms
		{
			if (multi)
				snprintf(length, l, "%'ld us (%.3f ms)", us, us_in_ms);
			else
				snprintf(length, l, "%'ld us", us);
		}
		else if (us_in_ms < (0.5 * MS_IN_SEC))	// 1.5 ms to 0.5 sec
		{
			if (multi)
				snprintf(length, l, "%.2f ms (%.2lf sec)", us_in_ms, (double)us / US_IN_SEC);
			else
				snprintf(length, l, "%.2f ms", us_in_ms);
		}
		else if (us_in_ms < (1.0 * MS_IN_SEC))	// between 0.5 sec and 1 sec
		{
			if (multi)
				snprintf(length, l, "%.2f ms (%.2lf sec)", us_in_ms, (double)us / US_IN_SEC);
			else
				snprintf(length, l, "%.1f ms", us_in_ms);
		}
		else									// over 1 sec
		{
			snprintf(length, l, "%.1lf sec", (double)us / US_IN_SEC);
		}

	}
	return(length);
}

// A user error was found.  Wait for the user to fix it.
void Allsky::waitToFix(char const *msg)
{
	printf("**********\n");
	printf("%s\n", msg);
	printf("*** After fixing, ");
	if (tty)
		printf("restart allsky.sh.\n");
	else
		printf("restart the allsky service.\n");
	if (notificationImages)
		system("scripts/copy_notification_image.sh Error &");
	sleep(5);	// give time for image to be copied
	printf("*** Sleeping until you fix the problem.\n");
	printf("**********\n");
	sleep(100000);	// basically, sleep forever until the user fixes this.
}

void Allsky::init(int argc, char *argv[])
{
	int help = 0;
	int i;


	setlinebuf(stdout);   // Line buffer output so entries appear in the log immediately.
	if (setlocale(LC_NUMERIC, locale) == NULL)
		printf("*** WARNING: Could not set locale to %s ***\n", locale);

	printf("\n%s", c(KGRN));
	printf("**********************************************\n");
	printf("*** Allsky Camera Software v0.8.2c |  2021 ***\n");
	printf("**********************************************\n\n");
	printf("Capture images of the sky with a Raspberry Pi and an ASI Camera\n");
	printf("%s\n", c(KNRM));
	printf("%sAdd -h or --help for available options%s\n\n", c(KYEL), c(KNRM));
	printf("Author: Thomas Jacquin - <jacquin.thomas@gmail.com>\n\n");
	printf("Contributors:\n");
	printf(" -Knut Olav Klo\n");
	printf(" -Daniel Johnsen\n");
	printf(" -Yang and Sam from ZWO\n");
	printf(" -Robert Wagner\n");
	printf(" -Michael J. Kidd - <linuxkidd@gmail.com>\n");
	printf(" -Chris Kuethe\n");
	printf(" -Eric Claeys\n");
	printf("\n");

	if (argc > 1)
	{
		// Many of the argument names changed to allow day and night values.
		// However, still check for the old names in case the user didn't update their settings.json file.
		// The old names should be removed below in a future version.
		for (i=1 ; i <= argc - 1 ; i++)
		{
			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0)
			{
				help = 1;
			}
			else if (strcmp(argv[i], "-locale") == 0)
			{
				locale = argv[++i];
			}
#ifdef CAM_RPIHQ
			else if (strcmp(argv[i], "-rotation") == 0)
			{
				asiRotation = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-background") == 0)
			{
				background = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-saturation") == 0)
			{
				saturation = atof(argv[++i]);
			}
			else if (strcmp(argv[i], "-mean-value") == 0)
			{
				myModeMeanSetting.mean_value = std::min(1.0,std::max(0.0,atof(argv[i + 1])));
				myModeMeanSetting.mode_mean = true;
				i++;
			}
			else if (strcmp(argv[i], "-mean-threshold") == 0)
			{
				myModeMeanSetting.mean_threshold = std::min(0.1,std::max(0.0001,atof(argv[i + 1])));
				myModeMeanSetting.mode_mean = true;
				i++;
			}
			else if (strcmp(argv[i], "-mean-p0") == 0)
			{
				myModeMeanSetting.mean_p0 = std::min(50.0,std::max(0.0,atof(argv[i + 1])));
				myModeMeanSetting.mode_mean = true;
				i++;
			}
			else if (strcmp(argv[i], "-mean-p1") == 0)
			{
				myModeMeanSetting.mean_p1 = std::min(50.0,std::max(0.0,atof(argv[i + 1])));
				myModeMeanSetting.mode_mean = true;
				i++;
			}
			else if (strcmp(argv[i], "-mean-p2") == 0)
			{
				myModeMeanSetting.mean_p2 = std::min(50.0,std::max(0.0,atof(argv[i + 1])));
				myModeMeanSetting.mode_mean = true;
				i++;
			}
			else if (strcmp(argv[i], "-wbr") == 0)
			{
				asiWBR = atof(argv[++i]);
			}
			else if (strcmp(argv[i], "-wbb") == 0)
			{
				asiWBB = atof(argv[++i]);
			}
#else
			else if (strcmp(argv[i], "-wbr") == 0)
			{
				asiWBR = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-wbb") == 0)
			{
				asiWBB = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-newexposure") == 0)
			{
				if (atoi(argv[++i]))
					use_new_exposure_algorithm = true;
				else
					use_new_exposure_algorithm = false;
			}
			else if (strcmp(argv[i], "-dayskipframes") == 0)
			{
				day_skip_frames = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightskipframes") == 0)
			{
				night_skip_frames = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-gaintransitiontime") == 0)
			{
				// user specifies minutes but we want seconds.
				gainTransitionTime = atoi(argv[++i]) * 60;
			}
			else if (strcmp(argv[i], "-nightmaxgain") == 0 || strcmp(argv[i], "-maxgain") == 0)
			{
				asiNightMaxGain = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightmaxexposure") == 0 || strcmp(argv[i], "-maxexposure") == 0)
			{
				asi_night_max_autoexposure_ms = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-daymaxexposure") == 0)
			{
				asi_day_max_autoexposure_ms = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightexposure") == 0 || strcmp(argv[i], "-exposure") == 0)
			{
				asi_night_exposure_us = atoi(argv[++i]) * US_IN_MS;
			}
			else if (strcmp(argv[i], "-dayexposure") == 0)
			{
				asi_day_exposure_us = atof(argv[++i]) * US_IN_MS;  // allow fractions
			}
			else if (strcmp(argv[i], "-coolerEnabled") == 0)
			{
				asiCoolerEnabled = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-targetTemp") == 0)
			{
				asiTargetTemp = atol(argv[++i]);
			}
			else if (strcmp(argv[i], "-usb") == 0)
			{
				asiBandwidth = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-autousb") == 0)
			{
				asiAutoBandwidth = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-gamma") == 0)
			{
				asiGamma = atoi(argv[++i]);
			}
#endif
			else if (strcmp(argv[i], "-tty") == 0)	// overrides what was automatically determined
			{
				Allsky::tty = atoi(argv[++i]) ? true : false;
			}
			else if (strcmp(argv[i], "-width") == 0)
			{
				width = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-height") == 0)
			{
				height = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-type") == 0)
			{
				Allsky::Image_type = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-quality") == 0)
			{
				quality = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-dayautoexposure") == 0)
			{
				asiDayAutoExposure = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightautoexposure") == 0 || strcmp(argv[i], "-autoexposure") == 0)
			{
				asiNightAutoExposure = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightgain") == 0 || strcmp(argv[i], "-gain") == 0)
			{
				asiNightGain = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightautogain") == 0 || strcmp(argv[i], "-autogain") == 0)
			{
				asiNightAutoGain = atoi(argv[++i]);
			}
			// old "-brightness" applied to day and night
			else if (strcmp(argv[i], "-brightness") == 0)
			{
				asiDayBrightness = atoi(argv[++i]);
				asiNightBrightness = asiDayBrightness;
			}
			else if (strcmp(argv[i], "-daybrightness") == 0)
			{
				asiDayBrightness = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightbrightness") == 0)
			{
				asiNightBrightness = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-daybin") == 0)
			{
				dayBin = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightbin") == 0 || strcmp(argv[i], "-bin") == 0)
			{
				nightBin = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-daydelay") == 0 || strcmp(argv[i], "-daytimeDelay") == 0)
			{
				dayDelay_ms = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-nightdelay") == 0 || strcmp(argv[i], "-delay") == 0)
			{
				nightDelay_ms = atoi(argv[++i]);
			}
			else if ((strcmp(argv[i], "-autowhitebalance") == 0) || (strcmp(argv[i], "-awb") == 0))
			{
				asiAutoAWB = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-text") == 0)
			{
				Allsky::ImgText = argv[++i];
			}
			else if (strcmp(argv[i], "-extratext") == 0)
			{
				Allsky::ImgExtraText = argv[++i];
			}
			else if (strcmp(argv[i], "-extratextage") == 0)
			{
				Allsky::extraFileAge = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-textlineheight") == 0)
			{
				Allsky::iTextLineHeight = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-textx") == 0)
			{
				Allsky::iTextX = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-texty") == 0)
			{
				Allsky::iTextY = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-fontname") == 0)
			{
				Allsky::fontnumber = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-fontcolor") == 0)
			{
				if (sscanf(argv[++i], "%d %d %d", &Allsky::fontcolor[0], &Allsky::fontcolor[1], &Allsky::fontcolor[2]) != 3)
					fprintf(stderr, "%s*** ERROR: Not enough font color parameters: '%s'%s\n", c(KRED), argv[i], c(KNRM));
			}
			else if (strcmp(argv[i], "-smallfontcolor") == 0)
			{
				if (sscanf(argv[++i], "%d %d %d", &Allsky::smallFontcolor[0], &Allsky::smallFontcolor[1], &Allsky::smallFontcolor[2]) != 3)
					fprintf(stderr, "%s*** ERROR: Not enough small font color parameters: '%s'%s\n", c(KRED), argv[i], c(KNRM));
			}
			else if (strcmp(argv[i], "-fonttype") == 0)
			{
				Allsky::linenumber = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-fontsize") == 0)
			{
				Allsky::fontsize = atof(argv[++i]);
			}
			else if (strcmp(argv[i], "-fontline") == 0)
			{
				Allsky::linewidth = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-outlinefont") == 0)
			{
				Allsky::outlinefont = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-flip") == 0)
			{
				asiFlip = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-filename") == 0)
			{
				fileName = argv[++i];
			}
			else if (strcmp(argv[i], "-latitude") == 0)
			{
				latitude = argv[++i];
			}
			else if (strcmp(argv[i], "-longitude") == 0)
			{
				longitude = argv[++i];
			}
			else if (strcmp(argv[i], "-angle") == 0)
			{
				angle = argv[++i];
			}
			else if (strcmp(argv[i], "-notificationimages") == 0)
			{
				Allsky::notificationImages = atoi(argv[++i]);
			}
#ifdef USE_HISTOGRAM
			else if (strcmp(argv[i], "-histogrambox") == 0)
			{
				if (sscanf(argv[++i], "%d %d %f %f", &histogramBoxSizeX, &histogramBoxSizeY, &histogramBoxPercentFromLeft, &histogramBoxPercentFromTop) != 4)
					fprintf(stderr, "%s*** ERROR: Not enough histogram box parameters: '%s'%s\n", c(KRED), argv[i], c(KNRM));

				// scale user-input 0-100 to 0.0-1.0
				histogramBoxPercentFromLeft /= 100;
				histogramBoxPercentFromTop /= 100;
			}
			else if (strcmp(argv[i], "-showhistogrambox") == 0)
			{
				showHistogramBox = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-aggression") == 0)
			{
				aggression = atoi(argv[++i]);
				if (aggression < 1)
				{
					fprintf(stderr, "Aggression must be between 1 and 100; setting to 1.\n");
				}
				else if (aggression > 100)
				{
					fprintf(stderr, "Aggression must be between 1 and 100; setting to 100.\n");
				}
			}
#endif
			else if (strcmp(argv[i], "-preview") == 0)
			{
				preview = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-debuglevel") == 0)
			{
				Allsky::debugLevel = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-showTime") == 0 || strcmp(argv[i], "-time") == 0)
			{
				showTime = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-timeformat") == 0)
			{
				timeFormat = argv[++i];
			}
			else if (strcmp(argv[i], "-darkframe") == 0)
			{
				taking_dark_frames = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-showDetails") == 0)
			{
				showDetails = atoi(argv[++i]);
				// showDetails is an obsolete variable that shows ALL details except time.
				// It's been replaced by separate variables for various lines.
				Allsky::showTemp = showDetails;
				Allsky::showExposure = showDetails;
				Allsky::showGain = showDetails;
			}
			else if (strcmp(argv[i], "-showTemp") == 0)
			{
				Allsky::showTemp = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-temptype") == 0)
			{
				Allsky::tempType = argv[++i];
			}
			else if (strcmp(argv[i], "-showExposure") == 0)
			{
				Allsky::showExposure = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-showGain") == 0)
			{
				Allsky::showGain = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-showBrightness") == 0)
			{
				Allsky::showBrightness = atoi(argv[++i]);
			}
#ifdef USE_HISTOGRAM
			else if (strcmp(argv[i], "-showHistogram") == 0)
			{
				showHistogram = atoi(argv[++i]);
			}
#endif
			else if (strcmp(argv[i], "-daytime") == 0)
			{
				daytimeCapture = atoi(argv[++i]);
			}
		}
	}

	if (help == 1)
	{
		printf("%sUsage:\n", c(KRED));
		printf(" ./capture -width 640 -height 480 -nightexposure 5000000 -gamma 50 -type 1 -nightbin 1 -filename Lake-Laberge.PNG\n\n");
		printf("%s", c(KNRM));

		printf("%sAvailable Arguments:\n", c(KYEL));
		printf(" -width                 - Default = %d = Camera Max Width\n", DEFAULT_WIDTH);
		printf(" -height                - Default = %d = Camera Max Height\n", DEFAULT_HEIGHT);
		printf(" -daytime               - Default = %d: 1 enables capture daytime images\n", DEFAULT_DAYTIMECAPTURE);
		printf(" -type = Image Type     - Default = %d: 99 = auto,  0 = RAW8,  1 = RGB24,  2 = RAW16,  3 = Y8\n", DEFAULT_IMAGE_TYPE);
		printf(" -quality               - Default PNG=3, JPG=95, Values: PNG=0-9, JPG=0-100\n");
#ifdef CAM_RPIHQ
		printf(" -background= Font Color            - Default = 0  - Backgroud gray scale color (0 - 255)\n");
		printf(" -mean-value                        - Default = 0.3 Set mean-value and activates exposure control\n");
		printf("                                      NOTE: Auto-Gain should be On in the WebUI\n");
		printf("                                            -autoexposure should be set in config.sh:\n");
		printf("                                            CAPTURE_EXTRA_PARAMETERS='-mean-value 0.3 -autoexposure 1'\n"); 
		printf(" -mean-threshold                    - Default = 0.01 Set mean-value and activates exposure control\n");
		printf(" -mean-p0                           - Default = 5.0, be careful changing these values, ExposureChange (Steps) = p0 + p1 * diff + (p2*diff)^2\n");
		printf(" -mean-p1                           - Default = 20.0\n");
		printf(" -mean-p2                           - Default = 45.0\n");
#else
		printf(" -dayexposure           - Default = %'d: Daytime exposure in us (equals to %.4f sec)\n", DEFAULT_ASIDAYEXPOSURE, (float)DEFAULT_ASIDAYEXPOSURE/US_IN_SEC);
		printf(" -nightexposure         - Default = %'d: Nighttime exposure in us (equals to %.4f sec)\n", DEFAULT_ASINIGHTEXPOSURE, (float)DEFAULT_ASINIGHTEXPOSURE/US_IN_SEC);
		printf(" -dayautoexposure       - Default = %d: 1 enables daytime auto-exposure\n", DEFAULT_DAYAUTOEXPOSURE);
		printf(" -nightautoexposure     - Default = %d: 1 enables nighttime auto-exposure\n", DEFAULT_NIGHTAUTOEXPOSURE);
		printf(" -daymaxexposure        - Default = %'d: Maximum daytime auto-exposure in ms (equals to %.1f sec)\n", DEFAULT_ASIDAYMAXAUTOEXPOSURE_MS, (float)DEFAULT_ASIDAYMAXAUTOEXPOSURE_MS/US_IN_MS);
		printf(" -nightmaxexposure      - Default = %'d: Maximum nighttime auto-exposure in ms (equals to %.1f sec)\n", DEFAULT_ASINIGHTMAXAUTOEXPOSURE_MS, (float)DEFAULT_ASINIGHTMAXAUTOEXPOSURE_MS/US_IN_MS);
		printf(" -daybrightness         - Default = %d: Daytime brightness level\n", DEFAULT_BRIGHTNESS);
		printf(" -nightbrightness       - Default = %d: Nighttime brightness level\n", DEFAULT_BRIGHTNESS);
		printf(" -nightgain             - Default = %d: Nighttime gain\n", DEFAULT_ASINIGHTGAIN);
		printf(" -dayDelay              - Default = %'d: Delay between daytime images in milliseconds - 5000 = 5 sec.\n", DEFAULT_DAYDELAY);
		printf(" -nightDelay            - Default = %'d: Delay between nighttime images in milliseconds - %d = 1 sec.\n", DEFAULT_NIGHTDELAY, MS_IN_SEC);
		printf(" -nightmaxgain          - Default = %d: Nighttime maximum auto gain\n", DEFAULT_ASINIGHTMAXGAIN);
		printf(" -nightautogain         - Default = %d: 1 enables nighttime auto gain\n", DEFAULT_NIGHTAUTOGAIN);
		printf(" -gaintransitiontime    - Default = %'d: Seconds to transition gain from day-to-night or night-to-day.  0 disable it.\n", DEFAULT_GAIN_TRANSITION_TIME);
		printf(" -dayskipframes         - Default = %d: Number of auto-exposure daytime frames to skip when starting software.\n", DEFAULT_DAYSKIPFRAMES);
		printf(" -nightskipframes       - Default = %d: Number of auto-exposure nighttime frames to skip when starting software.\n", DEFAULT_NIGHTSKIPFRAMES);
		printf(" -coolerEnabled         - 1 enables cooler (cooled cameras only)\n");
		printf(" -targetTemp            - Target temperature in degrees C (cooled cameras only)\n");
		printf(" -gamma                 - Default = %d: Gamma level\n", DEFAULT_ASIGAMMA);
		printf(" -wbr                   - Default = %d: Manual White Balance Red\n", DEFAULT_ASIWBR);
		printf(" -wbb                   - Default = %d: Manual White Balance Blue\n", DEFAULT_ASIWBB);
		printf(" -autowhitebalance      - Default = %d: 1 enables auto White Balance\n", DEFAULT_AUTOWHITEBALANCE);
		printf(" -daybin                - Default = %d: 1 = binning OFF (1x1), 2 = 2x2 binning, 4 = 4x4 binning\n", DEFAULT_DAYBIN);
		printf(" -nightbin              - Default = %d: same as daybin but for night\n", DEFAULT_NIGHTBIN);
		printf(" -usb = USB Speed       - Default = %d: Values between 40-100, This is BandwidthOverload\n", DEFAULT_ASIBANDWIDTH);
		printf(" -autousb               - Default = 0: 1 enables auto USB Speed\n");
#endif
		printf(" -filename              - Default = %s\n", DEFAULT_FILENAME);
		printf(" -flip                  - Default = 0: 0 = No flip, 1 = Horizontal, 2 = Vertical, 3 = Both\n");
		printf("\n");
		printf(" -text                  - Default = \"\": Text Overlay\n");
		printf(" -extratext             - Default = \"\": Full Path to extra text to display\n");
		printf(" -extratextage          - Default = 0: If the extra file is not updated after this many seconds its contents will not be displayed. 0 disables it.\n");
		printf(" -textlineheight        - Default = %d: Text Line Height in pixels\n", DEFAULT_ITEXTLINEHEIGHT);
		printf(" -textx                 - Default = %d: Text Placement Horizontal from LEFT in pixels\n", DEFAULT_ITEXTX);
		printf(" -texty                 - Default = %d: Text Placement Vertical from TOP in pixels\n", DEFAULT_ITEXTY);
		printf(" -fontname              - Default = %d: Font Types (0-7), Ex. 0 = simplex, 4 = triplex, 7 = script\n", DEFAULT_FONTNUMBER);
		printf(" -fontcolor             - Default = 255 0 0: Text font color (BGR)\n");
		printf(" -smallfontcolor        - Default = 0 0 255: Small text font color (BGR)\n");
		printf(" -fonttype              - Default = %d: Font Line Type: 0=AA, 1=8, 2=4\n", DEFAULT_LINENUMBER);
		printf(" -fontsize              - Default = %d: Text Font Size\n", DEFAULT_FONTSIZE);
		printf(" -fontline              - Default = %d: Text Font Line Thickness\n", DEFAULT_LINEWIDTH);
		printf(" -outlinefont           - Default = %d: 1 enables outline font\n", DEFAULT_OUTLINEFONT);
		printf("\n");
		printf("\n");
		printf(" -latitude              - Default = %7s: Latitude of the camera.\n", DEFAULT_LATITUDE);
		printf(" -longitude             - Default = %7s: Longitude of the camera\n", DEFAULT_LONGITUDE);
		printf(" -angle                 - Default = %s: Angle of the sun below the horizon.\n", DEFAULT_ANGLE);
		printf("        -6=civil twilight   -12=nautical twilight   -18=astronomical twilight\n");
		printf("\n");
		printf(" -locale                - Default = %s: Your locale - to determine thousands separator and decimal point.\n", DEFAULT_LOCALE);
		printf("                          Type 'locale' at a command prompt to determine yours.\n");
		printf(" -notificationimages    - 1 enables notification images, for example, 'Camera is off during day'.\n");
#ifdef USE_HISTOGRAM
		printf(" -histogrambox          - Default = %d %d %0.2f %0.2f (box width X, box width y, X offset percent (0-100), Y offset (0-100))\n", DEFAULT_BOX_SIZEX, DEFAULT_BOX_SIZEY, DEFAULT_BOX_FROM_LEFT * 100, DEFAULT_BOX_FROM_TOP * 100);
		printf(" -showhistogrambox      - 1 displays an outline of the histogram box on the image overlay.\n");
		printf("                          Useful to determine what parameters to use with -histogrambox.\n");
		printf(" -aggression            - Default = %d%%: Percent of exposure change to make, similar to PHD2.\n", DEFAULT_AGGRESSION);
#endif
		printf(" -darkframe             - 1 disables the overlay and takes dark frames instead\n");
		printf(" -preview               - 1 previews the captured images. Only works with a Desktop Environment\n");
		printf(" -time                  - 1 displayes the time. Combine with Text X and Text Y for placement\n");
		printf(" -timeformat            - Format the optional time is displayed in; default is '%s'\n", DEFAULT_TIMEFORMAT);
		printf(" -showTemp              - 1 displays the camera sensor temperature\n");
		printf(" -temptype              - Units to display temperature in: 'C'elsius, 'F'ahrenheit, or 'B'oth.\n");
		printf(" -showExposure          - 1 displays the exposure length\n");
		printf(" -showGain              - 1 display the gain\n");
		printf(" -showBrightness        - 1 displays the brightness\n");
#ifdef USE_HISTOGRAM
		printf(" -showHistogram         - 1 displays the histogram mean\n");
#endif
		printf(" -debuglevel            - Default = 0. Set to 1,2, 3, or 4 for more debugging information.\n");

		printf("%s", c(KNRM));
		exit(0);
	}
}


void Allsky::info(void)
{
	printf("%s", c(KGRN));
#ifdef CAM_RPIHQ
	printf("\nCapture Settings for RPiHQ:\n");
	if (myModeMeanSetting.mode_mean) {
		printf("    Mean Value: %1.3f\n", myModeMeanSetting.mean_value);
		printf("    Threshold: %1.3f\n", myModeMeanSetting.mean_threshold);
		printf("    p0: %1.3f\n", myModeMeanSetting.mean_p0);
		printf("    p1: %1.3f\n", myModeMeanSetting.mean_p1);
		printf("    p2: %1.3f\n", myModeMeanSetting.mean_p2);
	}
#else
	printf("\nCapture Settings:\n");
	printf(" Skip Frames (day): %d\n", day_skip_frames);
	printf(" Skip Frames (night): %d\n", night_skip_frames);
	printf(" Gain Transition Time: %.1f minutes\n", (float) gainTransitionTime/60);
	printf(" Gain (night only): %d, Auto: %s, max: %d\n", asiNightGain, yesNo(asiNightAutoGain), asiNightMaxGain);
	printf(" Max Auto-Exposure (night): %'dms (%'.1fs)\n", asi_night_max_autoexposure_ms, (float)asi_night_max_autoexposure_ms / MS_IN_SEC);
	printf(" Max Auto-Exposure (day): %'dms (%'.1fs)\n", asi_day_max_autoexposure_ms, (float)asi_day_max_autoexposure_ms / MS_IN_SEC);
	printf(" Exposure (night): %'1.0fms, Auto: %s\n", round(asi_night_exposure_us / US_IN_MS), yesNo(asiNightAutoExposure));
	printf(" Exposure (day): %'1.3fms, Auto: %s\n", (float)asi_day_exposure_us / US_IN_MS, yesNo(asiDayAutoExposure));
	printf(" Gamma: %d\n", asiGamma);
#endif
	printf(" Image Type: %s\n", sType);
	printf(" Resolution (before any binning): %dx%d\n", width, height);
	printf(" Quality: %d\n", quality);
	printf(" Daytime capture: %s\n", yesNo(daytimeCapture));
	printf(" Delay (day): %'dms\n", dayDelay_ms);
	printf(" Delay (night): %'dms\n", nightDelay_ms);
	printf(" Brightness (day): %d\n", asiDayBrightness);
	printf(" Brightness (night): %d\n", asiNightBrightness);
	printf(" Binning (day): %d\n", dayBin);
	printf(" Binning (night): %d\n", nightBin);
	printf(" Text Overlay: %s\n", Allsky::ImgText[0] == '\0' ? "[none]" : Allsky::ImgText);
	printf(" Text Extra File: %s, Age: %d seconds\n", Allsky::ImgExtraText[0] == '\0' ? "[none]" : Allsky::ImgExtraText, Allsky::extraFileAge);
	printf(" Text Line Height %dpx\n", Allsky::iTextLineHeight);
	printf(" Text Position: %dpx from left, %dpx from top\n", Allsky::iTextX, Allsky::iTextY);
	printf(" Font Name:  %s (%d)\n", Allsky::fontnames[Allsky::fontnumber], Allsky::fontname[Allsky::fontnumber]);
	printf(" Font Color: %d, %d, %d\n", Allsky::fontcolor[0], Allsky::fontcolor[1], Allsky::fontcolor[2]);
	printf(" Small Font Color: %d, %d, %d\n", Allsky::smallFontcolor[0], Allsky::smallFontcolor[1], Allsky::smallFontcolor[2]);
	printf(" Font Line Type: %d\n", Allsky::linetype[Allsky::linenumber]);
	printf(" Font Size: %1.1f\n", Allsky::fontsize);
	printf(" Font Line Width: %d\n", Allsky::linewidth);
	printf(" Outline Font : %s\n", yesNo(Allsky::outlinefont));
	printf(" Flip Image: %d\n", asiFlip);
	printf(" Filename: %s\n", fileName);
	printf(" Latitude: %s, Longitude: %s\n", latitude, longitude);
	printf(" Sun Elevation: %s\n", angle);
	printf(" Locale: %s\n", locale);
	printf(" Notification Images: %s\n", yesNo(Allsky::notificationImages));
	printf(" Show Time: %s (format: %s)\n", yesNo(showTime), timeFormat);
	printf(" Show Details: %s\n", yesNo(showDetails));
	printf(" Show Temperature: %s, type: %s\n", yesNo(Allsky::showTemp), Allsky::tempType);
	printf(" Show Exposure: %s\n", yesNo(Allsky::showExposure));
	printf(" Show Gain: %s\n", yesNo(Allsky::showGain));
	printf(" Show Brightness: %s\n", yesNo(Allsky::showBrightness));
	printf(" Preview: %s\n", yesNo(preview));
	printf(" Taking Dark Frames: %s\n", yesNo(taking_dark_frames));
	printf(" Debug Level: %d\n", Allsky::debugLevel);
	printf(" On TTY: %s\n", Allsky::tty ? "Yes" : "No");
#ifdef CAM_RPIHQ
#else
	printf(" Aggression: %d%%\n", aggression);

	if (ASICameraInfo.IsCoolerCam)
	{
		printf(" Cooler Enabled: %s", yesNo(asiCoolerEnabled));
		if (asiCoolerEnabled) printf(", Target Temperature: %ld C\n", asiTargetTemp);
		printf("\n");
	}
	if (ASICameraInfo.IsColorCam)
	{
		printf(" WB Red: %d, Blue: %d, Auto: %s\n", asiWBR, asiWBB, yesNo(asiAutoAWB));
	}
	printf(" USB Speed: %d, auto: %s\n", asiBandwidth, yesNo(asiAutoBandwidth));
	#ifdef USE_HISTOGRAM
		printf(" Histogram Box: %d %d %0.0f %0.0f, center: %dx%d, upper left: %dx%d, lower right: %dx%d\n",
			histogramBoxSizeX, histogramBoxSizeY,
			histogramBoxPercentFromLeft * 100, histogramBoxPercentFromTop * 100,
			centerX, centerY, left_of_box, top_of_box, right_of_box, bottom_of_box);
		printf(" Show Histogram Box: %s\n", yesNo(showHistogramBox));
		printf(" Show Histogram Mean: %s\n", yesNo(showHistogram));
	#endif
	printf(" Video OFF Between Images: %s\n", yesNo(use_new_exposure_algorithm));
	printf(" ZWO SDK version %s\n", ASIGetSDKVersion());
#endif

	printf("%s\n", c(KNRM));
}

// Simple function to make flags easier to read for humans.
char const *Allsky::yesNo(int flag)
{
	if (flag)
		return("Yes");
	else
		return("No");
}

// Return the string for the specified color, or "" if we're not on a tty.
char const *Allsky::c(char const *color)
{
	if (Allsky::tty)
	{
		return(color);
	}
	else
	{
		return("");
	}
}

// Return the numeric time.
timeval Allsky::getTimeval()
{
	timeval curTime;
	gettimeofday(&curTime, NULL);
	return(curTime);
}

// Format a numeric time as a string.
char *Allsky::formatTime(timeval t, char const *tf)
{
		static char TimeString[128];
		strftime(TimeString, 80, tf, localtime(&t.tv_sec));
		return(TimeString);
}

// Return the current time as a string.  Uses both functions above.
char *Allsky::getTime(char const *tf)
{
		return(formatTime(Allsky::getTimeval(), tf));
}

std::string Allsky::exec(const char *cmd)
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

void Allsky::IntHandle(int i)
{
	gotSignal = 1;
	closeUp(0);
}

// Exit the program gracefully.
void Allsky::closeUp(int e)
{
	static int closingUp = 0;		// indicates if we're in the process of exiting.
	// For whatever reason, we're sometimes called twice, but we should only execute once.
	if (closingUp) return;

	closingUp = 1;

#ifdef CAM_RPIHQ
#else
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
#endif

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

// Calculate if it is day or night
void Allsky::calculateDayOrNight(const char *latitude, const char *longitude, const char *angle)
{
	char sunwaitCommand[128];
	sprintf(sunwaitCommand, "sunwait poll angle %s %s %s", angle, latitude, longitude);
	dayOrNight = Allsky::exec(sunwaitCommand);
	dayOrNight.erase(std::remove(dayOrNight.begin(), dayOrNight.end(), '\n'), dayOrNight.end());

	if (dayOrNight != "DAY" && dayOrNight != "NIGHT")
	{
		sprintf(Allsky::debugText, "*** ERROR: dayOrNight isn't DAY or NIGHT, it's '%s'\n", dayOrNight == "" ? "[empty]" : dayOrNight.c_str());
		Allsky::waitToFix(Allsky::debugText);
		Allsky::closeUp(2);
	}
}

// Calculate how long until nighttime.
int Allsky::calculateTimeToNightTime(const char *latitude, const char *longitude, const char *angle)
{
	std::string t;
	char sunwaitCommand[128];	// returns "hh:mm, hh:mm" (sunrise, sunset)
	sprintf(sunwaitCommand, "sunwait list angle %s %s %s | awk '{print $2}'", angle, latitude, longitude);
	t = Allsky::exec(sunwaitCommand);
	t.erase(std::remove(t.begin(), t.end(), '\n'), t.end());

	int h=0, m=0, secs;
// xxxx TODO: Check - this might be getting time to DAY, not NIGHT
	sscanf(t.c_str(), "%d:%d", &h, &m);
	secs = (h*60*60) + (m*60);

	char *now = Allsky::getTime("%H:%M");
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