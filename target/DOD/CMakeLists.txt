# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project("target-dod" LANGUAGES CXX C)

include(include_external)

add_subdirectory("playground") # use this to test code for later use in web server
add_subdirectory("tests") # tests to check logic in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- DOD samples" )
message( STATUS )

# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET_NAME_} PRIVATE "/bigobj")
   endif()
endfunction()

set( USE_TARGET_ ON ) # ========================================================= ${TARGET_NAME_}
if( USE_TARGET_ )
   set(TARGET_NAME_ ${PROJECT_NAME})

   add_executable(${TARGET_NAME_}
      "main.cpp"
   )
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../external)
   set_compiler_options()
endif()