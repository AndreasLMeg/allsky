#!/bin/bash

# mocking saveImage.sh
# usage:
#    mock_saveImage mock    ....... enables mocking
#    mock_saveImage         ....... disables mocking
#
#   /home/pi/allsky/scripts/saveImage.sh

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	if [ ! -e "${ALLSKY_HOME}/scripts/saveImage.sh_original" ] ; then
		cp ${ALLSKY_HOME}/scripts/saveImage.sh ${ALLSKY_HOME}/scripts/saveImage.sh_original 
	fi
	cp "${BASEDIR}/saveImage_mocked" ${ALLSKY_HOME}/scripts/saveImage.sh

	echo "${ALLSKY_HOME}/scripts/saveImage.sh mocked"
else
	cp ${ALLSKY_HOME}/scripts/saveImage.sh_original ${ALLSKY_HOME}/scripts/saveImage.sh
	echo "${ALLSKY_HOME}/scripts/saveImage.sh democked"
fi

