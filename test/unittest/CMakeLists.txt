cmake_minimum_required(VERSION 3.8)

set(This UT_AllskyTest)

set(Sources
	UT_Allsky.cpp
)

add_executable(${This} ${Sources})

find_package(OpenCV REQUIRED)
if( OpenCV_FOUND )
  # Additional Include Directories
  include_directories( ${OpenCV_INCLUDE_DIRS} )

  # Additional Library Directories
  link_directories( ${OpenCV_LIB_DIR} )

  # Additional Dependencies
  #target_link_libraries(${OpenCV_LIBS} )
endif()

target_link_libraries(${This} PUBLIC
	gtest_main
	UT_Allsky
	${OpenCV_LIBS}
)

add_test(
	NAME ${This}
	COMMAND ${This}
)