#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <tr1/memory>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <algorithm> 

#include "RPiHQ_raspistill.h"
#include "RPiHQ_modeMean.h"

double mean_history [5] = {0.5,0.5,0.5,0.5,0.5};
int MeanCnt = 0;

//double Kp = 50.0;
double Ta = 60.0;
double Tn = 600.0;
double Ek = 0.0;
double Esum = 0.0;
double STG = 0.0;

double Kp_Brightness = 1.5;
double Ek_Brightness = 0.0;
double Esum_Brightness = 0.0;
double STG_Brightness = 0.0;

double ExposureTest = 60;
bool createMaskHorizon = true;

// remove same areas
void RPiHQmask(const char* fileName)
{
	bool foundMaskFile = true;

	//std::cout <<  "RPiHQcalcMean Bild wird zur Analyse geladen" << std::endl;
    cv::Mat image = cv::imread(fileName, cv::IMREAD_UNCHANGED);

	//std::cout <<  "RPiHQcalcMean Laden fertig" << std::endl;
    if (!image.data)
    {
            std::cout << "Error reading file " << basename(fileName) << std::endl;
    }
	else {
		//Define your destination image
		cv::Mat dstImage = cv::Mat::zeros(image.size(), CV_8U);    


//##########################################################################################
// Test Mask horizon 
// https://docs.opencv.org/4.5.2/d3/d96/tutorial_basic_geometric_drawing.html

		dstImage = cv::Mat::zeros(image.size(), CV_8U);
		cv::Mat maskHorizon;
    	std::vector<int> compression_params;
    	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    	compression_params.push_back(9);
    	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    	compression_params.push_back(100);

		if (createMaskHorizon) {
			// 1. Define maskHorizon image
			maskHorizon = cv::Mat::zeros(image.size(), CV_8U);   

			// 2. circle
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*10/12, cv::Scalar(255, 255, 255), -1, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*3/12, cv::Scalar(0, 0, 0), 5, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*4/12, cv::Scalar(0, 0, 0), 5, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*5/12, cv::Scalar(0, 0, 0), 5, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*6/12, cv::Scalar(0, 0, 0), 5, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*7/12, cv::Scalar(0, 0, 0), 5, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*8/12, cv::Scalar(0, 0, 0), 5, 8, 0);
			cv::circle(maskHorizon, cv::Point(image.cols/2, image.rows/2), image.rows*9/12, cv::Scalar(0, 0, 0), 5, 8, 0);

			// 3. some guidelines
			cv::line( maskHorizon,
    			cv::Point( 0, image.rows/2 ), 
				cv::Point( maskHorizon.cols, maskHorizon.rows/2 ),
    			cv::Scalar( 0, 0, 0 ),
    			5,
    			cv::LINE_8 );

			cv::line( maskHorizon,
    			cv::Point( maskHorizon.cols/2, 0 ), 
				cv::Point( maskHorizon.cols/2, maskHorizon.rows ),
    			cv::Scalar( 0, 0, 0 ),
    			5,
    			cv::LINE_8 );

  			ellipse( maskHorizon,
       			cv::Point(maskHorizon.cols/2, maskHorizon.rows/2),
       			cv::Size( maskHorizon.cols, 1 ),
       			45,
       			0,
       			360,
       			cv::Scalar( 0, 0, 0 ),
       			5,
       			cv::LINE_8 );
  			ellipse( maskHorizon,
       			cv::Point(image.cols/2, image.rows/2),
       			cv::Size( image.cols, 1 ),
       			135,
       			0,
       			360,
       			cv::Scalar( 0, 0, 0 ),
       			5,
       			cv::LINE_8 );

			// 4. Save Mask to mask_template
    		cv::imwrite("mask_template.jpg", maskHorizon, compression_params);
			createMaskHorizon = false;
		}
		else {
    		maskHorizon = cv::imread("mask.jpg", cv::IMREAD_UNCHANGED);
    		if (!image.data)
    		{
    			maskHorizon = cv::imread("mask_template.jpg", cv::IMREAD_UNCHANGED);
				foundMaskFile = false;
    		}
		}

		// 5. Save masked image to mask_template_test or filename
		image.copyTo(dstImage, maskHorizon);

		if (foundMaskFile) {
			remove( fileName );
    		cv::imwrite(fileName, dstImage, compression_params);
		}
		else {
    		cv::imwrite("mask_template_test.jpg", dstImage, compression_params);
		}

//##########################################################################################
	}
}

