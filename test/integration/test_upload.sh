#!/bin/bash

./mock/mock_variables.sh mock
CONFIG="${ALLSKY_HOME}/mock_variables"

# allskyTest
source "allskyTest/allskyTest.sh"

# mock lftp via PATH variable ! 
PATH="${IT_ROOT}/mock/:$PATH"

./mock/mock_config.sh mock
./mock/mock_ftp-settings.sh mock
./mock/mock_settings.sh mock


source "${ALLSKY_HOME}/variables.sh"
pushd ${ALLSKY_HOME}

set_config CAMERA RPiHQ ${ALLSKY_HOME}/config/config.sh 
CAMERA_SETTINGS_DIR="${ALLSKY_HOME}/config" 
# redefine the settings variable
CAMERA_SETTINGS="${CAMERA_SETTINGS_DIR}/settings_RPiHQ.json" 
# Path to the camera settings (exposure, gain, delay, overlay, etc) files.
# Do not change unless you know what you are doing.
set_config CAMERA_SETTINGS_DIR ${ALLSKY_HOME}/config ${ALLSKY_HOME}/config/config.sh

start
INFO i "Testcase: upload.sh"
INFO i "  *** Usage: upload.sh [--silent]  file_to_upload  directory  destination_file_name [unique_name] [local_directory]"
INFO i "  Where:"
INFO i "     '--silent' doesn't display any status messages"
INFO i "     'file_to_upload' is the path name of the file you want to upload."
INFO i "     'directory' is the directory ON THE SERVER the file should be uploaded to."
INFO i "     'destination_file_name' is the name the file should be called ON THE SERVER."
INFO i "     'unique_name' is an optional, temporary name to use when uploading the file."
INFO i "     'local_directory' is the name of an optional local directory the file should be"
INFO i "          copied to in addition to being uploaded."
INFO i "  "
INFO i "  For example: upload.sh  keogram-20210710.jpg  /keograms  keogram.jpg"

# setup
INFO i "SETUP"
SETUP
set_config REMOTE_HOST "ftp-settings_remote_host" ${ALLSKY_HOME}/config/ftp-settings.sh 
set_config REMOTE_USER "ftp-settings_remote_user" ${ALLSKY_HOME}/config/ftp-settings.sh 
set_config REMOTE_PASSWORD "ftp-settings_remote_password" ${ALLSKY_HOME}/config/ftp-settings.sh 
set_config LFTP_COMMANDS "ftp-settings_lftp_commands" ${ALLSKY_HOME}/config/ftp-settings.sh 


#echo "########################################"
#cat ${ALLSKY_HOME}/config/settings_RPiHQ.json
#cat ${ALLSKY_HOME}/config/config.sh
#echo "########################################"

# test execution
INFO i "EXECUTION"
${ALLSKY_HOME}/scripts/upload.sh
RET0=$?
${ALLSKY_HOME}/scripts/upload.sh 1
RET1=$?
${ALLSKY_HOME}/scripts/upload.sh 1 2
RET2=$?
${ALLSKY_HOME}/scripts/upload.sh 1 2 3
RET3=$?
${ALLSKY_HOME}/scripts/upload.sh ${IT_IMG_DIR}/${IT_TEST_IMG} "remote_dir" "destination_file_name"
RET4=$?


# evaluation
INFO i "EVALUATION"
TEST "without parameter: usage + retval=1" 1 $RET0
TEST "with 1 parameter: usage + retval=1" 1 $RET1
TEST "with 2 parameter: usage + retval=1" 1 $RET2
TEST "with 3 parameter, but not existing file: retval=2" 2 $RET3
TEST "with 3 parameter, existing file: retval=0" 0 $RET4

# cleanup
INFO i "TEARDOWN"
TEARDOWN

end
popd

./mock/mock_config.sh demock
./mock/mock_ftp-settings.sh demock
./mock/mock_settings.sh demock
./mock/mock_variables.sh demock

exit $ERR_CNT
