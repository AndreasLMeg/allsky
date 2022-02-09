#include "allsky_helper.h"
#include "allsky_common.h"
#include <math.h> 
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

int AllskyHelper::gain2zwoGain(float gain)
{
	return (10.0 * 20.0 * log10(gain));
};

int AllskyHelper::zwoGain2gain(int zwoGain) 
{
	return (pow(10,(float)zwoGain / 10.0 / 20.0));
};

// Display a length of time in different units, depending on the length's value.
// If the "multi" flag is set, display in multiple units if appropriate.
char *AllskyHelper::length_in_units(long us, bool multi)	// microseconds
{
	const int l = 50;
	static char length[l];
	if (us == 0)
	{
		snprintf(length, l, "0 us");
	}
	else
	{
		double us_in_ms = (double)us / US_IN_MS;
		// The boundaries on when to display one or two units are really a matter of taste.
		if (us_in_ms < 0.5)						// less than 0.5 ms
		{
			snprintf(length, l, "%'ld us", us);
		}
		else if (us_in_ms < 1.5)				// between 0.5 and 1.5 ms
		{
			if (multi)
				snprintf(length, l, "%'ld us (%.3f ms)", us, us_in_ms);
			else
				snprintf(length, l, "%'ld us", us);
		}
		else if (us_in_ms < (0.5 * MS_IN_SEC))	// 1.5 ms to 0.5 sec
		{
			if (multi)
				snprintf(length, l, "%.2f ms (%.2lf sec)", us_in_ms, (double)us / US_IN_SEC);
			else
				snprintf(length, l, "%.2f ms", us_in_ms);
		}
		else if (us_in_ms < (1.0 * MS_IN_SEC))	// between 0.5 sec and 1 sec
		{
			if (multi)
				snprintf(length, l, "%.2f ms (%.2lf sec)", us_in_ms, (double)us / US_IN_SEC);
			else
				snprintf(length, l, "%.1f ms", us_in_ms);
		}
		else									// over 1 sec
		{
			snprintf(length, l, "%.1lf sec", (double)us / US_IN_SEC);
		}

	}
	return(length);
}

// Simple function to make flags easier to read for humans.
char const *AllskyHelper::yesNo(int flag)
{
	if (flag)
		return("Yes");
	else
		return("No");
}

