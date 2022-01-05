#if defined CAM_ZWO
#define USE_HISTOGRAM                     // use the histogram code as a workaround to ZWO's bug
#endif

#include "allsky.h"
#include "camera_rpihq.h"
#include "camera_zwo.h"
#include "allskyStatic.h"

Camera* myCam;
//-------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
#if defined CAM_RPIHQ
	myCam = new CameraRPi(argc, argv);
#elif defined CAM_ZWO
	myCam = new CameraZWO(argc, argv);
#endif

	myCam->info();
	myCam->run();
}
