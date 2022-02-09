#pragma once

#include <sys/time.h>

class AllskyHelperInterface {
	public:
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
};

