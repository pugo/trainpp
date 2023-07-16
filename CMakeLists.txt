cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)

project(trainpp)

find_package(Boost 1.74 COMPONENTS program_options log REQUIRED)

set(LIB_SOURCES
        z21/z21.cpp
        z21/z21_dataset.cpp
        z21/lan_x_command_base.cpp
        z21/lan_x_command.cpp)

add_executable(trainpp ${LIB_SOURCES} main.cpp)
target_link_libraries (trainpp ${Boost_LIBRARIES} )

add_library(trainpp_lib STATIC ${LIB_SOURCES})
target_link_libraries (trainpp_lib ${Boost_LIBRARIES} )

enable_testing()
add_subdirectory(tests)
