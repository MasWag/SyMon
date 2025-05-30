cmake_minimum_required(VERSION 3.10)
project(SyMon CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3 -DRELEASE -DNDEBUG")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG 1.67.0 REQUIRED COMPONENTS program_options
                                                     unit_test_framework graph)
find_package(PPL REQUIRED)

find_path(GMP_INCLUDE_DIRS NAMES gmp.h gmpxx.h)
find_library(GMP_LIBRARY NAMES gmp libgmp)
find_library(GMPXX_LIBRARY NAMES gmpxx libgmpxx)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG GMP_INCLUDE_DIRS GMP_LIBRARY)

include_directories(
  src/
  ${PROJECT_BINARY_DIR}
  ${PPL_INCLUDES}
  ${GMP_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS})

## Config for Main
add_executable(symon src/main.cc)

target_link_libraries(symon
  ${Boost_GRAPH_LIBRARY}
  ${PPL_PPL_LIBRARY}
  ${GMP_LIBRARY}
  ${GMPXX_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY})


## Config for Test
enable_testing()

add_executable(unit_test EXCLUDE_FROM_ALL
  test/unit_test.cc
  test/signature_test.cc
  test/timed_word_parser_test.cc
  test/boolean_monitor_test.cc
  test/automaton_parser_test.cc
  test/symbolic_update_test.cc
  test/non_symbolic_string_constraint_test.cc
  test/symbolic_string_constraint_test.cc
  test/io_operators_test.cc
  test/parametric_timing_constraint_helper_test.cc
  test/pta_parser_test.cc
  test/parametric_monitor_test.cc)

target_link_libraries(unit_test
  ${GMP_LIBRARY}
  ${Boost_GRAPH_LIBRARY}
  ${PPL_PPL_LIBRARY}
  ${GMPXX_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME unit_test
  COMMAND $<TARGET_FILE:unit_test>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# INSTALL
install(TARGETS symon DESTINATION bin)
