#!/bin/bash

echo "ALLSKY_HOME=${ALLSKY_HOME}"
IT_ROOT=$PWD
echo "IT_ROOT=${IT_ROOT}"


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
    sudo sed -i "s#\($1\s*=\s*\).*\$#\1$2#" $3
    echo "$(date +'%T') TEST: i $3: $1=$2" >> ${ALLSKY_HOME}/mock_output
}

function set_setting(){
    file="$3"
    tmpfile="$3_tmp"
    newvalue="$2"
    key="$1"

	echo "$key old: " $(jq -r --arg searchKey "$key" '.[$searchKey]' "${file}")

    cp $file "$tmpfile" &&
    jq '.[$searchKey] |= $newval ' --arg searchKey "$key" --arg newval "$newvalue"  "$tmpfile" > "${file}" &&
    rm -f "$tmpfile"

	echo "$key new: " $(jq -r --arg searchKey "$key" '.[$searchKey]' "${file}")
    echo "$(date +'%T') TEST: i $3: $1=$2" >> ${ALLSKY_HOME}/mock_output
}

function INFO(){
    echo "$(date +'%T') TEST: $1 $2" >> ${ALLSKY_HOME}/mock_output
}

function TEST(){
    if [ "$2"  != "$3" ] ; then
	    echo "$(date +'%T') TEST: e $1: FAIL ($2 != $3)" >> ${ALLSKY_HOME}/mock_output
        ERR_CNT=$((ERR_CNT+1))
    else
	    echo "$(date +'%T') TEST: i $1: OK" >> ${ALLSKY_HOME}/mock_output
    fi
}


function SETUP(){
    IT_IMG_DIR=${ALLSKY_HOME}/IT_tmp
    rm -rf ${IT_IMG_DIR}
    mkdir -p ${IT_IMG_DIR}
    set_config IMG_DIR ${ALLSKY_HOME}/IT_tmp ${ALLSKY_HOME}/config/config.sh 

    IT_TEST_IMG=IT_image_test.jpg
    echo "${IT_ROOT}"
    cp ${IT_ROOT}/images/image_4056_3040_night.jpg ${IT_IMG_DIR}/${IT_TEST_IMG}
    ls -la ${IT_IMG_DIR}

    set_config CAMERA RPiHQ ${ALLSKY_HOME}/config/config.sh 

    IT_DEST_IMG="IT_final_image.jpg"
    set_setting "filename" "${IT_DEST_IMG}" ${ALLSKY_HOME}/config/settings_RPiHQ.json
}

function TEARDOWN(){
    rm -rf ${IT_IMG_DIR}
}
