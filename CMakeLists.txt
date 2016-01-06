# build using
# mkdir build
# cd build
# cmake -G "MSYS Makefiles"  ../src/  # for Windows
# cmake  ../src   # for Linux
# make

PROJECT( libAlazar )
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_INSTALL_PREFIX .)

SET ( CMAKE_BUILD_TYPE Debug )

#alazar shared libraries
if(NOT ALAZAR_INCLUDE_PATH)
	get_filename_component(ALAZAR_INCLUDE_PATH ATS9870_dll_5_10_6/ ABSOLUTE)
endif()

if(SIM)
	add_subdirectory(test/alazarSim)
endif()
add_subdirectory(src/lib)
add_subdirectory(test/unitTest)




