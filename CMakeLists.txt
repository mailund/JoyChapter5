cmake_minimum_required(VERSION 3.5)
project(
    JoyChapter5
    LANGUAGES C
)

include(CTest)

add_executable(generated_list_test generated_list_test.c)
add_test(
    NAME    generated_list_test 
    COMMAND generated_list_test
)

add_executable(generated_hash_test generated_hash_test.c)
add_test(
    NAME    generated_hash_test 
    COMMAND generated_hash_test 19
)
