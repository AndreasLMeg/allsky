#pragma once

#include "allsky_common.h"

class ModeMean {
	public:
		ModeMean () {}; 
		ModeMean (int argc, char *argv[]) {}; 

  public:
		bool mode_mean = false;	// Activate mode mean.  User can change this.
		bool m_init	= true;		// Set some settings before first calculation. This is set to "false" after calculation.
		bool fastforward = false;

		double mean_history [5] = {0.0,0.0,0.0,0.0,0.0};
		int exp_history [5] = {0,0,0,0,0};

		double ExposureLevelMin	= - 1;		// Set during first calculation.
		double ExposureLevelMax	= 1; 		// Set during first calculation.
		double shuttersteps	= 6.0;		// shuttersteps
		int quickstart 	= 10;		// Shorten delay between captures for this many images, to help get us to a good exposure quicker.
		//int ExposureLevel	= 1;		// current ExposureLevel 
		int maxExposure_us = 60 * US_IN_SEC;
		double maxGain = 15;
		int minExposure_us = 1;
		double minGain = 1;

		struct setting_ModeMean {
			double mean_value	= 0.3;		// mean value for well exposed images
			double mean_threshold	= 0.01;		// threshold value
			int historySize	= 3;		// Use this many last images for mean target calculation
			double mean_p0 = 5.0;		// ExposureChange (Steps) = p0 + p1 * diff + (p2*diff)^2
			double mean_p1 = 20.0;
			double mean_p2 = 45.0;
		} setting_ModeMean;
		struct values {
			int current_ExposureLevel;
			int last_ExposureLevel;
			float current_mean;
			float last_mean;
			int cnt = 0;
			int idx = 0;
			int idxN1 = 0;

			int lastExposurelevelChange = 0;
			int dExposurelevelChange = 0;
		} values_ModeMean;

	public:
		void initModeMean(int exposure_us, double gain, int Exposure_us);

	  void calculateNextExposureSettings(void);
		float calculateMean(void);
		float calculateMeanAVG(void);
		int calculateExposurelevelChange(float mean_diff);
		int calculateExposureLevel(int ExposurelevelChange, float meanForcast);
		void calculateCaptureValues(int Exposurelevel);
};
