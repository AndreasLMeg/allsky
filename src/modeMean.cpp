#include "modeMean.h"

#include "allsky.h"
#include "log.h"
#include <opencv2/core/core.hpp>
#include <algorithm> 


void ModeMean::initModeMean(int maxExposure_us, double maxGain, int Exposure_us) 
{
	Allsky::Debug("initModeMean\n");

	if (m_init) {
		ExposureLevelMax = log(maxGain * maxExposure_us/(double) US_IN_SEC) / log (2.0) * pow(shuttersteps,2.0) + 1; 
		ExposureLevelMin = log(1.0  * 1.0              /(double) US_IN_SEC) / log (2.0) * pow(shuttersteps,2.0) - 1;
		// only for the output
		for (int i=0; i < setting_ModeMean.historySize; i++) {
			mean_history[i] = setting_ModeMean.mean_value;
			exp_history[i] = log(1.0  * Exposure_us/(double) US_IN_SEC) / log (2.0) * pow(shuttersteps,2.0) - 1;
		}
		// first exposure with currentRaspistillSetting.shutter_us, so we have to calculate the startpoint for ExposureLevel
		values_ModeMean.current_ExposureLevel = log(1.0  * Exposure_us/(double) US_IN_SEC) / log (2.0) * pow(shuttersteps,2.0) - 1;
		values_ModeMean.last_ExposureLevel = values_ModeMean.current_ExposureLevel;
		Allsky::Debug("  > Valid ExposureLevels: %1.8f to %1.8f\n", ExposureLevelMin, ExposureLevelMax);

		values_ModeMean.last_mean = setting_ModeMean.mean_value;
		values_ModeMean.current_mean = setting_ModeMean.mean_value;
		
		Allsky::valuesCapture.exposure_us = Exposure_us;
		Allsky::valuesCapture.gain = 1.0;
		Allsky::valuesCapture.gain_dB = AllskyHelper::gain2zwoGain(Allsky::valuesCapture.gain); 
		m_init = false;
	}
}

float ModeMean::calculateMean(void)
{
	double mean;

	//Then define your mask image
	cv::Mat mask = cv::Mat::zeros(Allsky::settings.image.pRgb.size(), CV_8U);

	//Define your destination image
	cv::Mat dstImage = cv::Mat::zeros(Allsky::settings.image.pRgb.size(), CV_8U);

	//I assume you want to draw the circle at the center of your image, with a radius of mask.rows/3
	cv::circle(mask, cv::Point(mask.cols/2, mask.rows/2), mask.rows/3, cv::Scalar(255, 255, 255), -1, 8, 0);

	//Now you can copy your source image to destination image with masking
	Allsky::settings.image.pRgb.copyTo(dstImage, mask);

	cv::Scalar mean_scalar = cv::mean(Allsky::settings.image.pRgb, mask);
	switch (Allsky::settings.image.pRgb.channels())
	{
		default: // mono case
			Allsky::Debug("  > mean_scalar.val[0] %d\n", mean_scalar.val[0]);
			mean = mean_scalar.val[0];
			break;
		case 3: // for color use average of the channels
		case 4:
			mean = (mean_scalar[0] + mean_scalar[1] + mean_scalar[2]) / 3.0;
			break;
	}
	// Scale to 0-1 range
	switch (Allsky::settings.image.pRgb.depth())
	{
		case CV_8U:
			mean /= 255.0;
			break;
		case CV_16U:
			mean /= 65535.0;
			break;
	}

	values_ModeMean.last_mean = values_ModeMean.current_mean; // save last value
	values_ModeMean.current_mean = mean;	// return current image's mean
	return (values_ModeMean.current_mean);	 
}

int ModeMean::calculateExposurelevelChange(float mean_diff)
{
	int ExposurelevelChange = shuttersteps / 2;

	// fast forward
	if ((fastforward) || (abs(mean_diff) > (setting_ModeMean.mean_threshold * 2.0))) {
		ExposurelevelChange = std::max(1.0, setting_ModeMean.mean_p0 + setting_ModeMean.mean_p1 * abs(mean_diff) + pow (setting_ModeMean.mean_p2 * mean_diff,2.0));
	}
	// slow forward
	else if (mean_diff > (setting_ModeMean.mean_threshold)) {
		ExposurelevelChange = std::max(1.0, setting_ModeMean.mean_p0 + setting_ModeMean.mean_p1 * abs(mean_diff));
	}

	return (ExposurelevelChange);
}


