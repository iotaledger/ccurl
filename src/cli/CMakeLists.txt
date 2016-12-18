find_library (CCURL_LIBRARY ccurl)
project (ccurl-cli)

file(GLOB SRC *.c )
set (SOURCES ${SRC})

set (CLI_NAME ccurl-cli)

add_executable (${CLI_NAME} ${SOURCES})
set_target_properties (${CLI_NAME} PROPERTIES 
	LINK_FLAGS -Wl,-lccurl
	BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
	)
#target_link_libraries(${CLI_NAME} ${OPENCL_LIBRARIES} ${CCURL_LIBRARY})
target_link_libraries(${CLI_NAME} ${CCURL_LIBRARY})

install(TARGETS ${CLI_NAME} 
	RUNTIME DESTINATION bin)
