cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- http PLAYGROUND" )
message( STATUS )


set(SOURCE_PLAYGROUND_ "main.h" "main.cpp")
set(TARGET_PLAYGROUND_ ${TARGET_FILES_})                                      # TARGET_FILES_ is from ../CMakeLists.txt
list(FILTER TARGET_PLAYGROUND_ EXCLUDE REGEX ".*main.*")                      # Exclude any file with 'main' in its name

message(STATUS "-- TARGET_PLAYGROUND_ = ${TARGET_PLAYGROUND_}")

# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TEST_NAME_} PRIVATE "/bigobj")
      target_compile_definitions(${TEST_NAME_} PRIVATE _WIN32_WINNT=0x0601)
   endif()
endfunction()


set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_uri")
   add_executable(${TEST_NAME_}
      ${GD_SOURCES_ALL}
#      ${TARGET_SOURCE_FILES_}
#      ${TARGET_PLAYGROUND_}   
      ${SOURCE_PLAYGROUND_}
      ${external_sqlite}
      ${external_catch2} 
      "main.cpp" 
      "${TEST_NAME_}.cpp"
   )
   set_compiler_options()
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_LOG_SIMPLE)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE )
endif()
