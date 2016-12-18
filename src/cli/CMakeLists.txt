project (ccurl-cli)

file(GLOB SRC *.c )
set (CLI_SOURCES ${SRC})

set (CLI_NAME ccurl-cli)

add_executable (${CLI_NAME} ${CLI_SOURCES} ../lib/PearlDiver.c)
set_target_properties (${CLI_NAME} PROPERTIES 
	LINK_FLAGS -Wl,-lpthread
	BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
	)
#target_link_libraries(${CLI_NAME} ${OPENCL_LIBRARIES} ${CCURL_LIBRARY})
target_link_libraries(${CLI_NAME})

install(TARGETS ${CLI_NAME} 
	RUNTIME DESTINATION bin)
