set(COMPILER_FLAGS -Wall -Werror -pedantic -fsanitize=address --coverage)
set(LINKER_FLAGS -fsanitize=address --coverage)

add_executable(default_list_test default_list_test.cpp)

add_library(utils STATIC utils.cpp)

target_link_libraries(default_list_test PRIVATE acid_list utils gtest_main)

target_compile_options(default_list_test PRIVATE ${COMPILER_FLAGS})
target_link_options(default_list_test PRIVATE ${LINKER_FLAGS})


add_test(NAME default_list_test COMMAND default_list_test)