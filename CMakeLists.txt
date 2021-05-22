cmake_minimum_required(VERSION 3.17)
project(ScheduleGA)

set(CMAKE_CXX_STANDARD 17)


include(cmake/Conan.cmake)
run_conan()

# Automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_executable(ScheduleGA main.cpp "ScheduleCommon.h" "ScheduleCommon.cpp" "utils.h" "LinearAllocator.h" "LinearAllocator.cpp" "ScheduleIndividual.h" "ScheduleIndividual.cpp" "ScheduleGA.h" "ScheduleGA.cpp")
add_library(LibScheduleGA STATIC main.cpp "ScheduleCommon.h" "ScheduleCommon.cpp" "utils.h" "LinearAllocator.h" "LinearAllocator.cpp" "ScheduleIndividual.h" "ScheduleIndividual.cpp" "ScheduleGA.h" "ScheduleGA.cpp")

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)

add_executable(Catch_test_ScheduleGA test_ScheduleGA.cpp)
target_link_libraries(Catch_test_ScheduleGA PRIVATE catch_main LibScheduleGA)


catch_discover_tests(
  Catch_test_ScheduleGA
  TEST_PREFIX
  "unittests."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)
