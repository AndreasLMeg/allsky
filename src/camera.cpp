#include "camera.h"
Camera::Camera (int argc, char *argv[]) 
{
	printf("Camera (int argc, char *argv[])\n");
	Allsky::Debug("Camera::Camera (int argc, char *argv[])\n");
	//init(argc, argv);
};

void Camera::init(int argc, char *argv[])
{
	printf("Camera::init(int argc, char *argv[])\n");

	Allsky::init(argc, argv);
	ModeMean::initModeMean(60 * US_IN_SEC, 16, 60 * US_IN_SEC);
};
