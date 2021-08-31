// Simple keogram composition program using OpenCV
// Copyright 2018 Jarno Paananen <jarno.paananen@gmail.com>
// Based on a script by Thomas Jacquin
// Rotation added by Agustin Nunez @agnunez
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <glob.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <stdio.h>

#ifdef OPENCV_C_HEADERS
#include <opencv2/core/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#endif

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

using namespace cv;

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << KRED << "You need to pass 3 arguments: source directory, "
                             "image extension, output file"
                  << std::endl;
        std::cout << "Optionally you can pass after them: " << std::endl;
        std::cout << " -no-label                    - Disable hour labels" << std::endl;
        std::cout << " -fontname = Font Name        - Default = 0    - Font Types "
                     "(0-7), Ex. 0 = simplex, 4 = triplex, 7 = script"
                  << std::endl;
        std::cout << " -fontcolor = Font Color      - Default = 255 0 0  - Text "
                     "blue (BRG)"
                  << std::endl;
        std::cout << " -fonttype = Font Type        - Default = 8    - Font Line "
                     "Type,(0-2), 0 = AA, 1 = 8, 2 = 4"
                  << std::endl;
        std::cout << " -fontsize                    - Default = 2.0  - Text Font Size" << std::endl;
        std::cout << " -fontline                    - Default = 3    - Text Font "
                     "Line Thickness"
                  << std::endl;
        std::cout << " -rotate                      - Default = 0    - Rotation angle anticlockwise (deg)" << std::endl;
        std::cout << " -addRow                      - Default = 0    - standard, keogram maybe small" << std::endl;
        std::cout << "                                          1    - copy the same row to get nearby source images size" << std::endl;
        std::cout << "                                          2    - copy the neighbor row to get nearby source images size" << std::endl;
        std::cout << " -finishline                  - Default = <image.cols / 2>  - line of interest (finishline)" << std::endl;
        std::cout << "    ex: keogram ../images/current/ jpg keogram.jpg -fontsize 2" << std::endl;
        std::cout << "    ex: keogram . png /home/pi/allsky/keogram.jpg -no-label" << KNRM << std::endl;
        return 3;
    }

    std::string directory  = argv[1];
    std::string extension  = argv[2];
    std::string outputfile = argv[3];

    bool labelsEnabled = true;
    int fontFace       = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
    double fontScale   = 1;
    int fontType       = 8;
    int thickness      = 1;
    unsigned char fontColor[3]  = { 255, 255, 255 };
    double angle       = 0;
    int finishline     = -1;
    int addRow         = 0;

    // Handle optional parameters
    for (int a = 4; a < argc; ++a)
    {
        if (!strcmp(argv[a], "-no-label"))
        {
            labelsEnabled = false;
        }
        else if (!strcmp(argv[a], "-fontname"))
        {
            fontFace = atoi(argv[++a]);
        }
        else if (!strcmp(argv[a], "-fonttype"))
        {
            fontType = atoi(argv[++a]);
        }
        else if (!strcmp(argv[a], "-fontsize"))
        {
            fontScale = atof(argv[++a]);
        }
        else if (!strcmp(argv[a], "-fontline"))
        {
            thickness = atoi(argv[++a]);
        }
        else if (!strcmp(argv[a], "-fontcolor"))
        {
            fontColor[0] = atoi(argv[++a]);
            fontColor[1] = atoi(argv[++a]);
            fontColor[2] = atoi(argv[++a]);
        }
        else if (!strcmp(argv[a], "-rotate"))
        {
            angle = atoi(argv[++a]);
        }
        else if (!strcmp(argv[a], "-finishline"))
        {
            finishline = atoi(argv[++a]);
        }
        else if (!strcmp(argv[a], "-addRow"))
        {
            addRow = atoi(argv[++a]);
        }
    }

    glob_t files;
    std::string wildcard = directory + "/*." + extension;
    glob(wildcard.c_str(), 0, NULL, &files);
    if (files.gl_pathc == 0)
    {
        globfree(&files);
        std::cout << "No images found, exiting." << std::endl;
        return 0;
    }


    cv::Mat accumulated;
    Mat mask;

    int prevHour = -1;
    int expand = 1;
    int destCol = 0;

    for (size_t f = 0; f < files.gl_pathc; f++)
    {
        cv::Mat imagesrc = cv::imread(files.gl_pathv[f], cv::IMREAD_UNCHANGED);
        if (!imagesrc.data)
        {
            std::cout << "Error reading file " << basename(files.gl_pathv[f]) << std::endl;
            continue;
        }

        if (f==0) {
           	mask = cv::Mat::zeros(imagesrc.size(), CV_8U);
        	cv::circle(mask, cv::Point(mask.cols/2, mask.rows/2), mask.rows/3, cv::Scalar(255, 255, 255), -1, 8, 0);
        }
        std::cout << "[" << f + 1 << "/" << files.gl_pathc << "] " << basename(files.gl_pathv[f]) << std::endl;

	    //double angle = -36;
	    cv::Point2f center((imagesrc.cols-1)/2.0, (imagesrc.rows-1)/2.0);
	    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), imagesrc.size(), angle).boundingRect2f();
	    rot.at<double>(0,2) += bbox.width/2.0 - imagesrc.cols/2.0;
	    rot.at<double>(1,2) += bbox.height/2.0 - imagesrc.rows/2.0;
	    cv::Mat imagedst;
	    cv::warpAffine(imagesrc, imagedst, rot, bbox.size());
        if (accumulated.empty())
        {
            if (addRow != 0) {
                expand = imagesrc.cols / (int) files.gl_pathc;
                if (expand < 1)
                    expand = 1;
            }
            accumulated.create(imagedst.rows, files.gl_pathc * expand, imagesrc.type());
        }

        // Copy middle column to destination
        if (finishline == -1) {
            imagedst.col(imagedst.cols / 2).copyTo(accumulated.col(destCol));
            if (expand > 1) {
                for (int i=1; i < expand; i++) {
                    if (addRow == 1)
                        imagedst.col(imagedst.cols / 2).copyTo(accumulated.col(destCol+i));   //copy
                    else    
                        imagedst.col((imagedst.cols / 2) + i).copyTo(accumulated.col(destCol+i)); //neighbor
                }
            }
        }
        else {
            imagedst.col(finishline).copyTo(accumulated.col(destCol));
            if (expand > 1) {
                for (int i=1; i < expand; i++) {
                    if (addRow == 1)
                        imagedst.col(finishline).copyTo(accumulated.col(destCol+i));  //copy
                    else
                        imagedst.col(finishline + i).copyTo(accumulated.col(destCol+i)); //neighbor
                }
            }
        }
