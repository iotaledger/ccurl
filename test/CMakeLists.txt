enable_testing ()

file(GLOB TESTS ${PROJECT_SOURCE_DIR}/test/*.c ${PROJECT_SOURCE_DIR}/test/acunit/test.c)
add_executable (test_suite ${TESTS} ${SOURCES})
set_target_properties (test_suite PROPERTIES LINK_FLAGS -Wl,-lbcunit)

#add_executable (test_pearldiver ${PROJECT_SOURCE_DIR}/test/test_pearldiver.c ${SOURCES} ${PROJECT_SOURCE_DIR}/test/random_trits.c)
#set_target_properties (test_pearldiver PROPERTIES LINK_FLAGS -Wl,-lbcunit,-lpthread)

add_test (test_suite ${EXECUTABLE_OUTPUT_PATH}/test_suite)
#add_test (test_pearldiver ${EXECUTABLE_OUTPUT_PATH}/test_pearldiver)
