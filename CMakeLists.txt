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

# log levels are: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
if( NOT FILE_LOG_LEVEL)
	set(FILE_LOG_LEVEL 4)
endif()
if( NOT CONSOLE_LOG_LEVEL)
	set(CONSOLE_LOG_LEVEL 3)
endif()


include_directories(deps/plog/include)

add_definitions(-DFILE_LOG_LEVEL=${FILE_LOG_LEVEL})
add_definitions(-DCONSOLE_LOG_LEVEL=${CONSOLE_LOG_LEVEL})


add_subdirectory(src/lib)
add_subdirectory(src/matlab)
add_subdirectory(src/python)
