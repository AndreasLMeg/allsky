#pragma once

#include <sys/time.h>

class AllskyHelperInterface {

	public:
		//virtual char *getTime(char const *tf) = 0;
};

class AllskyHelper : public AllskyHelperInterface
{
	public:
	private:
	public:
		static int gain2zwoGain(float);
		static int zwoGain2gain(int);
		static char *length_in_units(long us, bool multi);
		static char const *yesNo(int flag);
		static unsigned long createRGB(int r, int g, int b);
		//static timeval getTimeval();
		//static char *formatTime(timeval t, char const *tf);
		//char *_getTime(char const *tf);

		// interface
 		//virtual char *getTime(char const *tf) override {
    //	return _getTime(tf);
  	//}

};

