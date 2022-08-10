project( core )
cmake_minimum_required( VERSION 3.16.3 )

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set_target_properties(${TARGET_NAME}
#     PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )


file(
    GLOB SRCS
    core/**.cpp
    core/**.hpp
    main.cpp)

include_directories( ${CMAKE_SOURCE_DIR}/src/core)
# add_subdirectory( src )
# add_subdirectory( test )

# add_library( ${PROJECT_NAME} ${SRCS} )

