cmake_minimum_required (VERSION 3.6)
enable_language (C)
find_package (Threads)
project (iotaclminer)

file(GLOB SRC src/*.c)
set (SOURCES ${SRC})

#set (SOURCES ${PROJECT_SOURCE_DIR}/src/clspammer.c ${PROJECT_SOURCE_DIR}/src/PearlDiver.c)
set (LIBRARY_NAME ccurl)

#set (CC /usr/bin/clang)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 
SET (CTEST_BUILD_TARGET "DEVELOPMENT")

add_subdirectory (test)
add_subdirectory (src)

enable_testing ()

#### BUILD LIBRARY
# http://stackoverflow.com/questions/8096015/creating-a-simple-makefile-to-build-a-shared-library
# http://www.thegeekstuff.com/2012/06/linux-shared-libraries
