#!/bin/bash

# Script to save a DAY or NIGHT image.

./mock/mock_variables.sh mock
CONFIG="${ALLSKY_HOME}/mock_variables"

# allskyTest
source "allskyTest/allskyTest.sh"

./mock/mock_sunwait.sh mock
./mock/mock_endOfNight.sh mock
./mock/mock_saveImage.sh mock

set_config ALLSKY_MOCK_SUNWAIT NIGHT $CONFIG
start
sleep 200
seperation
set_config ALLSKY_MOCK_SUNWAIT DAY $CONFIG
sleep 200
seperation
set_config ALLSKY_MOCK_SUNWAIT NIGHT $CONFIG
sleep 200
end

./mock/mock_sunwait.sh demock 
./mock/mock_endOfNight.sh demock
./mock/mock_saveImage.sh demock
./mock/mock_variables.sh demock
