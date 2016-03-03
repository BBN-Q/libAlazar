
INCLUDE_DIRECTORIES("./" ${ALAZAR_INCLUDE_PATH}  )
MESSAGE("INCLUDE DIRS ${ALAZAR_INCLUDE_PATH}")


SET ( LIB_SRC
	./libAlazar.cpp
	./libAlazarAPI.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( Alazar SHARED ${LIB_SRC} )

#todo - alazar api library needs to be a build parameter

#this command is not supported on OSX cmake
add_custom_command(TARGET Alazar
                   PRE_BUILD
                   COMMAND sh ${CMAKE_CURRENT_LIST_DIR}/version.sh > ${CMAKE_CURRENT_LIST_DIR}/version.h
				   )

#Some compile flags
if(WIN32)
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall -pthread ")
else()
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall -pthread -fPIC")
endif()

#used by runTests.cpp for command line options
find_package(Boost 1.45.0 COMPONENTS program_options)


if(SIM)
	#an alazar simulator is used for testing purposes
	ADD_LIBRARY( AlazarSim STATIC alazarSim.cpp )

	TARGET_LINK_LIBRARIES(Alazar
		AlazarSim
    )
else()
	TARGET_LINK_LIBRARIES(Alazar
		/c/windows/system32/ATSApi.dll
    )
endif()


ADD_EXECUTABLE(runTests
	./runTests.cpp
)

TARGET_LINK_LIBRARIES(runTests
	Alazar
	${Boost_LIBRARIES}
	pthread

)

ADD_EXECUTABLE(errorTest
	./errorTest.cpp

)

TARGET_LINK_LIBRARIES(errorTest
	Alazar
)

install(TARGETS Alazar runTests errorTest DESTINATION bin)
install(FILES libAlazarAPI.h DESTINATION bin)
