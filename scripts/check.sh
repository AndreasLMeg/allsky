#!/bin/bash

check_file_x ()
{
if [[ -x "$1" ]]
then
    echo "File '$1' is executable"
		file $1
else
    echo "File '$1' is not executable or found"
		file $1
		exit 1
fi
}

check_file ()
{
if [[ -e "$1" ]]
then
    echo "File '$1' is found"
		file $1
else
    echo "File '$1' is not found"
		file $1
		exit 1
fi
}

check_service_active ()
{
systemctl is-active --quiet "$1"
RC=$?
if [[ $RC = 0 ]]
then
    echo "Service '$1' is active"
else
    echo "Service '$1' is not active ($RC)"
		sudo systemctl start allsky
		exit 1
fi
}

check_service_deactive ()
{
systemctl is-active --quiet "$1"
RC=$?
if [[ $RC != 0 ]]
then
    echo "Service '$1' is deactive  ($RC)"
else
    echo "Service '$1' is active ($RC)"
		exit 1
fi
}

check_capture_SaveImageNight () 
{
echo "------------------------------------------------"
echo "check 5x capture + SaveImageNight & + sleep $1s"
date
raspistill --output "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
bash "$ALLSKY_HOME/scripts/saveImageNight.sh" &
sleep $1
raspistill --output "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
bash "$ALLSKY_HOME/scripts/saveImageNight.sh" &
sleep $1
raspistill --output "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
bash "$ALLSKY_HOME/scripts/saveImageNight.sh" &
sleep $1
raspistill --output "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
bash "$ALLSKY_HOME/scripts/saveImageNight.sh" &
sleep $1
raspistill --output "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
bash "$ALLSKY_HOME/scripts/saveImageNight.sh" &
sleep $1
date
}

##################################################
# main

echo "------------------------------------------------"
lsb_release -irdc
echo "------------------------------------------------"
df
echo "------------------------------------------------"
cat /proc/meminfo
echo "------------------------------------------------"
uptime
echo "------------------------------------------------"
top -n 1
echo "------------------------------------------------"
pinout
echo "------------------------------------------------"
git describe --long --dirty --tags

if [ -z "${ALLSKY_HOME}" ]
then
	export ALLSKY_HOME="$(realpath $(dirname "${BASH_ARGV0}"))"
fi

echo "-----------------------------------"
echo "check files"
check_file_x "${ALLSKY_HOME}/allsky.sh" 
check_file_x "${ALLSKY_HOME}/scripts/filename.sh" 
check_file "${ALLSKY_HOME}/variables.sh" 
check_file_x "${ALLSKY_HOME}/capture" 
check_file_x "${ALLSKY_HOME}/capture_RPiHQ" 
check_file_x "${ALLSKY_HOME}/keogram" 
check_file_x "${ALLSKY_HOME}/startrails" 
check_file_x "${ALLSKY_HOME}/src/sunwait" 


echo "-----------------------------------"
echo "include and check variables.sh"
source "${ALLSKY_HOME}/variables.sh"
if [ -z "${ALLSKY_CONFIG}" ]; then
	echo "${RED}*** ERROR: variables not set, can't continue!${NC}"
	exit 1
fi
check_file "${ALLSKY_CONFIG}/config.sh" 

echo "-----------------------------------"
echo "include and check config.sh"
source "${ALLSKY_CONFIG}/config.sh"
if [ -z "${ALLSKY_DEBUG_LEVEL}" ]; then
	echo "${RED}*** ERROR: old version of config.sh detected.${NC}"
	exit 1
fi

echo "-----------------------------------"
echo "check services are running"
#systemctl status allsky 
check_service_active allsky
echo "stop allsky"
sudo systemctl stop allsky
check_service_deactive allsky

echo "-----------------------------------"
echo "check image"
echo "$CAMERA_SETTINGS"
echo "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)"
ls -la "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)"
file "$ALLSKY_HOME/$(jq -r '.filename' $CAMERA_SETTINGS)"
CMD="from settings: image w=$(jq -r '.width' $CAMERA_SETTINGS):h=$(jq -r '.height'  $CAMERA_SETTINGS)"
echo "$CMD"  


echo "-----------------------------------"
echo "check sunwait"
CMD="sunwait poll angle $(jq -r '.angle' $CAMERA_SETTINGS) $(jq -r '.latitude'  $CAMERA_SETTINGS) $(jq -r '.longitude' $CAMERA_SETTINGS)"
echo "$CMD: $($CMD)"  
CMD="sunwait list angle $(jq -r '.angle' $CAMERA_SETTINGS) $(jq -r '.latitude'  $CAMERA_SETTINGS) $(jq -r '.longitude' $CAMERA_SETTINGS)"
echo "$CMD: $($CMD)"  

echo "-----------------------------------"
echo "check raspistill (1x60s)"
date
raspistill --output "$ALLSKY_HOME/test1.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 60000000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
date
#raspistill --output "$ALLSKY_HOME/test2.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 60000000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 2" -ae 32,0xff,0x808000
#date
#raspistill --output "$ALLSKY_HOME/test3.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 60000000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 3" -ae 32,0xff,0x808000
#date
echo "-----------------------------------"
echo "check raspistill (3x1s)"
date
raspistill --output "$ALLSKY_HOME/test1.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1000000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
date
raspistill --output "$ALLSKY_HOME/test2.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1000000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 2" -ae 32,0xff,0x808000
date
raspistill --output "$ALLSKY_HOME/test3.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1000000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 3" -ae 32,0xff,0x808000
date
echo "-----------------------------------"
echo "check raspistill (3x1ms)"
date
raspistill --output "$ALLSKY_HOME/test1.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
date
raspistill --output "$ALLSKY_HOME/test2.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 2" -ae 32,0xff,0x808000
date
raspistill --output "$ALLSKY_HOME/test3.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1000 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 3" -ae 32,0xff,0x808000
date
echo "-----------------------------------"
echo "check raspistill (3x1us)"
date
raspistill --output "$ALLSKY_HOME/test1.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 1" -ae 32,0xff,0x808000
date
raspistill --output "$ALLSKY_HOME/test2.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 2" -ae 32,0xff,0x808000
date
raspistill --output "$ALLSKY_HOME/test3.jpg" --thumb none --burst -st --timeout 1 --nopreview --mode 3 --exposure off --shutter 1 --analoggain 1 --exif IFD0.Artist=li_60000000_3 --awb off --awbgains 2.5,1.5 --hflip --vflip --saturation 0 --brightness 50 --quality 100  -a 1036 -a "Test 3" -ae 32,0xff,0x808000
date
echo "-----------------------------------"
echo "check SaveImageDay"
date
bash -x "$ALLSKY_HOME/scripts/saveImageDay.sh"
date
echo "-----------------------------------"
echo "check SaveImageNight"
date
bash -x "$ALLSKY_HOME/scripts/saveImageNight.sh"
date
echo "-----------------------------------"
check_capture_SaveImageNight 20 
check_capture_SaveImageNight 10 
check_capture_SaveImageNight 5
check_capture_SaveImageNight 2
check_capture_SaveImageNight 1 
check_capture_SaveImageNight 0.5 
check_capture_SaveImageNight 0.2 
#check_capture_SaveImageNight 0.1 #1st upload problems
#check_capture_SaveImageNight 0.05 
#check_capture_SaveImageNight 0.02 
#check_capture_SaveImageNight 0.01 

echo "-----------------------------------"
echo "start allsky"
sudo systemctl start allsky
check_service_active allsky

echo "check finished"
