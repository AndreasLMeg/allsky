#!/bin/bash

# mocking darkCapture.sh
# usage:
#    mock_darkCapture mock    ....... enables mocking
#    mock_darkCapture         ....... disables mocking
#
#   /home/pi/allsky/scripts/darkCapture.sh

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	if [ ! -e "${ALLSKY_HOME}/scripts/darkCapture.sh_original" ] ; then
		cp ${ALLSKY_HOME}/scripts/saveImage.sh ${ALLSKY_HOME}/scripts/darkCapture.sh_original 
	fi
	cp "${BASEDIR}/darkCapture_mocked" ${ALLSKY_HOME}/scripts/darkCapture.sh

	echo "${ALLSKY_HOME}/scripts/darkCapture.sh mocked"
else
	cp ${ALLSKY_HOME}/scripts/darkCapture.sh_original ${ALLSKY_HOME}/scripts/darkCapture.sh
	echo "${ALLSKY_HOME}/scripts/darkCapture.sh democked"
fi

