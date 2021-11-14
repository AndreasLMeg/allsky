#include <stdio.h>
#include <sys/time.h>
#include <tr1/memory>
#include <signal.h>

#include "allsky.h"


/**
 * Helper function to display debug info
**/
void Allsky::displayDebugText(const char * text, int requiredLevel) {
    if (debugLevel >= requiredLevel) {
        printf("%s", text);
    }
}


// ASI
// Make sure we don't try to update a non-updateable control, and check for errors.
ASI_ERROR_CODE Allsky::setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto)
{
    ASI_ERROR_CODE ret = ASI_SUCCESS;
    int i;
    for (i = 0; i < iNumOfCtrl && i <= control; i++)    // controls are sorted 1 to n
    {
        ret = ASIGetControlCaps(CamNum, i, &ControlCaps);

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
            } else {
                printf("ERROR: ControlCap: '%s' (#%d) not writable; not setting to %ld.\n", ControlCaps.Name, ControlCaps.ControlType, value);
                ret = ASI_ERROR_INVALID_MODE;	// this seemed like the closest error
            }
            return ret;
        }
    }
    sprintf(debug_text, "NOTICE: Camera does not support ControlCap # %d; not setting to %ld.\n", control, value);
    Allsky::displayDebugText(debug_text, 3);
    return ASI_ERROR_INVALID_CONTROL_TYPE;
}

// Create Hex value from RGB
unsigned long Allsky::createRGB(int r, int g, int b)
{
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

void Allsky::cvText(cv::Mat &img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname,
            int fontcolor[], int imgtype, int outlinefont)
{
    if (imgtype == ASI_IMG_RAW16)
    {
        unsigned long fontcolor16 = Allsky::createRGB(fontcolor[2], fontcolor[1], fontcolor[0]);
        if (outlinefont)
            cv::putText(img, text, cv::Point(x, y), fontname, fontsize, cv::Scalar(0,0,0), linewidth+4, linetype);
        cv::putText(img, text, cv::Point(x, y), fontname, fontsize, fontcolor16, linewidth, linetype);
    }
    else
    {
        if (outlinefont)
            cv::putText(img, text, cv::Point(x, y), fontname, fontsize, cv::Scalar(0,0,0, 255), linewidth+4, linetype);
        cv::putText(img, text, cv::Point(x, y), fontname, fontsize,
                    cv::Scalar(fontcolor[0], fontcolor[1], fontcolor[2], 255), linewidth, linetype);
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
    return(formatTime(getTimeval(), tf));
}

double Allsky::time_diff_us(int64 start, int64 end)
{
	double frequency = cv::getTickFrequency();
	return (double)(end - start) / frequency;	// in Microseconds
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

void *Allsky::Display(void *params)
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

void *Allsky::SaveImgThd(void *para)
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

	char dT[500];	// Since we're in a thread, use our own copy of debugText
        sprintf(dT, "  > Saving %s image '%s'\n", taking_dark_frames ? "dark" : dayOrNight == "DAY" ? "DAY" : "NIGHT", fileName);
        Allsky::displayDebugText(dT, 1);
        int64 st, et;

        bool result = false;
        if (pRgb.data)
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
            sprintf(cmd, "%s %s '%s' '%2.0f' %ld &", s, dayOrNight.c_str(), fileName, (float) actualTemp/10, current_exposure_us);
            st = cv::getTickCount();
            try
            {
                result = imwrite(fileName, pRgb, compression_parameters);
            }
            catch (const cv::Exception& ex)
            {
                printf("*** ERROR: Exception saving image: %s\n", ex.what());
            }
            et = cv::getTickCount();

            if (result)
                system(cmd);
	    else
                printf("*** ERROR: Unable to save image '%s'.\n", fileName);

        } else {
            // This can happen if the program is closed before the first picture.
            Allsky::displayDebugText("----- SaveImgThd(): pRgb.data is null\n", 2);
        }
        bSavingImg = false;

        if (result)
	{
            static int total_saves = 0;
            static double total_time_ms = 0;
            total_saves++;
            double diff = Allsky::time_diff_us(st, et) * US_IN_MS;	// we want ms
            total_time_ms += diff;
            char const *x;
            if (diff > 1 * MS_IN_SEC)
               x = "  > *****\n";	// indicate when it takes a REALLY long time to save
            else
               x = "";
            sprintf(dT, "%s  > Image took %'.1f ms to save (average %'.1f ms).\n%s", x, diff, total_time_ms / total_saves, x);
            Allsky::displayDebugText(dT, 4);
	}

        pthread_mutex_unlock(&mtx_SaveImg);
    }

    return (void *)0;
}