//#########################################################
        Scalar mean_scalar = cv::mean(imagesrc, mask);
        Vec3b color;
        double mean;
        double mean_Sum;
        double mean_maxValue;

        // Scale to 0-1 range
        switch (imagesrc.depth())
        {
            case CV_8U:
                mean_maxValue = 255.0/100.0;
                break;
            case CV_16U:
                mean_maxValue = 65535.0/100.0;
                break;
        }

        switch (imagesrc.channels())
        {
            default: // mono case
			    std::cout <<  "mean_scalar.val[0]" << mean_scalar.val[0] << std::endl;
                mean = mean_scalar.val[0] / mean_maxValue;
                break;
            case 3: // for color choose maximum channel
            case 4:
			    std::cout <<  "imagesrc.channels() " << imagesrc.channels() << std::endl;
			    //std::cout <<  "mean_scalar.val[0] " << mean_scalar.val[0] << std::endl;
			    //std::cout <<  "mean_scalar.val[1] " << mean_scalar.val[1] << std::endl;
			    //std::cout <<  "mean_scalar.val[2] " << mean_scalar.val[2] << std::endl;
                //mean = cv::max(mean_scalar[0], cv::max(mean_scalar[1], mean_scalar[2]));
                line( accumulated, Point(destCol,0), Point(destCol,100),  Scalar( 255, 255, 255 ), 1,  LINE_8 );
                if (expand > 1) {
                    for (int i=1; i < expand; i++) {
                        line( accumulated, Point(destCol+i,0), Point(destCol+i,100),  Scalar( 255, 255, 255 ), 1,  LINE_8 );
                    }
                }
                mean_Sum = 0;
                for (int channel=0; channel <= 2; channel++) {
                    mean = mean_scalar[channel] / mean_maxValue;
                    mean_Sum += mean;
                    color.val[0] = 0;
                    color.val[1] = 0;
                    color.val[2] = 0;
                    color.val[channel] = 255;
                    accumulated.at<cv::Vec3b>(Point(destCol,100-mean)) = color;
                    if (expand > 1) {
                        for (int i=1; i < expand; i++) {
                            accumulated.at<cv::Vec3b>(Point(destCol+i,100-mean)) = color;
                        }
                    }
                }
                mean_Sum = mean_Sum / 3.0;
                color.val[0] = 0;
                color.val[1] = 0;
                color.val[2] = 0;
                accumulated.at<cv::Vec3b>(Point(destCol,100-mean_Sum)) = color;
                if (expand > 1) {
                    for (int i=1; i < expand; i++) {
                        accumulated.at<cv::Vec3b>(Point(destCol+i,100-mean_Sum)) = color;
                    }
                }
                break;
        }

