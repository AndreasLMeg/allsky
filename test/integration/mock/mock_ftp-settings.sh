#!/bin/bash -x

# mocking ftp-settings.sh
# usage:
#    mock_ftp-settings mock    ....... enables mocking
#    mock_ftp-settings         ....... disables mocking
#
#   /home/pi/allsky/config/ftp-settings.sh

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
  ls -la "${ALLSKY_HOME}"
	if [ ! -d "${ALLSKY_HOME}/config" ]; then
		mkdir -p "${ALLSKY_HOME}/config" 
	else
	  echo "${ALLSKY_HOME}/config exists"
	fi

	if [ ! -e "${ALLSKY_HOME}/config/ftp-settings.sh_original" ]; then
		cp ${ALLSKY_HOME}/config/ftp-settings.sh ${ALLSKY_HOME}/config/ftp-settings.sh_original 
	fi
	cp "${ALLSKY_HOME}/config_repo/ftp-settings.sh.repo" ${ALLSKY_HOME}/config/ftp-settings.sh

	echo "${ALLSKY_HOME}config/ftp-settings.sh mocked"
else
	cp ${ALLSKY_HOME}/config/ftp-settings.sh_original ${ALLSKY_HOME}/config/ftp-settings.sh
	echo "${ALLSKY_HOME}/config/ftp-settings.sh democked"
fi

