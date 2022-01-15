#!/bin/bash

# Script to save a DAY or NIGHT image.

./mock_variables.sh mock
CONFIG="${ALLSKY_HOME}/mock_variables"

function start(){
    echo "$(date +'%T') ======================================================================" > ${ALLSKY_HOME}/mock_output
    echo "$(date +'%T') MOCK: s $0: $@" >> ${ALLSKY_HOME}/mock_output
}

function seperation(){
    echo "$(date +'%T') ----------------------------------------------------------------------" >> ${ALLSKY_HOME}/mock_output
}

function end(){
    echo "$(date +'%T') ======================================================================" >> ${ALLSKY_HOME}/mock_output
    cat ${ALLSKY_HOME}/mock_output
}

function set_config(){
    sudo sed -i "s/\($1\s*=\s*\).*\$/\1$2/" $CONFIG
    echo "$(date +'%T') MOCK: i $CONFIG: $1=$2" >> ${ALLSKY_HOME}/mock_output
}


./mock_sunwait.sh mock
./mock_endOfNight.sh mock
./mock_saveImage.sh mock

set_config ALLSKY_MOCK_SUNWAIT NIGHT # sunwait response NIGHT
start
sleep 200
seperation
set_config ALLSKY_MOCK_SUNWAIT DAY # sunwait response DAY
sleep 200
seperation
set_config ALLSKY_MOCK_SUNWAIT NIGHT # sunwait response NIGHT
sleep 200
end

./mock_sunwait.sh demock 
./mock_endOfNight.sh demock
./mock_saveImage.sh demock
./mock_variables.sh demock
