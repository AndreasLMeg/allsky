#!/bin/bash

# mocking endOfNight.sh
# usage:
#    mock_endOfNight mock    ....... enables mocking
#    mock_endOfNight         ....... disables mocking
#
#   /home/pi/allsky/scripts/endOfNight.sh

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	if [ ! -e "${ALLSKY_HOME}/scripts/endOfNight.sh_original" ] ; then
		cp ${ALLSKY_HOME}/scripts/endOfNight.sh ${ALLSKY_HOME}/scripts/endOfNight.sh_original 
	fi
	cp "${BASEDIR}/endOfNight_mocked" ${ALLSKY_HOME}/scripts/endOfNight.sh

	echo "${ALLSKY_HOME}/sccipts/endOfNight.sh mocked"
else
	cp ${ALLSKY_HOME}/scripts/endOfNight.sh_original ${ALLSKY_HOME}/scripts/endOfNight.sh
	echo "${ALLSKY_HOME}/scripts/endOfNight.sh democked"
fi

