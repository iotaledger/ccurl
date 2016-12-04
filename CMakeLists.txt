cmake_minimum_required (VERSION 3.6)
project (Curl)

set (SOURCES src/Curl.c)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 

#add_executable (adder adder.c ${SOURCES})

enable_testing ()

add_executable (test_curl test/test_curl.c ${SOURCES})
set_target_properties (test_curl PROPERTIES LINK_FLAGS -Wl,-lcunit)

add_executable (tests test/tests.c ${SOURCES})
#set_target_properties (tests PROPERTIES LINK_FLAGS -Wl,-lcunit)

add_test (test_curl ${EXECUTABLE_OUTPUT_PATH}/test_curl)
