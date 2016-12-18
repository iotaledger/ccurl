cmake_minimum_required (VERSION 3.6)
enable_language (C)
find_package (Threads)
project (ccurl)

set(CMAKE_INSTALL_PREFIX "/usr")


set (LIBRARY_NAME ccurl)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 
SET (CTEST_BUILD_TARGET "RELEASE")

#add_subdirectory (src/lib)

#### OPENCL FILES

#### LIBCCURL
file(GLOB SRC src/lib/*.c)
set (SOURCES ${SRC})

add_library (${LIBRARY_NAME} SHARED ${SOURCES})

set_target_properties (${LIBRARY_NAME} PROPERTIES 
	LIBRARY_OUTPUT_DIRECTORY lib
	LINK_FLAGS -lpthread
	)

install(TARGETS ${LIBRARY_NAME} 
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)

# Install library headers
file(GLOB HEADERS src/lib/hash/*.h)
install(FILES ${HEADERS} DESTINATION include)


### TESTING
enable_testing ()
add_subdirectory (test)
