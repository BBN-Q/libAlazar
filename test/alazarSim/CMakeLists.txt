#include own library headers


INCLUDE_DIRECTORIES("./" ${ALAZAR_INLCUDE_PATH} "../../src/lib")

#Some compile flags
ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall")

SET ( LIB_SRC
	./alazarSim.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( AlazarSim SHARED ${LIB_SRC} )

install(TARGETS AlazarSim DESTINATION ".")