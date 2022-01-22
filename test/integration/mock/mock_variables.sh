#!/bin/bash

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	cp "${BASEDIR}/mock_variables" "${ALLSKY_MOCK_VARIABLES}"
	echo "${ALLSKY_MOCK_VARIABLES} copied"

	if [ ! -e "${ALLSKY_HOME}/variables.sh_original" ] ; then
		cp ${ALLSKY_HOME}/variables.sh ${ALLSKY_HOME}/variables.sh_original 
	fi
	cp "${BASEDIR}/variables_mocked" ${ALLSKY_HOME}/variables.sh

	echo "${ALLSKY_HOME}/variables.sh mocked"
else
	rm -f "${ALLSKY_MOCK_VARIABLES}"
	echo "${ALLSKY_MOCK_VARIABLES} removed"

	cp ${ALLSKY_HOME}/variables.sh_original ${ALLSKY_HOME}/variables.sh
	echo "${ALLSKY_HOME}/variables.sh democked"
fi
