#include <stdio.h>

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
