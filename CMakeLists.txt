cmake_minimum_required(VERSION 3.8)

set(This UT_Allsky)

project($(This) C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_GMOCK ON)

enable_testing()

# git clone https://github.com/google/googletest.git
add_subdirectory(test/googletest)

#https://embeddedartistry.com/blog/2016/12/21/giving-your-firmware-build-a-version/
execute_process( 
	COMMAND git describe --long --dirty --always 
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
	OUTPUT_VARIABLE RAW_GIT_VERSION 
	OUTPUT_STRIP_TRAILING_WHITESPACE 
) 
string(REPLACE "-dirty" "+" BUILD_INFO ${RAW_GIT_VERSION}) 

string(TIMESTAMP BUILD_DATE "%d-%m-%Y, %H:%M" UTC ) 

cmake_host_system_information(RESULT BUILD_MACHINE QUERY HOSTNAME) 

string(COMPARE EQUAL ${BUILD_MACHINE} jenkins-server _cmp) 
if (NOT _cmp) 
	string(PREPEND BUILD_INFO dev-) 
endif() 

string(REPLACE "-" ";" GIT_INFO_LIST ${RAW_GIT_VERSION}) 
list(GET GIT_INFO_LIST 0 VERSION_NUMBER) 
message("VERSION NUMBER: ${VERSION_NUMBER}") 
add_compile_definitions(VERSION_TAG="${VERSION_NUMBER}") 
message("BUILD DATE: ${BUILD_DATE}") 
add_compile_definitions(VERSION_BUILD_DATE="${BUILD_DATE}") 
message("BUILD MACHINE: ${BUILD_MACHINE}") 
add_compile_definitions(VERSION_BUILD_MACHINE="${BUILD_MACHINE}") 
message("BUILD_INFO: ${BUILD_INFO}") 
add_compile_definitions(VERSION_BUILD="${BUILD_INFO}") 

add_definitions(-DCAM_NEW)

find_package(OpenCV REQUIRED) 
include_directories( ${OpenCV_INCLUDE_DIRS} )


include_directories("${gtest_SOURCE_DIR}/include"
                    "${gmock_SOURCE_DIR}/include")
										
set(Headers
	src/camera_newcam.h
	src/allsky.h
	src/allsky_helper.h
	src/allskyStatic.h
	src/allsky_common.h
	src/log.h
	src/modeMean.h
)
set(Sources
	src/allsky.cpp
	src/allsky_helper.cpp
	src/camera_newcam.cpp
	src/log.cpp
	src/modeMean.cpp
)

add_library(${This} STATIC  ${Sources} ${Headers} )

add_subdirectory(test/unittest)