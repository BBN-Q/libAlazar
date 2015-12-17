
INCLUDE_DIRECTORIES("./" ${ALAZAR_INLCUDE_PATH}  )

#Some compile flags
ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall")

SET ( LIB_SRC
	./libAlazar.cpp
	./libAlazarAPI.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( Alazar SHARED ${LIB_SRC} )

#todo - alazar api library needs to be a build parameter

if(SIM)
	#an alazar simulator is used on osx for testing purposes
	TARGET_LINK_LIBRARIES(Alazar
		pthread
		AlazarSim
	)
else()
	TARGET_LINK_LIBRARIES(Alazar
		pthread
		#todo - handle dll location in windows
		Alazar
	)

endif()


install(TARGETS Alazar DESTINATION lib)
install(FILES libAlazarAPI.h DESTINATION lib)