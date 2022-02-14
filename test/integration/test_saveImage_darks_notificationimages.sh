#!/bin/bash

# Script to save a DAY or NIGHT image.


./mock/mock_variables.sh mock
CONFIG="${ALLSKY_HOME}/mock_variables"

# allskyTest
source "allskyTest/allskyTest.sh"

./mock/mock_upload.sh mock
./mock/mock_saveImage.sh mock
./mock/mock_config.sh mock
./mock/mock_settings.sh mock

#  scripts/saveImage.sh: NIGHT /home/pi/allsky/tmp/image-20220117200347.jpg EXPOSURE_US=30000000 BRIGHTNESS=50 MEAN=0.329 AUTOEXPOSURE=1 AUTOGAIN=1 AUTOWB=0 WBR=2.80 WBB=2.00 GAIN=2.09 GAINDB=064 BIN=1 FLIP=3 BIT_DEPTH=08 FOCUS=014

source "${ALLSKY_HOME}/variables.sh"
pushd ${ALLSKY_HOME}

set_config CAMERA RPiHQ ${ALLSKY_HOME}/config/config.sh
CAMERA_SETTINGS_DIR="${ALLSKY_HOME}/config" 
# redefine the settings variable
CAMERA_SETTINGS="${CAMERA_SETTINGS_DIR}/settings_RPiHQ.json" 
# Path to the camera settings (exposure, gain, delay, overlay, etc) files.
# Do not change unless you know what you are doing.
set_config CAMERA_SETTINGS_DIR ${ALLSKY_HOME}/config ${ALLSKY_HOME}/config/config.sh

cat ${ALLSKY_HOME}/config/config.sh| grep CAMERA

start
INFO i "Testcase: saveImage.sh NIGHT ${IT_IMG_DIR}/dark.jpg (RESIZE+CROP+AUTO_STRECH+RESIZE_UPLOADS)"
# setup
INFO i "SETUP"
SETUP

set_config IMG_RESIZE true ${ALLSKY_HOME}/config/config.sh
set_config IMG_WIDTH 2028 ${ALLSKY_HOME}/config/config.sh
set_config IMG_HEIGHT 1520 ${ALLSKY_HOME}/config/config.sh

set_config CROP_IMAGE true ${ALLSKY_HOME}/config/config.sh
set_config CROP_WIDTH 640 ${ALLSKY_HOME}/config/config.sh
set_config CROP_HEIGHT 480 ${ALLSKY_HOME}/config/config.sh
set_config CROP_OFFSET_X 0 ${ALLSKY_HOME}/config/config.sh
set_config CROP_OFFSET_Y 0 ${ALLSKY_HOME}/config/config.sh
# Auto stretch images saved at night.
set_config AUTO_STRETCH true ${ALLSKY_HOME}/config/config.sh
set_config AUTO_STRETCH_AMOUNT 10 ${ALLSKY_HOME}/config/config.sh
set_config AUTO_STRETCH_MID_POINT 10% ${ALLSKY_HOME}/config/config.sh
# Create thumbnails of images.  If you are not running the WebUI consider changing this to "false".
set_config IMG_CREATE_THUMBNAILS true ${ALLSKY_HOME}/config/config.sh
# Set to "true" to upload the current image to your website.
set_config IMG_UPLOAD true ${ALLSKY_HOME}/config/config.sh
# If UPLOAD_IMG is "true", upload images every IMG_UPLOAD_FREQUENCY frames, e.g., every 5 frames.
# 1 uploades every frame.
#IMG_UPLOAD_FREQUENCY=1

# Resize uploaded images.  Change the size to fit your sensor.
set_config RESIZE_UPLOADS true ${ALLSKY_HOME}/config/config.sh 
set_config RESIZE_UPLOADS_SIZE "962x720" ${ALLSKY_HOME}/config/config.sh 
# Set to "true" if you want to enable dark frame subtraction on your night-time images.
# You must first create dark frames - see the README.md file for instructions.
set_config DARK_FRAME_SUBTRACTION true ${ALLSKY_HOME}/config/config.sh

