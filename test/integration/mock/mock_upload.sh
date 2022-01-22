#!/bin/bash

# mocking upload.sh
# usage:
#    mock_upload mock    ....... enables mocking
#    mock_upload         ....... disables mocking
#
#   /home/pi/allsky/scripts/saveImage.sh

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	if [ ! -e "${ALLSKY_HOME}/scripts/upload.sh_original" ] ; then
		cp ${ALLSKY_HOME}/scripts/upload.sh ${ALLSKY_HOME}/scripts/upload.sh_original 
	fi
	cp "${BASEDIR}/upload_mocked" ${ALLSKY_HOME}/scripts/upload.sh

	echo "${ALLSKY_HOME}/scripts/upload.sh mocked"
else
	cp ${ALLSKY_HOME}/scripts/upload.sh_original ${ALLSKY_HOME}/scripts/upload.sh
	echo "${ALLSKY_HOME}/scripts/upload.sh democked"
fi

