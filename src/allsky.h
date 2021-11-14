#ifndef ALLSKY_H
#define ALLSKY_H

#include "include/ASICamera2.h"

// Base class
class Allsky {
  public:
    static int debugLevel;
    static int iNumOfCtrl;
    static ASI_CONTROL_CAPS ControlCaps;
    static char debug_text[500];		// buffer to hold debug messages displayed by displayDebugText()
    static char debug_text2[100];		// buffer to hold additional message


    static void displayDebugText(const char * text, int requiredLevel);

    static ASI_ERROR_CODE setControl(int CamNum, ASI_CONTROL_TYPE control, long value, ASI_BOOL makeAuto);
};

#endif