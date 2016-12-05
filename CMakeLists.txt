cmake_minimum_required (VERSION 3.6)
enable_language (C)
find_package (Threads)
project (Curl)

set (SOURCES src/Curl.c src/PearlDiver.c)

#set (CC /usr/bin/clang)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 

#add_executable (adder adder.c ${SOURCES}) 

enable_testing ()

add_executable (test_curl test/test_curl.c ${SOURCES})
set_target_properties (test_curl PROPERTIES LINK_FLAGS -Wl,-lcunit)

add_executable (test_pearldiver test/test_pearldiver.c ${SOURCES} test/random_trits.c)
set_target_properties (test_pearldiver PROPERTIES LINK_FLAGS -Wl,-lcunit,-lpthread)

add_test (test_curl ${EXECUTABLE_OUTPUT_PATH}/test_curl)
add_test (test_pearldiver ${EXECUTABLE_OUTPUT_PATH}/test_pearldiver)
