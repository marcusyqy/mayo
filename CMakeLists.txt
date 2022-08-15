cmake_policy(SET CMP0048 NEW)
project( zoo VERSION 0.0.0)

cmake_minimum_required( VERSION 3.16.3 )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_target_properties(${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

file( GLOB SRCS
    zoo/**.cpp
    zoo/**.hpp
    zoo/*/**.cpp
    zoo/*/**.hpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/zoo
    ${CMAKE_SOURCE_DIR}/deps/glfw/include
    ${CMAKE_SOURCE_DIR}/deps/spdlog/include
    )

add_subdirectory(deps/glfw)
add_executable( ${PROJECT_NAME} ${SRCS} )

target_link_libraries( ${PROJECT_NAME}
    glfw
    )

target_compile_definitions(${PROJECT_NAME} PUBLIC ZOO_ENABLE_LOGS)
# ====== TESTING ======
include(CTest)
enable_testing()
add_subdirectory(test)
