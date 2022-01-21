#!/bin/bash

# Script to save a DAY or NIGHT image.


./mock_variables.sh mock
CONFIG="${ALLSKY_HOME}/mock_variables"

function start(){
    echo "$(date +'%T') ======================================================================" > ${ALLSKY_HOME}/mock_output
    echo "$(date +'%T') TEST: s $0: $@" >> ${ALLSKY_HOME}/mock_output
    ERR_CNT=0
}

function seperation(){
    echo "$(date +'%T') ----------------------------------------------------------------------" >> ${ALLSKY_HOME}/mock_output
}

function end(){
    echo "$(date +'%T') TEST: i ERR_CNT=$ERR_CNT" >> ${ALLSKY_HOME}/mock_output
    echo "$(date +'%T') ======================================================================" >> ${ALLSKY_HOME}/mock_output
    cat ${ALLSKY_HOME}/mock_output
}

function set_config(){
    sudo sed -i "s/\($1\s*=\s*\).*\$/\1$2/" $3
    echo "$(date +'%T') TEST: i $3: $1=$2" >> ${ALLSKY_HOME}/mock_output
}

function INFO(){
    echo "$(date +'%T') TEST: $1 $2" >> ${ALLSKY_HOME}/mock_output
}

function TEST(){
    if [ "$2"  != "$3" ] ; then
	    echo "$(date +'%T') TEST: e $1 FAIL ($2 != $3)" >> ${ALLSKY_HOME}/mock_output
        ERR_CNT=$((ERR_CNT+1))
    else
	    echo "$(date +'%T') TEST: i $1 OK" >> ${ALLSKY_HOME}/mock_output
    fi

}

./mock_upload.sh mock
./mock_saveImage.sh mock
./mock_config.sh mock

#  scripts/saveImage.sh: NIGHT /home/pi/allsky/tmp/image-20220117200347.jpg EXPOSURE_US=30000000 BRIGHTNESS=50 MEAN=0.329 AUTOEXPOSURE=1 AUTOGAIN=1 AUTOWB=0 WBR=2.80 WBB=2.00 GAIN=2.09 GAINDB=064 BIN=1 FLIP=3 BIT_DEPTH=08 FOCUS=014
ls -la images
cp ./images/image_4056_3040_night.jpg ${ALLSKY_HOME}/tmp

source "${ALLSKY_HOME}/variables.sh"
pushd ${ALLSKY_HOME}

set_config CAMERA RPiHQ ${ALLSKY_HOME}/config/config.sh 

start
bash -x ${ALLSKY_HOME}/scripts/saveImage.sh
seperation
INFO i "Testcase: saveImage.sh TEST"
${ALLSKY_HOME}/scripts/saveImage.sh TEST
seperation
INFO i "Testcase: saveImage.sh DAY"
${ALLSKY_HOME}/scripts/saveImage.sh DAY
seperation
INFO i "Testcase: saveImage.sh NIGHT"
${ALLSKY_HOME}/scripts/saveImage.sh NIGHT
#seperation
#bash -x ${ALLSKY_HOME}/scripts/saveImage.sh NIGHT ${ALLSKY_HOME}/tmp/image_4056_3040_night.jpg
seperation
INFO i "Testcase: saveImage.sh NIGHT ${ALLSKY_HOME}/tmp/image_test.jpg (RESIZE+CROP+AUTO_STRECH+RESIZE_UPLOADS)"
# setup
INFO i "SETUP"
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
# Path to the camera settings (exposure, gain, delay, overlay, etc) files.
# Do not change unless you know what you are doing.
set_config CAMERA_SETTINGS_DIR /etc/raspap ${ALLSKY_HOME}/config/config.sh


cp ${ALLSKY_HOME}/tmp/image_4056_3040_night.jpg ${ALLSKY_HOME}/tmp/image_test.jpg
identify ${ALLSKY_HOME}/tmp/image_test.jpg

# test
INFO i "EXECUTION"
bash -x ${ALLSKY_HOME}/scripts/saveImage.sh NIGHT ${ALLSKY_HOME}/tmp/image_test.jpg

# evaluation
INFO i "EVALUATION"
identify ${ALLSKY_HOME}/tmp/image_test.jpg
# Resize
#identify ${ALLSKY_HOME}/tmp/image_test.jpg | grep "JPEG 2028x1520 2028x1520+0+0 8-bit sRGB 406513B"
#TEST "${ALLSKY_HOME}/tmp/image_test.jpg resized (identify)" 0 $?
# Crop
#identify ${ALLSKY_HOME}/tmp/image_test.jpg | grep "JPEG 640x480 640x480+0+0 8-bit sRGB 46269B"
#TEST "${ALLSKY_HOME}/tmp/image_test.jpg croped (identify)" 0 $?
# stretch
identify ${ALLSKY_HOME}/tmp/image_test.jpg | grep "JPEG 640x480 640x480+0+0 8-bit sRGB 59459B"
TEST "${ALLSKY_HOME}/tmp/image_test.jpg streched (identify)" 0 $?
# resize and upload
identify ${ALLSKY_HOME}/tmp/resize-image_test.jpg_mock | grep "JPEG 958x720 958x720+0+0 8-bit sRGB 108202B"
TEST "${ALLSKY_HOME}/tmp/resize-image_test.jpg_mock resized for upload (identify)" 0 $?

# directory
# The 12 hours ago option ensures that we're always using today's date
# even at high latitudes where civil twilight can start after midnight.
DATE_DIR="${ALLSKY_IMAGES}/$(date -d '12 hours ago' +'%Y%m%d')"
# else
# During the daytime we alway save the file in today's directory.
#DATE_DIR="${ALLSKY_IMAGES}/$(date +'%Y%m%d')"
[ -d "${DATE_DIR}" ]; TEST "${DATE_DIR} exists" 0 $?
[ -d "${DATE_DIR}" ]; TEST "${DATE_DIR}/thumbnails exists" 0 $?

identify ${DATE_DIR}/image_test.jpg | grep "JPEG 640x480 640x480+0+0 8-bit sRGB 59459B"
TEST "${DATE_DIR}/image_test.jpg (identify)" 0 $?

identify ${DATE_DIR}/thumbnails/image_test.jpg | grep "JPEG 100x75 100x75+0+0 8-bit sRGB 2135B"
TEST "${DATE_DIR}/thumbnails/image_test.jpg (identify)" 0 $?

[ -e "${ALLSKY_HOME}/tmp/resize-image_test.jpg" ]
TEST "${ALLSKY_HOME}/tmp/resize-image_test.jpg removed" 1 $?

# cleanup
INFO i "TEARDOWN"
rm -f "${ALLSKY_HOME}/tmp/image_test.jpg"
rm -f "${DATE_DIR}/image_test.jpg"
rm -f "${DATE_DIR}/thumbnails/image_test.jpg"
rm -f "${ALLSKY_HOME}/tmp/resize-image_test.jpg_mock"

end
popd

./mock_upload.sh demock
./mock_saveImage.sh demock
./mock_config.sh demock
./mock_variables.sh demock

exit $ERR_CNT
