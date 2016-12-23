cmake_minimum_required (VERSION 3.6)
enable_language (C)
find_package (Threads)
find_library (OPENCL_LIBRARIES OpenCL)
project (ccurl)

set(CMAKE_INSTALL_PREFIX "/usr")


set (LIBRARY_NAME ccurl)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall -g -O0 ")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -g -O0 -Wno-unused-function")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 
SET (CTEST_BUILD_TARGET "RELEASE")


#### OPENCL FILES

SET(RESOURCE_COMPILER xxd)
FILE(GLOB COMPILED_RESOURCES "src/lib/cl/*")
FOREACH(INPUT_FILE ${COMPILED_RESOURCES})
	get_filename_component(INPUT_NAME ${INPUT_FILE} NAME)
	get_filename_component(INPUT_PATH ${INPUT_FILE} DIRECTORY)
	SET(OUTPUT_FILE ${INPUT_PATH}/../${INPUT_NAME}.h)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${OUTPUT_FILE}
		COMMAND ${RESOURCE_COMPILER} -i ${INPUT_NAME} ${OUTPUT_FILE}
		WORKING_DIRECTORY ${INPUT_PATH}
		)
	LIST(APPEND COMPILED_RESOURCES ${OUTPUT_FILE})
ENDFOREACH()

#### LIBCCURL
file(GLOB SRC src/lib/*.c src/lib/claccess/*.c src/lib/util/*.c)
set (SOURCES ${SRC})

#add_library (${LIBRARY_NAME} SHARED ${SOURCES})
add_library (${LIBRARY_NAME} SHARED ${SOURCES} ${COMPILED_RESOURCES})
target_link_libraries(${LIBRARY_NAME} ${OPENCL_LIBRARIES})

set_target_properties (${LIBRARY_NAME} PROPERTIES 
	LIBRARY_OUTPUT_DIRECTORY lib
	LINK_FLAGS -lpthread
	)

install(TARGETS ${LIBRARY_NAME} 
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)

# Install library headers
file(GLOB HEADERS src/lib/*.h)
install(FILES ${HEADERS} DESTINATION include)

### TESTING
#enable_testing ()
#add_subdirectory (test)
add_subdirectory (src/cli)
