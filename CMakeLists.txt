cmake_minimum_required(VERSION 3.17)
project(ScheduleGA)

set(CMAKE_CXX_STANDARD 20)


include(cmake/Conan.cmake)
run_conan()

# Automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

set(SRC_FILE "main.cpp"
			"ScheduleCommon.h"
			"ScheduleCommon.cpp"
			"utils.h"
			"LinearAllocator.h"
			"LinearAllocator.cpp"
			"ScheduleIndividual.h"
			"ScheduleIndividual.cpp"
			"ScheduleGA.h"
			"ScheduleGA.cpp"
			"ScheduleChromosomes.h"
			"ScheduleChromosomes.cpp")

add_executable(ScheduleGA ${SRC_FILE})
target_compile_features(ScheduleGA PUBLIC cxx_std_20)

add_library(LibScheduleGA STATIC ${SRC_FILE})
target_compile_features(LibScheduleGA PUBLIC cxx_std_20)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)

add_executable(Catch_test_ScheduleGA test_ScheduleGA.cpp)
target_link_libraries(Catch_test_ScheduleGA PRIVATE catch_main LibScheduleGA)
target_compile_features(Catch_test_ScheduleGA PUBLIC cxx_std_20)


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
