#### BUILD LIBRARY
add_library (${LIBRARY_NAME} SHARED ${SOURCES})

set_target_properties (${LIBRARY_NAME} PROPERTIES 
	LIBRARY_OUTPUT_DIRECTORY lib
	LINK_FLAGS -lpthread
	)
#target_include_directories (curl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Install library
#install(TARGETS curl DESTINATION lib/${LIBRARY_NAME})
install(TARGETS ${LIBRARY_NAME} 
	LIBRARY DESTINATION ../lib/${LIBRARY_NAME}
	ARCHIVE DESTINATION ../lib/${LIBRARY_NAME})

# Install library headers
file(GLOB HEADERS ./*.h)
install(FILES ${HEADERS} DESTINATION ../include/${LIBRARY_NAME})
