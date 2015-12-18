#include own library headers

INCLUDE_DIRECTORIES("./" "../../src/lib" ${ALAZAR_INCLUDE_PATH})

#Some compile flags
ADD_DEFINITIONS(${CMAKE_CXX_FLAGS} "-std=c++11 -Wall")
ADD_DEFINITIONS(${CMAKE_C_FLAGS} "-Wall")

SET_SOURCE_FILES_PROPERTIES( ${LIB_SRC} PROPERTIES LANGUAGE CXX )

SET ( TEST_SRC
	./runTests.cpp
)

ADD_EXECUTABLE(run_tests
	${TEST_SRC}
)

TARGET_LINK_LIBRARIES(run_tests
	Alazar
)

TARGET_LINK_LIBRARIES(run_tests
	Alazar
)

ADD_EXECUTABLE(errorTest
	./errorTest.cpp
)

TARGET_LINK_LIBRARIES(errorTest
	Alazar
)




install(TARGETS run_tests errorTest DESTINATION bin)