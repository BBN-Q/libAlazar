
INCLUDE_DIRECTORIES("./")

# include alazar SDK header files
if(NOT ALAZAR_INCLUDE_PATH)
    MESSAGE("ALAZAR_INCLUDE_PATH not specified. Will attempt to fetch sources at build time.")
    include(ExternalProject)
    ExternalProject_Add(ats-sdk
        GIT_REPOSITORY git@qiplab.bbn.com:buq-lab/ats-sdk-headers.git
        PREFIX ${PROJECT_SOURCE_DIR}/ats-sdk/
        UPDATE_COMMAND git pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )
    ExternalProject_Get_Property(ats-sdk SOURCE_DIR)
    set(ALAZAR_INCLUDE_PATH ${SOURCE_DIR})
else()
    ADD_CUSTOM_TARGET( ats-sdk
        COMMAND echo "Using system ATS SDK at: " ${ALAZAR_INCLUDE_PATH}
    )
endif()
INCLUDE_DIRECTORIES(${ALAZAR_INCLUDE_PATH})
INCLUDE_DIRECTORIES("../../deps/Catch/include")
INCLUDE_DIRECTORIES("../../deps/readerwriterqueue")

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#find threading library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#we need C++11 support
set(CMAKE_CXX_STANDARD 11)

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
add_dependencies( Alazar version ats-sdk)

#todo - alazar api library needs to be a build parameter

#Some compile flags
if(WIN32)
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-Wall -O3")
else()
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-Wall")
endif()

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
        ATSApi
        Threads::Threads
    )
endif()

if(WIN32)
    TARGET_LINK_LIBRARIES(Alazar ws2_32)
endif()

ADD_EXECUTABLE(apiExample
	./apiExample.cpp
)

TARGET_LINK_LIBRARIES(apiExample
	Alazar
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