float ModeMean::calculateMeanAVG(void)
{
	// avg of mean history 
	Allsky::Debug("  > MeanCnt: %d, mean_historySize: %d\n", values_ModeMean.cnt, setting_ModeMean.historySize);

  // history
	mean_history[values_ModeMean.cnt % setting_ModeMean.historySize] = values_ModeMean.current_mean;
	int values = 0;
	float mean=0.0;
	for (int i=1; i <= setting_ModeMean.historySize; i++) {
		int idx =  (values_ModeMean.cnt + i) % setting_ModeMean.historySize;
		Allsky::Debug("  > i=%d: idx=%d mean=%1.4f exp=%d\n", i, idx, mean_history[idx], exp_history[idx]);
		mean += mean_history[idx] * (double) i;
		values += i;
	} 

	values_ModeMean.idx = (values_ModeMean.cnt + setting_ModeMean.historySize) % setting_ModeMean.historySize;
	values_ModeMean.idxN1 = (values_ModeMean.cnt + setting_ModeMean.historySize-1) % setting_ModeMean.historySize;

	//double dMean = mean_history[idx] - mean_history[idxN1];
	//int dExp = exp_history[idx] - exp_history[idxN1];

	// forcast (m_forcast = m_neu + diff = m_neu + m_neu - m_alt = 2*m_neu - m_alt)
	double mean_forecast = 2.0 * mean_history[values_ModeMean.idx] - mean_history[values_ModeMean.idxN1];
	mean_forecast = std::min((double) std::max((double) mean_forecast, 0.0), 1.0);
	Allsky::Debug("  > mean_forecast: %1.4f\n", mean_forecast);

	// gleiche Wertigkeit wie aktueller Wert
	mean += mean_forecast * setting_ModeMean.historySize;
	values += setting_ModeMean.historySize;

	Allsky::Debug("  > values: %d\n", values);

	mean = mean / (double) values;

	Allsky::Debug("  > mean (AVG): %1.4f\n", mean);
	return (mean);
}

int ModeMean::calculateExposureLevel(int ExposurelevelChange, float meanForcast)
{
	if (meanForcast < (setting_ModeMean.mean_value - (setting_ModeMean.mean_threshold))) {
		if ((Allsky::valuesCapture.gain < maxGain ) || (Allsky::valuesCapture.exposure_us < maxExposure_us)) {  // obere Grenze durch Gaim und shutter
			values_ModeMean.current_ExposureLevel += ExposurelevelChange;
		}
	}
	if (meanForcast > (setting_ModeMean.mean_value + setting_ModeMean.mean_threshold))  {
		if (Allsky::valuesCapture.exposure_us <= minExposure_us) { // untere Grenze durch shuttertime
			Allsky::Debug("  > ExposureTime_s too low - stop !\n");
		}
		else {
			values_ModeMean.current_ExposureLevel -= ExposurelevelChange;
		}
	}

	printf("values_ModeMean.current_ExposureLevel=%d\n", values_ModeMean.current_ExposureLevel);

	// check limits of exposurelevel 
	return (std::max(std::min((int)values_ModeMean.current_ExposureLevel, (int)ExposureLevelMax), (int)ExposureLevelMin));
}

 void ModeMean::calculateCaptureValues(int Exposurelevel)
{
	//#############################################################################################################
	// calculate gain und exposuretime
	double newGain = std::min(maxGain, std::max(1.0, pow(2.0, double(Exposurelevel)/pow(shuttersteps,2.0)) / (maxExposure_us/(double) US_IN_SEC))); 
	double deltaGain = newGain - Allsky::valuesCapture.gain; 
	if (deltaGain > 2.0) {
		Allsky::valuesCapture.gain += 2.0;
	}
	else if (deltaGain < -2.0) {
		Allsky::valuesCapture.gain -= 2.0;
	}
	else {
		Allsky::valuesCapture.gain = newGain;
	}
	Allsky::valuesCapture.gain_dB = AllskyHelper::gain2zwoGain(Allsky::valuesCapture.gain); 

  double current_Exposuretime_s = pow(2.0, double(Exposurelevel)/pow(shuttersteps,2.0))/Allsky::valuesCapture.gain;
  double current_Exposuretime_us = current_Exposuretime_s * (double) US_IN_SEC;
	printf("current_Exposuretime_us=%.2f\n", current_Exposuretime_us);
	current_Exposuretime_us = std::max((double)minExposure_us, current_Exposuretime_us);
	printf("current_Exposuretime_us=%.2f\n", current_Exposuretime_us);
	current_Exposuretime_us = std::min((double)maxExposure_us, current_Exposuretime_us);
	printf("current_Exposuretime_us=%.2f\n", current_Exposuretime_us);

	// result	
	Allsky::valuesCapture.exposure_us = current_Exposuretime_us;
}

