#include "allsky.h"
#include "camera_rpihq.h"
#include "allskyStatic.h"

Camera* myCam;
//-------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
#ifdef CAM_RPIHQ
	myCam = new CameraRPi(argc, argv);
#endif
	// TODO: other cameras....

	myCam->info();
	myCam->run();
}
