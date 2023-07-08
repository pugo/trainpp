project(tests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

add_executable(Google_Tests_run
                    z21_dataset_test.cpp)

target_link_libraries(Google_Tests_run gtest gtest_main gmock trainpp_lib)
