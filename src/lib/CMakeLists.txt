#include own library headers


INCLUDE_DIRECTORIES("./" ${ALAZAR_INLCUDE_PATH}  )

#Some compile flags
ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall ")
ADD_DEFINITIONS(${CMAKE_C_FLAGS} "-Wall ")

SET ( LIB_SRC
	./libAlazar.cpp
	./libAlazarAPI.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( Alazar SHARED ${LIB_SRC} )

#todo - alazar api library needs to be a build parameter
TARGET_LINK_LIBRARIES(Alazar
	pthread
	AlazarSim
)

install(TARGETS Alazar DESTINATION ".")