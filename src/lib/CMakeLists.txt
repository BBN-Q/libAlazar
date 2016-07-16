
INCLUDE_DIRECTORIES("./" ${ALAZAR_INCLUDE_PATH} "../../deps/Catch/include"  )
MESSAGE("INCLUDE DIRS ${ALAZAR_INCLUDE_PATH}")

add_custom_target (
    version ALL
    COMMAND cd ${CMAKE_CURRENT_LIST_DIR} && sh ${CMAKE_CURRENT_LIST_DIR}/version.sh > ${CMAKE_CURRENT_LIST_DIR}/version.h
 )

SET ( LIB_SRC
	./libAlazar.cpp
	./libAlazarAPI.cpp
)

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

ADD_LIBRARY( Alazar SHARED ${LIB_SRC} )
add_dependencies( Alazar version )

#todo - alazar api library needs to be a build parameter

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#Some compile flags
if(WIN32)
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall -O3")
else()
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall")
endif()

#used by runTests.cpp for command line options
find_package(Boost 1.45.0 COMPONENTS program_options)

if(SIM)
	#an alazar simulator is used for testing purposes
	ADD_LIBRARY( AlazarSim STATIC alazarSim.cpp )
  set_property(TARGET AlazarSim PROPERTY POSITION_INDEPENDENT_CODE TRUE)
	TARGET_LINK_LIBRARIES(Alazar
		AlazarSim
    Threads::Threads
    )
else()
	TARGET_LINK_LIBRARIES(Alazar
		/c/windows/system32/ATSApi.dll
    )
endif()

ADD_EXECUTABLE(apiExample
	./apiExample.cpp
)

TARGET_LINK_LIBRARIES(apiExample
	Alazar
	${Boost_LIBRARIES}
	Threads::Threads

)

ADD_EXECUTABLE(errorTest
	./errorTest.cpp

)

TARGET_LINK_LIBRARIES(errorTest
	Alazar
)

ADD_EXECUTABLE(unittest
	./unittest.cpp
	./testBufferQ.cpp

)

TARGET_LINK_LIBRARIES(unittest
	Alazar
)

install(TARGETS Alazar apiExample errorTest unittest DESTINATION bin)
install(TARGETS Alazar DESTINATION ${INSTALL_PATH})
install(FILES libAlazarAPI.h DESTINATION ${INSTALL_PATH})
install(FILES libAlazarAPI.h DESTINATION bin)
