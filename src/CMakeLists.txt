cmake_minimum_required(VERSION 3.0)
project(main VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

find_package (SQLite3)

set(Boost_USE_MULTITHREADED TRUE)
find_package(Boost COMPONENTS filesystem REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
endif ()


file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(main ${SOURCE_FILES})

MESSAGE("  COMPILATION UNITS: ${SOURCE_FILES}") 

target_link_libraries(main
    ${Boost_LIBRARIES}
        ${SQLite3_LIBRARIES}
        )

