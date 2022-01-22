#!/bin/bash

echo "ALLSKY_HOME=${ALLSKY_HOME}"

function start(){
    echo "$(date +'%T') ======================================================================" > ${ALLSKY_HOME}/mock_output
    echo "$(date +'%T') TEST: s $0: $@" >> ${ALLSKY_HOME}/mock_output
    ERR_CNT=0
}

function seperation(){
    echo "$(date +'%T') ----------------------------------------------------------------------" >> ${ALLSKY_HOME}/mock_output
}

function end(){
    echo "$(date +'%T') TEST: i ERR_CNT=$ERR_CNT" >> ${ALLSKY_HOME}/mock_output
    echo "$(date +'%T') ======================================================================" >> ${ALLSKY_HOME}/mock_output
    cat ${ALLSKY_HOME}/mock_output
}

function set_config(){
    sudo sed -i "s/\($1\s*=\s*\).*\$/\1$2/" $3
    echo "$(date +'%T') TEST: i $3: $1=$2" >> ${ALLSKY_HOME}/mock_output
}

function INFO(){
    echo "$(date +'%T') TEST: $1 $2" >> ${ALLSKY_HOME}/mock_output
}

function TEST(){
    if [ "$2"  != "$3" ] ; then
	    echo "$(date +'%T') TEST: e $1 FAIL ($2 != $3)" >> ${ALLSKY_HOME}/mock_output
        ERR_CNT=$((ERR_CNT+1))
    else
	    echo "$(date +'%T') TEST: i $1 OK" >> ${ALLSKY_HOME}/mock_output
    fi
}
