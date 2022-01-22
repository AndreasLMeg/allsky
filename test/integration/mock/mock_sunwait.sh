#!/bin/bash

# mocking sunwait
# usage:
#    mock_sunwait mock    ....... enables mocking
#    mock_sunwait         ....... disables mocking


BASEDIR=$(dirname "$0")
source ${BASEDIR}/mock_variables

if [ "$1" == "mock" ]; then
	if [ ! -e "/usr/bin/sunwait_original" ] ; then
		sudo cp /usr/bin/sunwait /usr/bin/sunwait_original
	fi
	sudo cp "${BASEDIR}/sunwait_mocked" /usr/bin/sunwait

	echo "sunwait mocked"
else
	sudo cp /usr/bin/sunwait_original /usr/bin/sunwait
	echo "sunwait democked"
fi

