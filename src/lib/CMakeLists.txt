INCLUDE_DIRECTORIES("./")

# include alazar SDK header files
if(MSVC AND DEFINED ENV{AlazarSDK})
  message("Linking to Alazar library defined by AlazarSDK environment variable.")
  link_directories("$ENV{AlazarSDK}/Library/x64")
endif()

if(NOT ALAZAR_INCLUDE_PATH)
    MESSAGE("ALAZAR_INCLUDE_PATH not specified. Will attempt to fetch sources at build time.")
    include(ExternalProject)
    ExternalProject_Add(ats-sdk
        GIT_REPOSITORY https://qiplab.bbn.com/buq-lab/ats-sdk-headers.git
        PREFIX ${PROJECT_SOURCE_DIR}/ats-sdk/
        UPDATE_COMMAND git pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )
    ExternalProject_Get_Property(ats-sdk SOURCE_DIR)
    set(ALAZAR_INCLUDE_PATH ${SOURCE_DIR})
else()
    message("Using ATS SDK headers from: ${ALAZAR_INCLUDE_PATH}")
    ADD_CUSTOM_TARGET( ats-sdk )
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
add_dependencies( Alazar version ats-sdk )
set_target_properties( Alazar PROPERTIES
    VERSION ${ALAZAR_VERSION_STRING}
    SOVERSION ${ALAZAR_VERSION_MAJOR}
    PREFIX "lib" # force lib prefix even on Windows
    MACOSX_RPATH 1
)

# don't let windows.h clobber std::min and std::max
if(MSVC)
    add_definitions(-DNOMINMAX)
endif()

# compiler warnings
if(MSVC)
    # on MSVC compress warnings about:
    #   "unsafe" print functions (4996)
    #   initializers (4244),
    #   size_t conversions (4267)
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "/W3 /wd4996 /wd4244 /wd4267")
else()
    ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-Wall")
endif()

if(SIM)
    # an alazar simulator is used for testing purposes
    ADD_LIBRARY( AlazarSim STATIC alazarSim.cpp )
    set_property(TARGET AlazarSim PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    add_dependencies( AlazarSim ats-sdk )
    set(ATS_LIB AlazarSim)
elseif(WIN32)
    set(ATS_LIB /c/Windows/System32/ATSApi.dll)
else()
    set(ATS_LIB ATSApi)
endif()

TARGET_LINK_LIBRARIES(Alazar
    ${ATS_LIB}
    Threads::Threads
)
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
  ${LIB_SRC}
)

TARGET_LINK_LIBRARIES(errorTest
  ${ATS_LIB}
  Threads::Threads
)
if(WIN32)
    TARGET_LINK_LIBRARIES(errorTest ws2_32)
endif()

ADD_EXECUTABLE(unittest
	./unittest.cpp
	./testBufferQ.cpp
)

TARGET_LINK_LIBRARIES(unittest
	Alazar
)

install(
    TARGETS Alazar
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES libAlazarAPI.h DESTINATION include)

if(APPLE)
    set_target_properties(Alazar PROPERTIES INSTALL_RPATH "@loader_path")
    set_target_properties(apiExample errorTest unittest PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
elseif(UNIX)
    set_target_properties(Alazar PROPERTIES INSTALL_RPATH "\$ORIGIN")
    set_target_properties(apiExample errorTest unittest PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Basic "package" target
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_PACKAGE_VERSION_MAJOR ${ALAZAR_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${ALAZAR_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${ALAZAR_VERSION_PATCH})
INCLUDE(CPack)
