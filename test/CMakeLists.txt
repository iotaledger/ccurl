enable_testing ()

#file(GLOB TESTS ${PROJECT_SOURCE_DIR}/test/*.c ${PROJECT_SOURCE_DIR}/test/acunit/test.c)
#add_executable (test_suite ${TESTS} ${SOURCES})
#set_target_properties (test_suite PROPERTIES LINK_FLAGS -Wl,-lbcunit)

#add_test (test_suite ${EXECUTABLE_OUTPUT_PATH}/test_suite)

FILE(GLOB THE_TESTS "test*.c")
FOREACH(TEST_TO_RUN ${THE_TESTS})
    get_filename_component(TEST_NAME ${TEST_TO_RUN} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" TEST_WITHOUT_EXT ${TEST_NAME})
    #MESSAGE("${TEST_TO_RUN} NAME IS ${TEST_NAME} W/O EXT IS ${TEST_WITHOUT_EXT}")

    add_executable (${TEST_WITHOUT_EXT} ${TEST_TO_RUN} ./cunit_include.c ${SOURCES})

    set_target_properties (${TEST_WITHOUT_EXT} PROPERTIES LINK_FLAGS -Wl,-lbcunit,-lpthread)
    target_link_libraries(${TEST_WITHOUT_EXT} ${OPENCL_LIBRARIES} ${CLCONTEXT_LIBRARY})

    add_test (${TEST_WITHOUT_EXT} ${EXECUTABLE_OUTPUT_PATH}/${TEST_WITHOUT_EXT})

	LIST(APPEND COMPILED_RESOURCES ${OUTPUT_FILE})
ENDFOREACH()
