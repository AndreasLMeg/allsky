#include <stdio.h>
#include <sys/time.h>
#include <tr1/memory>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

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
