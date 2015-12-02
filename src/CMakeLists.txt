# build using
# mkdir build
# cd build
# cmake -G "MSYS Makefiles"  ../src/  # for Windows
# cmake  ../src   # for Linux
# make


PROJECT( libx6 )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CMAKE_BUILD_TYPE Debug )
SET (CMAKE_C_COMPILER "/usr/bin/gcc")

#include own library headers
INCLUDE_DIRECTORIES("./lib/")

#Catch unit-testing
INCLUDE_DIRECTORIES("../deps/Catch/include")

#alazar shared libraries
#if(WIN32)
#	SET(ALAZAR_LIB_PATH ${ALAZAR_LIB_PATH}/tbd)
#else()
#	SET(ALAZAR_LIB_PATH ${ALAZAR_LIB_PATH}/tbd)
#endif()

#LINK_DIRECTORIES(${ALAZAR_LIB_PATH}/tbd)

#Some compile flags
ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++0x -Wall ")
ADD_DEFINITIONS(${CMAKE_C_FLAGS} "-Wall ")

SET ( LIB_SRC
	./lib/libAlazar.cpp
	./lib/libAlazarAPI.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( Alazar SHARED ${LIB_SRC} )


set_source_files_properties(
	../test/runTests.c
	PROPERTIES 
	COMPILE_FLAGS 
	"-Wall")

SET ( TEST_SRC
	../test/runTests.cpp
)

ADD_EXECUTABLE(run_tests
	${TEST_SRC}
)

TARGET_LINK_LIBRARIES(run_tests
	Alazar
)

TARGET_LINK_LIBRARIES(Alazar
	pthread
)
