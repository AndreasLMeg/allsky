cmake_minimum_required(VERSION 3.8)

set(This Example)

project($(This) C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

# git clone https://github.com/google/googletest.git
add_subdirectory(test/googletest)

add_definitions(-DCAM_NEW -DVERSION_TAG -DVERSION_BUILD -DVERSION_BUILD_DATE)

find_package(OpenCV REQUIRED) 
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(Headers
	src/camera_newcam.h
	src/allsky.h
	src/allskyStatic.h
	src/allsky_common.h
	src/log.h
	src/modeMean.h
)
set(Sources
	src/allsky.cpp
	src/camera_newcam.cpp
	src/log.cpp
	src/modeMean.cpp
)

add_library(${This} STATIC  ${Sources} ${Headers} )

add_subdirectory(test/test)