#!/bin/bash

BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	cp "${BASEDIR}/mock_variables" "${ALLSKY_MOCK_VARIABLES}"
	echo "${ALLSKY_MOCK_VARIABLES} copied"
else
	rm -f "${ALLSKY_MOCK_VARIABLES}"
	echo "${ALLSKY_MOCK_VARIABLES} removed"
fi

