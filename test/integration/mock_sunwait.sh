#!/bin/bash

# mocking sunwait
# usage:
#    mock_sunwait mock    ....... enables mocking
#    mock_sunwait         ....... disables mocking


BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	if [ ! -e "/usr/local/bin/sunwait_original" ] ; then
		sudo cp /usr/local/bin/sunwait /usr/local/bin/sunwait_original
	fi
	sudo cp "${BASEDIR}/sunwait_mocked" /usr/local/bin/sunwait

	echo "sunwait mocked"
else
	sudo cp /usr/local/bin/sunwait_original /usr/local/bin/sunwait
	echo "sunwait democked"
fi