//#########################################################

        if (labelsEnabled)
        {
            struct stat s;
            stat(files.gl_pathv[f], &s);

            struct tm *t = localtime(&s.st_mtime);

            if (t->tm_hour != prevHour)
            {
                if (prevHour != -1)
                {
                    // Draw a dashed line and label for hour
                    cv::LineIterator it(accumulated, cv::Point(destCol, 0), cv::Point(destCol, accumulated.rows));
                    for (int i = 0; i < it.count; i++, ++it)
                    {
                        // 4 pixel dashed line
                        if (i & 4)
                        {
                            uchar *p = *it;
                            for (int c = 0; c < it.elemSize; c++)
                            {
                                *p = ~(*p);
                                p++;
                            }
                        }
                    }

                    if (t->tm_hour == 0) {
                        // Draw date
                        char    time_buf[256];
                        (void) strftime(time_buf, sizeof (time_buf), "%m-%d-%Y", t);
                        std::string text(time_buf);
                        int baseline      = 0;
                        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);

                        if (destCol - textSize.width >= 0)
                        {
                            cv::putText(accumulated, text,
                                        cv::Point(destCol - textSize.width, accumulated.rows - (2.5 * textSize.height)), fontFace,
                                        fontScale, cv::Scalar(0, 0, 0), thickness+1,
                                        fontType);
                            cv::putText(accumulated, text,
                                        cv::Point(destCol - textSize.width, accumulated.rows - (2.5 * textSize.height)), fontFace,
                                        fontScale, cv::Scalar(fontColor[0], fontColor[1], fontColor[2]), thickness,
                                        fontType);
                        }
                    }

                    // Draw text label to the left of the dash
                    char hour[3];
                    snprintf(hour, 3, "%02d", t->tm_hour);
                    std::string text(hour);
                    int baseline      = 0;
                    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);

                    if (destCol - textSize.width >= 0)
                    {
                        cv::putText(accumulated, text,
                                    cv::Point(destCol - textSize.width, accumulated.rows - textSize.height), fontFace,
                                    fontScale, cv::Scalar(0, 0, 0), thickness+1,
                                    fontType);
                        cv::putText(accumulated, text,
                                    cv::Point(destCol - textSize.width, accumulated.rows - textSize.height), fontFace,
                                    fontScale, cv::Scalar(fontColor[0], fontColor[1], fontColor[2]), thickness,
                                    fontType);
                    }
                }
                prevHour = t->tm_hour;
            }
        }

        destCol +=expand;
    }
    globfree(&files);

    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);

    cv::imwrite(outputfile, accumulated, compression_params);
}
