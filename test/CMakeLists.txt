        cmake_minimum_required(VERSION 3.8)
project("TESTS_DOD")

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- DOD GENERAL TESTS" )
message( STATUS )

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   # Enable AddressSanitizer
   # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
   # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

#[[
   ======================== TEST Code executables ================================
   INFO: Add your test executables as you want when new functionality are added.
   Code here is used for testing general functionality in DOD, 
]]


# -- playground source code
set(SOURCE_TEST_ "main.h" "main.cpp")

message( STATUS "CMAKE_CONFIGURATION_TYPES = ${CMAKE_CONFIGURATION_TYPES}" )

# -- TESTS for gd_core
set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_GD")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd_core} ${external_catch2} 
      "TEST_GD.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)

   # target_compile_options(${TEST_NAME_} PRIVATE $<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT>)
   # target_link_options(${TEST_NAME_} PRIVATE $<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT>)

if (MSVC)
    target_compile_options(${TEST_NAME_} PRIVATE /RTC1 /Od /fsanitize=address )

    #    $<$<CONFIG:Debug>:/RTC1>   # Enable runtime checks
    #    $<$<CONFIG:Debug>:/Od>      # Disable optimizations (if not already set)
        # Optionally, add /Zi for complete debug info if not already set:
        # $<$<CONFIG:Debug>:/Zi>
    #)
endif()   

   if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      # target_compile_options(${TEST_NAME_} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/MD>)
      target_compile_options(${TEST_NAME_} PRIVATE -fsanitize=address -g -fno-omit-frame-pointer)
      target_link_options(${TEST_NAME_} PRIVATE -fsanitize=address)
   else()
      # target_compile_options(${TEST_NAME_} PRIVATE -fsanitize=address)
      # target_link_options(${TEST_NAME_} PRIVATE -fsanitize=address)
   endif()
endif()

