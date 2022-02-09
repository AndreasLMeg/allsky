#include "../../src/camera_rpihq.h"
#include "../../src/camera_newcam.h"

//#include <gtest/gtest.h>
//#include <gmock/gmock.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../../src/allsky.h"
#include "../../src/allskyStatic.h"

Camera* myCam;

class AllskyUT : public Allsky {
  public:
		AllskyUT () {};
		AllskyUT (AllskyExternalsInterface *Externals) 
		{
			Allsky::externals = Externals;
		};

		void setupForCapture(void) {};
		/* Camera captures one image */
		int capture(void) { return (0);};
		/* all camara depending things after the capture */
		void postCapture(void) {};
		void waitForNextCapture(void) {};
		void prepareForDayOrNight(void) {};
		void kill();
		void initCamera();
};

AllskyExternals MyAllskyExternals;
Allsky* myAllsky = new AllskyUT(&MyAllskyExternals);

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

TEST(AllskyExternals, createRGB) {
	EXPECT_EQ( myAllsky->externals->createRGB(0,0,0),0x000000);
	EXPECT_EQ( myAllsky->externals->createRGB(0x01,0x02,0x03),0x010203);
	EXPECT_EQ( myAllsky->externals->createRGB(0x101,0x102,0x103),0x010203);
	EXPECT_EQ( myAllsky->externals->createRGB(0xFF,0x02,0x03),0xFF0203);
	EXPECT_EQ( myAllsky->externals->createRGB(0x01,0xFF,0x03),0x01FF03);
	EXPECT_EQ( myAllsky->externals->createRGB(0x01,0x02,0xFF),0x0102FF);
	// todo ?
	EXPECT_NE( myAllsky->externals->createRGB(-1,-1,-1),0x000000) << "Todo ?";
}

TEST(AllskyExternals, getTime) {
	using ::testing::MatchesRegex;
	EXPECT_THAT(myAllsky->externals->getTime ("%H:%M"), MatchesRegex("[0-9]+:[0-9]+"));
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

class MockAllskyExternalsInterface : public AllskyExternalsInterface {
 public:
  MockAllskyExternalsInterface() {}
	MOCK_METHOD(int, system, (const char *__command), (override));
	MOCK_METHOD(unsigned int, s_sleep, (unsigned int __seconds), (override));
  MOCK_METHOD(std::string, exec, (const char *cmd), (override));
  MOCK_METHOD(char *, getTime, (char const *tf), (override));
  MOCK_METHOD(char *, formatTime, (timeval t, char const *tf), (override));
  MOCK_METHOD(timeval, getTimeval, (), (override));
	MOCK_METHOD(void, cvText, (cv::Mat img, const char *text, int x, int y, double fontsize, int linewidth, int linetype, int fontname, int fontcolor[], int imgtype, int outlinefont), (override));
	MOCK_METHOD(unsigned long, createRGB, (int r, int g, int b), (override));
};

TEST(Allsky, calculateTimeToNightTime) {
	using ::testing::_;
	using ::testing::Return;
	//using ::testing::StrictMock;

	MockAllskyExternalsInterface mockAllskyExternals;
  EXPECT_CALL(mockAllskyExternals, exec(_))
	.Times(1)
	.WillRepeatedly(Return("14:00, 08:00"));
  EXPECT_CALL(mockAllskyExternals, getTime(_))
	.Times(1)
	.WillRepeatedly(Return((char*)"00:00"));
	
	Allsky* myAllsky = new AllskyUT(&mockAllskyExternals);
  EXPECT_EQ(myAllsky->calculateTimeToNightTime(), 50400);
}

TEST(Allsky, waitToFix) {
	using ::testing::_;
	using ::testing::Return;

	MockAllskyExternalsInterface mockAllskyExternals;

  EXPECT_CALL(mockAllskyExternals, s_sleep(5))
	.Times(1)
	.WillRepeatedly(Return(0));

  EXPECT_CALL(mockAllskyExternals, s_sleep(100000))
	.Times(1)
	.WillRepeatedly(Return(0));
	
	Allsky* myAllsky = new AllskyUT(&mockAllskyExternals);
  myAllsky->waitToFix("Test");
}

TEST(Allsky, waitToFixSettingsNotificationImages) {
	using ::testing::_;
	using ::testing::Return;

	MockAllskyExternalsInterface mockAllskyExternals;
  EXPECT_CALL(mockAllskyExternals, system(_))
	.Times(1)
	.WillRepeatedly(Return(0));

  EXPECT_CALL(mockAllskyExternals, s_sleep(5))
	.Times(1)
	.WillRepeatedly(Return(0));

  EXPECT_CALL(mockAllskyExternals, s_sleep(100000))
	.Times(1)
	.WillRepeatedly(Return(0));
	
	Allsky* myAllsky = new AllskyUT(&mockAllskyExternals);
	myAllsky->settings.notificationImages = true;
  myAllsky->waitToFix("Test");
}

TEST(Allsky, preCapture) {
	using ::testing::_;
	using ::testing::Return;

  timeval retval_timeval;
	retval_timeval.tv_sec = 0;
	retval_timeval.tv_usec = 0;
	
	MockAllskyExternalsInterface mockAllskyExternals;
  EXPECT_CALL(mockAllskyExternals, getTimeval())
	.Times(1)
	.WillRepeatedly(Return(retval_timeval));

  EXPECT_CALL(mockAllskyExternals, formatTime(_,_))
	.Times(2)
	.WillRepeatedly(Return((char*)"Formated Timestring"));

	Allsky* myAllsky = new AllskyUT(&mockAllskyExternals);
	myAllsky->currentExposure_us = 0;
  myAllsky->preCapture();
}

TEST(Allsky, dayOrNightNotChanged) {
	using ::testing::_;
	using ::testing::Return;
	
	myAllsky->runtime.lastDayOrNight="NIGHT"; myAllsky->runtime.dayOrNight="NIGHT"; EXPECT_EQ(myAllsky->dayOrNightNotChanged(), true);
	myAllsky->runtime.lastDayOrNight="NIGHT"; myAllsky->runtime.dayOrNight="DAY"; EXPECT_EQ(myAllsky->dayOrNightNotChanged(), false);
	myAllsky->runtime.lastDayOrNight="DAY"; myAllsky->runtime.dayOrNight="DAY"; EXPECT_EQ(myAllsky->dayOrNightNotChanged(), true);
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