void ModeMean::calculateNextExposureSettings(void)
{
	printf("--- new -----------------------------------------\n");
	// first calculate mean
	Allsky::Debug("  > mean (last): %1.4f\n", values_ModeMean.last_mean);
	Allsky::Debug("  > mean (cur): %1.4f\n", values_ModeMean.current_mean);
	calculateMean();
	Allsky::Debug("  > mean (last): %1.4f\n", values_ModeMean.last_mean);
	Allsky::Debug("  > mean (cur): %1.4f\n", values_ModeMean.current_mean);
	Allsky::Info("  > %s: expusure=%.1f sec, gain=%.1f [%d 0.1dB] mean: %.2f (diff=%.2f)\n", basename(Allsky::settings.image.fileName), (double)Allsky::valuesCapture.exposure_us/(double)US_IN_SEC, Allsky::valuesCapture.gain, Allsky::valuesCapture.gain_dB, values_ModeMean.current_mean, (values_ModeMean.current_mean - values_ModeMean.last_mean));

	// calculate AVG
	float meanAVG = calculateMeanAVG();

	// calculate mean difference from setpoint
	Allsky::Debug("  > mean (last): %1.4f\n", values_ModeMean.last_mean);
	Allsky::Debug("  > mean (cur): %1.4f\n", values_ModeMean.current_mean);
	double mean_diff = abs(meanAVG - setting_ModeMean.mean_value);
	Allsky::Debug("  > mean_diff: %1.4f\n", mean_diff);

	// calculate exposurelevel change
	int ExposurelevelChange = calculateExposurelevelChange(mean_diff);
	values_ModeMean.dExposurelevelChange = ExposurelevelChange-values_ModeMean.lastExposurelevelChange;
	values_ModeMean.lastExposurelevelChange = ExposurelevelChange;
	Allsky::Debug("  > ExposureChange: %d (%d)\n", ExposurelevelChange, values_ModeMean.dExposurelevelChange);


	values_ModeMean.last_ExposureLevel = values_ModeMean.current_ExposureLevel;
	values_ModeMean.current_ExposureLevel = calculateExposureLevel(ExposurelevelChange, meanAVG);
	Allsky::Debug("  > ExposureLevel: %d (%d) \n", values_ModeMean.current_ExposureLevel, values_ModeMean.current_ExposureLevel - values_ModeMean.last_ExposureLevel);

	// fastforward ?
	if ((values_ModeMean.current_ExposureLevel == (int)ExposureLevelMax) || (values_ModeMean.current_ExposureLevel == (int)ExposureLevelMin)) {
		fastforward = true;
		Allsky::Debug("  > FF aktiviert\n");
	}
	if ((abs(mean_history[values_ModeMean.idx] - setting_ModeMean.mean_value) < setting_ModeMean.mean_threshold) &&
		(abs(mean_history[values_ModeMean.idxN1] - setting_ModeMean.mean_value) < setting_ModeMean.mean_threshold)) {
		fastforward = false;
		Allsky::Debug("  > FF deaktiviert\n");
	}

	// capture values
	calculateCaptureValues(values_ModeMean.current_ExposureLevel);

	//#############################################################################################################
	// prepare for the next measurement
	if (quickstart > 0) {
		// xxxx  TODO: If already at the max exposure and we want to increase, then set quickstart to 0.
		// xxxx OR, if at a good exposure, set quickstart to 0.
		quickstart--;
	}

	// Exposure gilt fuer die naechste Messung
	values_ModeMean.cnt++;
	exp_history[values_ModeMean.cnt % setting_ModeMean.historySize] = values_ModeMean.current_ExposureLevel;

	//currentRaspistillSetting.shutter_us = ExposureTime_s * (double) US_IN_SEC;
	Allsky::Debug("  > Mean: %f, diff: %f, Exposure level:%d (%d), Exposure time:%1.8f s, gain:%1.2f [%d 0.1dB]\n", 
										values_ModeMean.current_mean, 
										mean_diff, 
										values_ModeMean.current_ExposureLevel, 
										values_ModeMean.current_ExposureLevel-exp_history[values_ModeMean.idx], 
										(double)Allsky::valuesCapture.exposure_us / (double)US_IN_SEC, 
										Allsky::valuesCapture.gain, 
										Allsky::valuesCapture.gain_dB);

}
