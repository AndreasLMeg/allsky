#include "../../src/camera_rpihq.h"
#include "../../src/camera_newcam.h"

//#include <gtest/gtest.h>
//#include <gmock/gmock.h>
//#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../../src/allsky.h"
#include "../../src/allskyStatic.h"

Camera* myCam;

//############################################################################################################
TEST(Camera, setWaitForNextCaptureANDgetWaitForNextCaptureANDwaitForNextCapture) {
	myCam = new CameraNewCam();
	myCam->setWaitForNextCapture(-100); EXPECT_EQ( myCam->getWaitForNextCapture(),0);
	myCam->setWaitForNextCapture(0); EXPECT_EQ( myCam->getWaitForNextCapture(),0);
	myCam->setWaitForNextCapture(1500000); EXPECT_EQ( myCam->getWaitForNextCapture(),1500000);

	// Acquire time
  time_t p1, t1 = time(&p1);
  // valid?
  ASSERT_NE(static_cast<time_t>(0), t1);
  ASSERT_NE(static_cast<time_t>(-1), t1);
  ASSERT_EQ(p1, t1);

  myCam->waitForNextCapture();
	
	time_t p2, t2 = time(&p2);
  // valid?
  ASSERT_NE(static_cast<time_t>(0), t2);
  ASSERT_NE(static_cast<time_t>(-1), t2);
  ASSERT_EQ(p2, t2);
  // Expect time progression
  ASSERT_LT(p1, p2);
  ASSERT_LE(t2 - t1, static_cast<time_t>(2));

	delete myCam;
}

//############################################################################################################
TEST(AllskyHelper, gain2zwoGain) {
	EXPECT_EQ( AllskyHelper::gain2zwoGain(1.0),0);
	EXPECT_EQ( AllskyHelper::gain2zwoGain(10.0),200);
	EXPECT_EQ( AllskyHelper::gain2zwoGain(100.0),400);
}

TEST(AllskyHelper, zwoGain2gain) {
	EXPECT_EQ( AllskyHelper::zwoGain2gain(0),1.0);
	EXPECT_EQ( AllskyHelper::zwoGain2gain(200),10);
	EXPECT_EQ( AllskyHelper::zwoGain2gain(400),100);
}

TEST(AllskyHelper, length_in_units) {
	myCam = new CameraNewCam();
	EXPECT_STREQ(myCam->length_in_units(987654321, true), "987.7 sec");
	EXPECT_STREQ(myCam->length_in_units(654321, true), "654.32 ms (0.65 sec)");
	EXPECT_STREQ(myCam->length_in_units(54321, true), "54.32 ms (0.05 sec)");
	EXPECT_STREQ(myCam->length_in_units(4321, true), "4.32 ms (0.00 sec)");
	EXPECT_STREQ(myCam->length_in_units(321, true), "321 us");
	EXPECT_STREQ(myCam->length_in_units(21, true), "21 us");
	EXPECT_STREQ(myCam->length_in_units(1, true), "1 us");
	EXPECT_STREQ(myCam->length_in_units(0, true), "0 us");
	EXPECT_STREQ(myCam->length_in_units(-1, true), "-1 us");
	// multi= false
	EXPECT_STREQ(myCam->length_in_units(987654321, false), "987.7 sec");
	EXPECT_STREQ(myCam->length_in_units(654321, false), "654.3 ms");
	EXPECT_STREQ(myCam->length_in_units(54321, false), "54.32 ms");
	EXPECT_STREQ(myCam->length_in_units(4321, false), "4.32 ms");
	EXPECT_STREQ(myCam->length_in_units(321, false), "321 us");
	EXPECT_STREQ(myCam->length_in_units(21, false), "21 us");
	EXPECT_STREQ(myCam->length_in_units(1, false), "1 us");
	EXPECT_STREQ(myCam->length_in_units(0, false), "0 us");
	EXPECT_STREQ(myCam->length_in_units(-1, false), "-1 us");
	delete myCam;
}

TEST(AllskyHelper, yesNo) {
	EXPECT_STREQ(AllskyHelper::yesNo(0), "No");
	EXPECT_STREQ(AllskyHelper::yesNo(1), "Yes");
	EXPECT_STREQ(AllskyHelper::yesNo(100), "Yes");
	EXPECT_STREQ(AllskyHelper::yesNo(-100), "Yes");
}

TEST(AllskyHelper, createRGB) {
	EXPECT_EQ( AllskyHelper::createRGB(0,0,0),0x000000);
	EXPECT_EQ( AllskyHelper::createRGB(0x01,0x02,0x03),0x010203);
	EXPECT_EQ( AllskyHelper::createRGB(0x101,0x102,0x103),0x010203);
	EXPECT_EQ( AllskyHelper::createRGB(0xFF,0x02,0x03),0xFF0203);
	EXPECT_EQ( AllskyHelper::createRGB(0x01,0xFF,0x03),0x01FF03);
	EXPECT_EQ( AllskyHelper::createRGB(0x01,0x02,0xFF),0x0102FF);
	// todo ?
	EXPECT_NE( AllskyHelper::createRGB(-1,-1,-1),0x000000) << "Todo ?";
}

TEST(AllskyHelper, DISABLED_getTime) {
	using ::testing::Ge;
	using ::testing::Le;
	using ::testing::MatchesRegex;
	// don't know how to use EXPECT_THAT !!!
	//EXPECT_THAT(AllskyHelper::getTime("%H:%M"), MatchesRegex("%d.:%d."));
	EXPECT_STREQ(AllskyHelper::getTime("%H:%M"), "*:*");
}

//############################################################################################################
TEST(Allsky, c) {
	myCam = new CameraNewCam();
	myCam->settings.tty = false;
	EXPECT_STREQ(myCam->c("green"), "");
	myCam->settings.tty = true;
	EXPECT_STREQ(myCam->c("green"), "green");
	delete myCam;
}

//############################################################################################################
TEST(ModeMean, calculateMeanAVG) {
	myCam = new CameraNewCam();
	myCam->values_ModeMean.current_mean = 0.3;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.3);
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.3);
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.3);
	myCam->values_ModeMean.current_mean = 1.0;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.66666669);
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.66666669);
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.66666669);
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.66666669);
  //-----------------------------------------------------------
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.66666669); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.8888889); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),1.0); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),1.0); myCam->values_ModeMean.cnt++;
  //-----------------------------------------------------------
	myCam->values_ModeMean.current_mean = 0.0;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.33333334); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.11111111); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.0); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.0); myCam->values_ModeMean.cnt++;
  //-----------------------------------------------------------
	myCam->values_ModeMean.current_mean = 1.0;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.66666669); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.8888889); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),1.0); myCam->values_ModeMean.cnt++;
	EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),1.0); myCam->values_ModeMean.cnt++;
  //-----------------------------------------------------------
	myCam->values_ModeMean.current_mean = 0.0; EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.33333334); myCam->values_ModeMean.cnt++;
	myCam->values_ModeMean.current_mean = 1.0; EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.77777779); myCam->values_ModeMean.cnt++;
	myCam->values_ModeMean.current_mean = 0.0; EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.22222222); myCam->values_ModeMean.cnt++;
	myCam->values_ModeMean.current_mean = 1.0; EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.77777779); myCam->values_ModeMean.cnt++;
	myCam->values_ModeMean.current_mean = 0.0; EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.22222222); myCam->values_ModeMean.cnt++;
	myCam->values_ModeMean.current_mean = 1.0; EXPECT_FLOAT_EQ( myCam->calculateMeanAVG(),0.77777779); myCam->values_ModeMean.cnt++;
	delete myCam;
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

	delete myCam;
}