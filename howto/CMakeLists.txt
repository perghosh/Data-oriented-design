        cmake_minimum_required(VERSION 3.8)
project("TESTS_DOD")

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- DOD HOWTO - tutorials" )
message( STATUS )

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   # Enable AddressSanitizer
   # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
   # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

#[[
   ======================= HOWTO Code executables =============================
   INFO: Add your test executables as you want when new functionality are added.
   Code here is used for testing general functionality in DOD, 
]]


# -- playground source code
set(SOURCE_TEST_ "main.h" "main.cpp")

message( STATUS "CMAKE_CONFIGURATION_TYPES = ${CMAKE_CONFIGURATION_TYPES}" )

# -- TESTS for gd_core
set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "HOWTO_gd_arguments")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd_core} ${external_catch2} 
      "gd/HOWTO_arguments.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "HOWTO_gd_table")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd_core} ${external_catch2} 
      "gd/HOWTO_table.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()
