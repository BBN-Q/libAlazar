# build using
# mkdir build
# cd build
# cmake -G "MSYS Makefiles"  ../src/  # for Windows
# cmake  ../src   # for Linux
# make

PROJECT( libAlazar )
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build)

SET ( CMAKE_BUILD_TYPE Debug )
SET (CMAKE_C_COMPILER "/usr/bin/gcc")

#alazar shared libraries
if(WIN32)
	get_filename_component(ALAZAR_INCLUDE_PATH ../AlazarTech/ATS-SDK/6.0.3/Samples/Include ABSOLUTE)
else()
    get_filename_component(ALAZAR_INLCUDE_PATH test/alazarSim ABSOLUTE)
endif()

add_subdirectory(test/alazarSim)
add_subdirectory(src/lib)
add_subdirectory(test/unitTest)




