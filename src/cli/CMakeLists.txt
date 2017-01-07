project (ccurl-cli)
if(WIN32)
find_library(ws2_32_LIBRARY_PATH Ws2_32)
endif()
#file(GLOB SRC ccurl.c )
set (CLI_SOURCES ccurl.c)
set (DIGEST_SOURCE ccurl-digest.c)

set (CLI_NAME ccurl-cli)
set (DIGEST_NAME ccurl-digest)

#add_executable (${CLI_NAME} ${CLI_SOURCES} ../lib/PearlDiver.c ../lib/pearcldiver.c ../lib/exports.c)
#add_executable (${CLI_NAME} ${CLI_SOURCES} ${SOURCES})
add_executable (${CLI_NAME} ${CLI_SOURCES})
add_executable (${DIGEST_NAME} ${DIGEST_SOURCE})
if (WIN32)
set_target_properties (${CLI_NAME} PROPERTIES 
	LINK_FLAGS -Wl
	BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
	)
set_target_properties (${DIGEST_NAME} PROPERTIES 
	LINK_FLAGS -Wl
	BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
	)
else()
set_target_properties (${DIGEST_NAME} PROPERTIES 
	LINK_FLAGS -Wl,-lpthread
	BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
	)
set_target_properties (${CLI_NAME} PROPERTIES 
	LINK_FLAGS -Wl,-lpthread
	BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
	)
endif()
#target_link_libraries(${CLI_NAME} ${OPENCL_LIBRARIES} )
target_link_libraries(${CLI_NAME} ${LIBRARY_NAME}static ${ws2_32_LIBRARY_PATH})
target_link_libraries(${DIGEST_NAME} ${LIBRARY_NAME}static ${ws2_32_LIBRARY_PATH})

install(TARGETS ${DIGEST_NAME} 
	RUNTIME DESTINATION bin)
install(TARGETS ${CLI_NAME} 
	RUNTIME DESTINATION bin)
