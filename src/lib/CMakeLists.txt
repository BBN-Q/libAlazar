
INCLUDE_DIRECTORIES("./" ${ALAZAR_INCLUDE_PATH}  )
MESSAGE("INCLUDE DIRS ${ALAZAR_INCLUDE_PATH}")


SET ( LIB_SRC
	./libAlazar.cpp
	./libAlazarAPI.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( Alazar SHARED ${LIB_SRC} )

#todo - alazar api library needs to be a build parameter

add_custom_command(TARGET Alazar
                   PRE_BUILD 
                   COMMAND sh ${CMAKE_CURRENT_LIST_DIR}/version.sh > ${CMAKE_CURRENT_LIST_DIR}/version.h
				   ) 

#Some compile flags
ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall")


if(SIM)
	#an alazar simulator is used for testing purposes
	TARGET_LINK_LIBRARIES(Alazar
		AlazarSim
)
else()
	TARGET_LINK_LIBRARIES(Alazar
		/c/windows/system32/ATSApi.dll		
)
endif()


install(TARGETS Alazar DESTINATION bin)
install(FILES libAlazarAPI.h DESTINATION bin)