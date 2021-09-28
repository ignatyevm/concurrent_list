set(COMPILER_FLAGS -Wall -pedantic)

add_library(utils STATIC utils.cpp)

set(tests unit_tests concurrent_tests)

foreach(test IN LISTS tests)
    add_executable(${test} "${test}.cpp")
    target_link_libraries(${test} PRIVATE acid_list gtest_main utils)
    target_compile_options(${test} PRIVATE ${COMPILER_FLAGS})
    target_link_options(${test} PRIVATE ${LINKER_FLAGS})
    add_test(NAME ${test} COMMAND ${test})
endforeach()