enable_testing ()

add_executable (test_curl ${PROJECT_SOURCE_DIR}/test/test_curl.c ${SOURCES})
set_target_properties (test_curl PROPERTIES LINK_FLAGS -Wl,-lcunit)

add_executable (test_pearldiver ${PROJECT_SOURCE_DIR}/test/test_pearldiver.c ${SOURCES} ${PROJECT_SOURCE_DIR}/test/random_trits.c)
set_target_properties (test_pearldiver PROPERTIES LINK_FLAGS -Wl,-lcunit,-lpthread)

add_test (test_curl ${EXECUTABLE_OUTPUT_PATH}/test_curl)
add_test (test_pearldiver ${EXECUTABLE_OUTPUT_PATH}/test_pearldiver)
