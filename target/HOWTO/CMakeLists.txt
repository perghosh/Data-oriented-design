# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project("HOWTO" LANGUAGES CXX C)

include(include_external)



message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- HOWTO code, how to use other parts" )
message( STATUS )

# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET_NAME_} PRIVATE "/bigobj")
   endif()
endfunction()

set( USE_TARGET_ ON ) # ========================================================= arguments
if( USE_TARGET_ )
   set(HOWTO_NAME_ "HOWTO_arguments")
   add_executable(${HOWTO_NAME_} "main.cpp" ${GD_SOURCES_ALL} 
      "${HOWTO_NAME_}.cpp"
   )
   target_include_directories(${HOWTO_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${HOWTO_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   set_compiler_options()
endif()