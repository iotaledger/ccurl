cmake_minimum_required (VERSION 3.6)
enable_language (C)
find_package (OpenCL)
if (WIN32)
    find_library (OPENCL_LIBRARIES OpenCL)
    find_library(ws2_32_LIBRARY_PATH Ws2_32)
else()
    find_library (OPENCL_LIBRARIES OpenCL)
    find_package (Threads)
endif()
project (ccurl)

set(CMAKE_INSTALL_PREFIX "/usr")


set (LIBRARY_NAME ccurl)

#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall -g -O0")
if (WIN32)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")
else()
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall -g -O0")
endif()

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

add_library (${LIBRARY_NAME} SHARED ${SOURCES} ${COMPILED_RESOURCES})
add_library (${LIBRARY_NAME}static STATIC ${SOURCES} ${COMPILED_RESOURCES})
target_link_libraries(${LIBRARY_NAME} ${OPENCL_LIBRARIES} ${ws2_32_LIBRARY_PATH})
target_link_libraries(${LIBRARY_NAME}static ${OPENCL_LIBRARIES} ${ws2_32_LIBRARY_PATH})

if (WIN32)
    set_target_properties (${LIBRARY_NAME} PROPERTIES 
        LIBRARY_OUTPUT_DIRECTORY lib
        )
    set_target_properties(${LIBRARY_NAME}static PROPERTIES 
        LIBRARY_OUTPUT_DIRECTORY lib
        OUTPUT_NAME ${LIBRARY_NAME}
        )
else()
    set_target_properties (${LIBRARY_NAME} PROPERTIES 
        LIBRARY_OUTPUT_DIRECTORY lib
        LINK_FLAGS -lpthread
        )
    set_target_properties(${LIBRARY_NAME}static PROPERTIES 
        LIBRARY_OUTPUT_DIRECTORY lib
        OUTPUT_NAME ${LIBRARY_NAME}
        LINK_FLAGS -lpthread
        )
endif()

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
