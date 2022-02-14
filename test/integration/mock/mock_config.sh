#!/bin/bash -x

# mocking config.sh
# usage:
#    mock_config mock    ....... enables mocking
#    mock_config         ....... disables mocking
#
#   /home/pi/allsky/config/config.sh

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
  ls -la "${ALLSKY_HOME}"
	if [ ! -d "${ALLSKY_HOME}/config" ]; then
		mkdir -p "${ALLSKY_HOME}/config" 
		cp "${ALLSKY_HOME}/config_repo/config.sh.repo" ${ALLSKY_HOME}/config/config.sh
	else
	  echo "${ALLSKY_HOME}/config exists"
	fi

	if [ ! -e "${ALLSKY_HOME}/config/config.sh_original" ]; then
		cp ${ALLSKY_HOME}/config/config.sh ${ALLSKY_HOME}/config/config.sh_original 
	fi

	echo "${ALLSKY_HOME}config/config.sh mocked"
else
	cp ${ALLSKY_HOME}/config/config.sh_original ${ALLSKY_HOME}/config/config.sh
	echo "${ALLSKY_HOME}/config/config.sh democked"
fi

