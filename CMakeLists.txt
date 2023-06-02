cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)

project(trainpp)

find_package(Boost 1.74 COMPONENTS program_options log REQUIRED)


add_executable(trainpp main.cpp z21.cpp z21_dataset.cpp)

target_link_libraries (trainpp ${Boost_LIBRARIES} )
