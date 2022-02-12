#!/bin/bash

# mocking config.sh
# usage:
#    mock_config mock    ....... enables mocking
#    mock_config         ....... disables mocking
#
#   /home/pi/INTEGRATIONTEST/config_repo/settings_ZWO.json.repo
#   /home/pi/INTEGRATIONTEST/config_repo/settings_RPiHQ.json.repo


BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	# ZWO
	if [ ! -e "${ALLSKY_HOME}/config/settings_ZWO.json_original" ] ; then
		if [ ! -e "${ALLSKY_HOME}/config/settings_ZWO.json" ] ; then
			touch ${ALLSKY_HOME}/config/settings_ZWO.json
		fi
		cp ${ALLSKY_HOME}/config/settings_ZWO.json ${ALLSKY_HOME}/config/settings_ZWO.json_original 
	fi
	cp "${ALLSKY_HOME}/config_repo/settings_ZWO.json.repo" ${ALLSKY_HOME}/config/settings_ZWO.json
	echo "${ALLSKY_HOME}config/settings_ZWO.json mocked"

	# RPiHQ
	if [ ! -e "${ALLSKY_HOME}/config/settings_RPiHQ.json_original" ] ; then
		if [ ! -e "${ALLSKY_HOME}/config/settings_RPiHQ.json" ] ; then
			touch ${ALLSKY_HOME}/config/settings_RPiHQ.json
		fi
		cp ${ALLSKY_HOME}/config/settings_RPiHQ.json ${ALLSKY_HOME}/config/settings_RPiHQ.json_original 
	fi
	cp "${ALLSKY_HOME}/config_repo/settings_RPiHQ.json.repo" ${ALLSKY_HOME}/config/settings_RPiHQ.json
	echo "${ALLSKY_HOME}config/settings_RPiHQ.json mocked"

	# /home/pi/INTEGRATIONTEST/config_repo/ftp-settings.sh.repo
	if [ ! -e "${ALLSKY_HOME}/config/ftp-settings.sh_original" ] ; then
		if [ ! -e "${ALLSKY_HOME}/config/ftp-settings.sh" ] ; then
			touch ${ALLSKY_HOME}/config/ftp-settings.sh
		fi
		cp ${ALLSKY_HOME}/config/ftp-settings.sh ${ALLSKY_HOME}/config/ftp-settings.sh_original 
	fi
	cp "${ALLSKY_HOME}/config_repo/ftp-settings.sh.repo" ${ALLSKY_HOME}/config/ftp-settings.sh
	echo "${ALLSKY_HOME}config/ftp-settings.sh mocked"
else
	# ZWO
	cp ${ALLSKY_HOME}/config/settings_ZWO.json_original ${ALLSKY_HOME}/config/settings_ZWO.json
	echo "${ALLSKY_HOME}/config/settings_ZWO democked"

	# RPiHQ
	cp ${ALLSKY_HOME}/config/settings_RPiHQ.json_original ${ALLSKY_HOME}/config/settings_RPiHQ.json
	echo "${ALLSKY_HOME}/config/settings_RPiHQ democked"

	# /home/pi/INTEGRATIONTEST/config_repo/ftp-settings.sh.repo
	cp ${ALLSKY_HOME}/config/ftp-settings.sh_original ${ALLSKY_HOME}/config/ftp-settings.sh
	echo "${ALLSKY_HOME}/config/ftp-settings.sh democked"
fi
