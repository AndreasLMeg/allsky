#include "../Example.hpp"
#include "../../src/camera_rpihq.h"
#include "../../src/camera_newcam.h"

#include <gtest/gtest.h>

#include "../../src/allsky.h"
#include "../../src/allskyStatic.h"

Camera* myCam;


TEST(Allsky, gain2zwoGain) {
	EXPECT_EQ( Allsky::gain2zwoGain(1.0),0);
	EXPECT_EQ( Allsky::gain2zwoGain(10.0),200);
	EXPECT_EQ( Allsky::gain2zwoGain(100.0),400);
}

TEST(Allsky, zwoGain2gain) {
	EXPECT_EQ( Allsky::zwoGain2gain(0),1.0);
	EXPECT_EQ( Allsky::zwoGain2gain(200),10);
	EXPECT_EQ( Allsky::zwoGain2gain(400),100);
}

TEST(ModeMean, calculateExposurelevelChange) {
	// shuttersteps = 6;
	// mean_p0 = 5.0;		// ExposureChange (Steps) = p0 + p1 * diff + (p2*diff)^2
	// double mean_p1 = 20.0;
	// double mean_p2 = 45.0;
	// double mean_threshold	= 0.01;		// threshold value

	myCam = new CameraNewCam();
	// defaultvalue
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.0),3) << "erwarte shuttersteps (" << myCam->shuttersteps << "/2)";
  //-----------------------------------------------------------
	myCam->shuttersteps = 13;
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.0),6) << "erwarte shuttersteps (" << myCam->shuttersteps << "/2)";
	myCam->shuttersteps = 6;
  //-----------------------------------------------------------
	// allways fast forward 
	myCam->fastforward = true;
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.0),5) << "erwarte 5.0 + 0.0 * 20.0 + (0.0 * 45)^2 = 5";
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.5),521) << "erwarte 5.0 + 0.5 * 20.0 + (0.5 * 45)^2 = 521";
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.1),27) << "erwarte 5.0 + 0.1 * 20.0 + (0.1 * 45)^2 = 27";
	EXPECT_EQ( myCam->calculateExposurelevelChange(-0.5),521) << "erwarte 5.0 + 0.5 * 20.0 + (0.5 * 45)^2 = 521";
	myCam->fastforward = false;
  //-----------------------------------------------------------
	// fast forward 
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.0),3) << "erwarte 5.0 + 0.0 * 20.0 + (0.0 * 45)^2 = 2 (0.0 < mean_threshold 0.02)";
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.5),521) << "erwarte 5.0 + 0.5 * 20.0 + (0.5 * 45)^2 = 521";
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.1),27) << "erwarte 5.0 + 0.1 * 20.0 + (0.1 * 45)^2 = 27";
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.021),6) << "erwarte 5.0 + 0.021 * 20.0 + (0.021 * 45)^2 = 6";
	// normal forward 
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.020),5) << "erwarte 5.0 + 0.020 * 20.0 = 5";
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.011),5) << "erwarte 5.0 + 0.011 * 20.0 = 5";
	// shutter / 2 
	EXPECT_EQ( myCam->calculateExposurelevelChange(0.010),3) << "erwarte 3";
	// negative 
	EXPECT_EQ( myCam->calculateExposurelevelChange(-0.5),521) << "erwarte 5.0 + 0.5 * 20.0 + (0.5 * 45)^2 = 521";


}