# build using
# mkdir build
# cd build
# cmake -G "MSYS Makefiles"  ../src/  # for Windows
# cmake  ../src   # for Linux
# make

PROJECT( libAlazar )
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(CMAKE_INSTALL_PREFIX .)

SET ( CMAKE_BUILD_TYPE Debug )

#log levels: ERROR=0, WARNING=1, INFO=2, DEBUG=3, DEBUG1=4, DEBUG2=5, DEBUG3=6, DEBUG4=7
#NOTE: these need to match TLogLevel in logger.h
if( NOT LOG_LEVEL)
	set(LOG_LEVEL 2)
endif()

add_definitions(-DLOG_LEVEL=${LOG_LEVEL})

#alazar shared libraries
if(NOT ALAZAR_INCLUDE_PATH)
	get_filename_component(ALAZAR_INCLUDE_PATH ATS9870_dll_5_10_6/ ABSOLUTE)
endif()

#alazar shared libraries
if(NOT INSTALL_PATH)
	set(INSTALL_PATH dist)
endif()

add_subdirectory(src/lib)
add_subdirectory(src/matlab)
add_subdirectory(src/python)
