#include "allsky.h"
#include "camera_rpihq.h"

#include <string.h>
#include <string>
#include <cstring>


//#include <sys/time.h>
//#include <unistd.h>

//#include <string.h>
//#include <errno.h>
//#include <string>

//#include <iomanip>
//#include <cstring>
//#include <sstream>
//#include <tr1/memory>

//#include <stdlib.h>
//#include <signal.h>
//#include <fstream>
//#include <stdarg.h>


using namespace std;

void CameraRPi::kill() 
{
	// Ensure no process is still running.
	// Define command line.
	string command;
	if (is_libcamera) command = "libcamera-still";
	else command = "raspistill";

	// Include "--" so we only find the command, not a different program with the command
	// on its command line.
	string kill = "ps -ef | grep '" + command + " --' | grep -v color | awk '{print $2}' | xargs kill -9 1> /dev/null 2>&1";
	char kcmd[kill.length() + 1];		// Define char variable
	strcpy(kcmd, kill.c_str());			// Convert command to character variable

	Allsky::Trace(" > Kill command: %s\n", kcmd);
	system(kcmd);						// Stop any currently running process
}


void CameraRPi::setup() 
{
	// Define command line.
	string command;
	if (is_libcamera) command = "libcamera-still";
	else command = "raspistill";

	stringstream ss;
	ss << Allsky::fileName;
	command += " --output '" + ss.str() + "'";
	if (Allsky::is_libcamera)
		// xxx TODO: does this do anything?
		command += " --tuning-file /usr/share/libcamera/ipa/raspberrypi/imx477.json";
	else
		command += " --thumb none --burst -st";
}