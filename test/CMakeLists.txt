include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
#
enable_testing()
# include_directories( ${CMAKE_SOURCE_DIR}/src/core)
#
# add_executable(
#     hello_test
#     hello_test.cpp
#     expected_test.cpp
#     fixed_string_test.cpp
# )
#
# target_link_libraries(
#     hello_test
#     GTest::gtest_main
# )
#
include(GoogleTest)
# gtest_discover_tests(hello_test)
#
add_subdirectory( core )