set_setting "darkframe" "1" ${ALLSKY_HOME}/config/settings_RPiHQ.json
set_setting "notificationimages" "1" ${ALLSKY_HOME}/config/settings_RPiHQ.json

echo "########################################"
cat ${ALLSKY_HOME}/config/settings_RPiHQ.json
cat ${ALLSKY_HOME}/config/config.sh
echo "########################################"


cp ${IT_IMG_DIR}/${IT_TEST_IMG} ${IT_IMG_DIR}/dark.jpg
identify ${IT_IMG_DIR}/dark.jpg

# test
INFO i "EXECUTION"
${ALLSKY_HOME}/scripts/saveImage.sh NIGHT ${IT_IMG_DIR}/dark.jpg

# evaluation
INFO i "EVALUATION"
identify ${IT_IMG_DIR}/dark.jpg

# directory
# The 12 hours ago option ensures that we're always using today's date
# even at high latitudes where civil twilight can start after midnight.
DATE_DIR="${ALLSKY_IMAGES}/$(date -d '12 hours ago' +'%Y%m%d')"

[ -d "${ALLSKY_DARKS}" ]; TEST "${ALLSKY_DARKS} exists" 0 $?

# Darkframe activ !
[ -e "${ALLSKY_DARKS}/dark.jpg" ]
TEST "${ALLSKY_DARKS}/dark.jpg darkframe exists" 0 $?
identify ${ALLSKY_DARKS}/dark.jpg | grep "JPEG 4056x3040 4056x3040+0+0 8-bit sRGB 1512650B"
TEST "${ALLSKY_DARKS}/dark.jpg darkframe -> image should not change (identify)" 0 $?

[ -e "${DATE_DIR}/dark.jpg" ]
TEST "${DATE_DIR}/dark.jpg darkframe -> should not exists in date-dir" 1 $?

[ -e "${DATE_DIR}/thumbnails/dark.jpg" ]
TEST "${DATE_DIR}/thumbnails/dark.jpg darkframe -> should not exists in date-dir/thumbnails" 1 $?

[ -e "${IT_IMG_DIR}/dark.jpg" ]
TEST "${IT_IMG_DIR}/dark.jpg removed by script" 1 $?

[ -e "${IT_IMG_DIR}/resize-dark.jpg" ]
TEST "${IT_IMG_DIR}/resize-dark.jpg darkframe -> not to resize and upload" 1 $?

[ -e "${IT_IMG_DIR}/${IT_DEST_IMG}" ]
TEST "${IT_IMG_DIR}/${IT_DEST_IMG} does not exists, because notification is made from capture (is not running during test)" 1 $?  


identify ${IT_IMG_DIR}/${IT_DEST_IMG}
identify ${IT_IMG_DIR}/${IT_DEST_IMG} | grep "JPEG 4056x3040 4056x3040+0+0 8-bit sRGB 1512650B"
TEST "${IT_IMG_DIR}/${IT_DEST_IMG}  notificationimages=1, dont show darkframe (identify)" 1 $?

[ -e "${ALLSKY_HOME}/${IT_DEST_IMG}" ]
TEST "${ALLSKY_HOME}/${IT_DEST_IMG} should not exists" 1 $?  


# cleanup
INFO i "TEARDOWN"
TEARDOWN
ls -la ${ALLSKY_HOME}/tmp
ls -la ${ALLSKY_HOME}/darks
#tree ${ALLSKY_HOME} -Dsp
rm -f "${ALLSKY_HOME}/tmp/dark.jpg"
rm -f "${DATE_DIR}/dark.jpg"
rm -f "${DATE_DIR}/thumbnails/dark.jpg"
rm -f "${ALLSKY_HOME}/tmp/resize-dark.jpg_mock"
rm -f "${ALLSKY_HOME}/darks/dark.jpg"
rm -f "${ALLSKY_HOME}/tmp/image.jpg"

end
popd

./mock/mock_upload.sh demock
./mock/mock_saveImage.sh demock
./mock/mock_config.sh demock
./mock/mock_settings.sh demock
./mock/mock_variables.sh demock

exit $ERR_CNT
