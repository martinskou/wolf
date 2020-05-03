cmake_minimum_required(VERSION 3.0)
project(main VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


set(Boost_USE_MULTITHREADED TRUE)
find_package(Boost COMPONENTS filesystem REQUIRED)

if(CMAKE_BUILD_TYPE MATCHES Debug)
 MESSAGE("Boost information:") 
 MESSAGE("  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}") 
 MESSAGE("  Boost_LIBRARIES: ${Boost_LIBRARIES}") 
 MESSAGE("  Boost_FILESYSTEM_LIBRARY: ${Boost_FILESYSTEM_LIBRARY}") 
 MESSAGE("  Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}") 
endif()


set(SOURCE_FILES main.cpp server.cpp)
add_executable(main ${SOURCE_FILES}) 

MESSAGE("  COMPILATION UNITS: ${SOURCE_FILES}") 

target_link_libraries(main
    ${Boost_LIBRARIES}
)

