project(tests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

add_executable(gtests_run
                    z21_dataset_test.cpp
                    lan_x_command_test.cpp)

target_link_libraries(gtests_run gtest gtest_main gmock trainpp_lib)


include(GoogleTest)
gtest_discover_tests(gtests_run PROPERTIES LABELS "unit" DISCOVERY_TIMEOUT 240)
