#if defined CAM_RPIHQ
#include "camera_rpihq.h"
#elif defined CAM_ZWO
#define USE_HISTOGRAM                     // use the histogram code as a workaround to ZWO's bug
#include "camera_zwo.h"
#endif

#include "allsky.h"
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
