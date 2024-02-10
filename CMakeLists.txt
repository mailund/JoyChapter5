cmake_minimum_required(VERSION 3.5)
project(
    JoyChapter5
    LANGUAGES C
)

include(CTest)

add_library(linked_list generic_linked_list.c)

add_executable(linked_list_test linked_list_test.c)
target_link_libraries(linked_list_test linked_list)
add_test(
    NAME linked_list_test 
    COMMAND    linked_list_test
)
