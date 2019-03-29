# build using
# mkdir build
# cd build
# cmake -G "MSYS Makefiles"  ../src/  # for Windows
# cmake  ../src   # for Linux
# make

PROJECT( libAlazar )
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

# build the library version string from git
execute_process(
  COMMAND git describe --dirty
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
string( REGEX MATCH "^v([0-9]+)\.([0-9]+)" VERSION_STRING ${GIT_DESCRIBE} )

if (NOT VERSION_STRING)
  message(WARNING "Failed to extract version string from 'git describe'")
endif()
set(ALAZAR_VERSION_MAJOR ${CMAKE_MATCH_1})
set(ALAZAR_VERSION_MINOR ${CMAKE_MATCH_2})
set(ALAZAR_VERSION_PATCH 0)
set(ALAZAR_VERSION_STRING ${ALAZAR_VERSION_MAJOR}.${ALAZAR_VERSION_MINOR}.${ALAZAR_VERSION_PATCH})

#log levels: ERROR=0, WARNING=1, INFO=2, DEBUG=3, DEBUG1=4, DEBUG2=5, DEBUG3=6, DEBUG4=7
#NOTE: these need to match TLogLevel in logger.h
if( NOT LOG_LEVEL)
	set(LOG_LEVEL 2)
endif()

add_definitions(-DLOG_LEVEL=${LOG_LEVEL})

add_subdirectory(src/lib)
add_subdirectory(src/matlab)
add_subdirectory(src/python)