// Build capture command to capture the image from the HQ camera
void RPiHQcalcMean(const char* fileName, int asiExposure, double asiGain, int asiBrightness, raspistillSetting &currentRaspistillSetting, modeMeanSetting &currentModeMeanSetting)
{

	// get old ExposureTime
	double ExposureTime = (double) currentRaspistillSetting.shutter/1000000.0;

	//std::cout <<  "RPiHQcalcMean Bild wird zur Analyse geladen" << std::endl;
    cv::Mat image = cv::imread(fileName, cv::IMREAD_UNCHANGED);

	//std::cout <<  "RPiHQcalcMean Laden fertig" << std::endl;
    if (!image.data)
    {
            std::cout << "Error reading file " << basename(fileName) << std::endl;
    }
	else {
		//Then define your mask image
		//cv::Mat mask = cv::Mat::zeros(image.size(), image.type());
		cv::Mat mask = cv::Mat::zeros(image.size(), CV_8U);

		//Define your destination image
		cv::Mat dstImage = cv::Mat::zeros(image.size(), CV_8U);    

		//I assume you want to draw the circle at the center of your image, with a radius of  mask.rows/3
		cv::circle(mask, cv::Point(mask.cols/2, mask.rows/2), mask.rows/3, cv::Scalar(255, 255, 255), -1, 8, 0);

		//Now you can copy your source image to destination image with masking
		image.copyTo(dstImage, mask);

/////////////////////////////////////////////////////////////////////////////////////
// Test focus
// https://stackoverflow.com/questions/7765810/is-there-a-way-to-detect-if-an-image-is-blurry
// https://drive.google.com/file/d/0B6UHr3GQEkQwYnlDY2dKNTdudjg/view?resourcekey=0-a73PvBnc3a2B5wztAV0QaA
 		cv::Mat lap;
    	cv::Laplacian(dstImage, lap, CV_64F);

    	cv::Scalar mu, sigma;
    	cv::meanStdDev(lap, mu, sigma);

    	double focusMeasure = sigma.val[0]*sigma.val[0];
	    std::cout <<  "focusMeasure: " << focusMeasure << std::endl;
/////////////////////////////////////////////////////////////////////////////////////
        

    	std::vector<int> compression_params;
    	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    	compression_params.push_back(9);
    	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    	compression_params.push_back(95);

    	cv::imwrite("test.jpg", dstImage, compression_params);

        //cv::Scalar mean_scalar = cv::mean(image);
        //cv::Scalar mean_scalar = cv::mean(dstImage);
        cv::Scalar mean_scalar = cv::mean(image, mask);
        double mean;
        switch (image.channels())
        {
            default: // mono case
			    std::cout <<  "mean_scalar.val[0]" << mean_scalar.val[0] << std::endl;
                mean = mean_scalar.val[0];
                break;
            case 3: // for color choose maximum channel
            case 4:
			    //std::cout <<  "image.channels() " << image.channels() << std::endl;
			    //std::cout <<  "mean_scalar.val[0] " << mean_scalar.val[0] << std::endl;
			    //std::cout <<  "mean_scalar.val[1] " << mean_scalar.val[1] << std::endl;
			    //std::cout <<  "mean_scalar.val[2] " << mean_scalar.val[2] << std::endl;
                mean = cv::max(mean_scalar[0], cv::max(mean_scalar[1], mean_scalar[2]));
                break;
        }
        // Scale to 0-1 range
        switch (image.depth())
        {
            case CV_8U:
                mean /= 255.0;
                break;
            case CV_16U:
                mean /= 65535.0;
                break;
        }
		 
		/* 
        if (ExposureTest != 1) 
			std::cout <<  basename(fileName) << " " << ExposureTest << " " << mean << " " << (mean_value - mean) << std::endl;
		ExposureTest = ExposureTest / 2;
		*/
		std::cout <<  basename(fileName) << " " << ExposureTime << " " << mean << " " << (currentModeMeanSetting.mean_value - mean) << std::endl;

		// Versuch PI Regler
		if (STG == 0.0) {
			STG = log (1000000.0); // Startbelichtung 1s
		}
		Ta = (double) asiExposure / 1000000.0;
		//printf("Ta: %1.8f\n", Ta);

		Tn = Ta * 10.0;
		//printf("Tn: %1.8f\n", Tn);

		Ek = currentModeMeanSetting.Kp * (currentModeMeanSetting.mean_value - mean);
		//printf("Ek: %1.8f\n", Ek);

		Esum = Esum + Ek;
		STG = STG + Ek + (Esum * Ta / Tn);

		// STG ... Steuergroeße (1...asiExposure) [us]
		// Versuch e^STG
		if (STG <  0.0) {
			STG = 0.0;
		}
		else if (STG > log (asiExposure)) {
			STG = log (asiExposure);
		}

		//printf("STG: %1.8f\n", STG);
		

		if (STG_Brightness == 0.0) {
			STG_Brightness = asiBrightness; // Start asiBrightness
		}

		Ek_Brightness = Kp_Brightness * (currentModeMeanSetting.mean_value - mean);
		//printf("Ek_Brightness: %1.8f\n", Ek_Brightness);

		Esum_Brightness = Esum_Brightness + Ek_Brightness;
		STG_Brightness = STG_Brightness + Ek_Brightness + (Esum_Brightness * Ta / Tn);

		if (STG_Brightness <  30.0) {
			STG_Brightness = 30.0;
		}
		else if (STG_Brightness > asiBrightness) {
			STG_Brightness = asiBrightness;
		}

		//printf("STG_Brightness: %1.8f\n", STG_Brightness);

		// avg of mean history 
		printf("MeanCnt: %d\n", MeanCnt);
		printf("mean_historySize: %d\n", currentModeMeanSetting.historySize);
		mean_history[MeanCnt % currentModeMeanSetting.historySize] = mean;
		int values = 0;
		mean=0.0;
		for (int i=1; i <= currentModeMeanSetting.historySize; i++) {
  			printf("i=%d: idx=%d mean=%1.4f\n", i, (MeanCnt + i) % currentModeMeanSetting.historySize, mean_history[(MeanCnt + i) % currentModeMeanSetting.historySize]);
			mean += mean_history[(MeanCnt + i) % currentModeMeanSetting.historySize] * (double) i;
			values += i;
		} 
		printf("values: %d\n", values);
		mean = mean / (double) values;
		MeanCnt++;

   		double mean_diff = abs(mean - currentModeMeanSetting.mean_value);
		printf("mean_diff: %1.4f\n", mean_diff);
    
		int ExposureChange = 1;
	    // fast forward
		if (mean_diff > (currentModeMeanSetting.mean_threshold * 2.0)) {
			ExposureChange = std::max(1.0, pow ((mean_diff * currentModeMeanSetting.fastforward * currentModeMeanSetting.shuttersteps),2.0));
		}
		printf("ExposureChange: %d\n", ExposureChange);

		//printf("asiExposure: %d\n", asiExposure);
		//printf("asiGain: %1.4f\n", asiGain);
		if (mean < (currentModeMeanSetting.mean_value - (currentModeMeanSetting.mean_threshold))) {
			if (currentModeMeanSetting.brightnessControl && (currentRaspistillSetting.brightness < asiBrightness)) {
				currentRaspistillSetting.brightness++;
			}
			else if ((currentRaspistillSetting.analoggain < asiGain) || (currentRaspistillSetting.shutter < asiExposure)) {  // obere Grenze durch Gaim und shutter
				currentModeMeanSetting.ExposureLevel += ExposureChange;
			}
		}
		if (mean > (currentModeMeanSetting.mean_value + currentModeMeanSetting.mean_threshold))  {
			if (ExposureTime <= 0.000001) { // untere Grenze durch shuttertime
				printf("ExposureTime to low - stop !\n");
				if (currentModeMeanSetting.brightnessControl && (currentRaspistillSetting.brightness > 0)) {
					currentRaspistillSetting.brightness--;
				}
				else {
					printf("Brightness to low - stop !\n");
				}
			}
			else {
				currentModeMeanSetting.ExposureLevel -= ExposureChange;
			}
		}
		else {
			if (currentModeMeanSetting.quickstart) {
				currentModeMeanSetting.quickstart = 0;
				printf("Quickstart deactivated !\n");
			}
		}
		if (currentModeMeanSetting.quickstart > 0) {
			currentModeMeanSetting.quickstart--;
		}

		// gain or exposure ?
		if (true) {
        	// change gain
			double newGain = std::min(asiGain, std::max(1.0, pow(2.0, double(currentModeMeanSetting.ExposureLevel)/pow(currentModeMeanSetting.shuttersteps,2.0)) / (asiExposure/1000000.0))); 
			double deltaGain = newGain - currentRaspistillSetting.analoggain; 
			if (deltaGain > 2.0) {
				currentRaspistillSetting.analoggain += 2.0;
			}
			else if (deltaGain < -2.0) {
				currentRaspistillSetting.analoggain -= 2.0;
			}
			else {
				currentRaspistillSetting.analoggain = newGain;
			}
			ExposureTime = pow(2.0, double(currentModeMeanSetting.ExposureLevel)/pow(currentModeMeanSetting.shuttersteps,2.0)) / currentRaspistillSetting.analoggain;
		}
		else {
			// change ExposureTime
			// calculate new ExposureTime
			ExposureTime = pow(2.0, double(currentModeMeanSetting.ExposureLevel)/pow(currentModeMeanSetting.shuttersteps,2.0)) / currentRaspistillSetting.analoggain;
			if ((ExposureTime > (asiExposure/1000000.0)) && (currentRaspistillSetting.analoggain < asiGain)) {
				currentRaspistillSetting.analoggain += 1.0;
				ExposureTime = pow(2.0, double(currentModeMeanSetting.ExposureLevel)/pow(currentModeMeanSetting.shuttersteps,2.0)) / currentRaspistillSetting.analoggain;
			}
			else if ((currentRaspistillSetting.analoggain >= 2) && (pow(2.0, double(currentModeMeanSetting.ExposureLevel)/pow(currentModeMeanSetting.shuttersteps,2.0)) / (currentRaspistillSetting.analoggain-1) <= (asiExposure/1000000.0))) {
				currentRaspistillSetting.analoggain -= 1.0;
				ExposureTime = pow(2.0, double(currentModeMeanSetting.ExposureLevel)/pow(currentModeMeanSetting.shuttersteps,2.0)) / currentRaspistillSetting.analoggain;
			}
		}

		if (ExposureTime > (asiExposure/1000000.0)) {
			ExposureTime = asiExposure/1000000.0;
		}
		else if (ExposureTime < 0.000001) {
			ExposureTime = 0.000001;
		}
		
		currentRaspistillSetting.shutter = ExposureTime * 1000 * 1000;
		printf("Mean: %1.4f Exposure level:%d Exposure time:%1.8f analoggain:%1.2f\n", mean, currentModeMeanSetting.ExposureLevel, ExposureTime, currentRaspistillSetting.analoggain);

	}
}
