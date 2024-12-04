# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("gd" LANGUAGES CXX C)

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- GD-samples" )
message( STATUS )

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )


# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET_NAME_} PRIVATE "/bigobj")
   endif()
endfunction()

set( USE_TARGET_ ON ) # ========================================================= ${main_worm}
if( USE_TARGET_ )
   set(TARGET_NAME_ "gd_worm")

   add_executable(${TARGET_NAME_} ${external_gd}
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp
      "worm/Application.cpp"
      "worm_main.cpp"

   )
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   set_compiler_options()
endif()

set( USE_TARGET_ ON ) # ========================================================= ${main_defender}
if( USE_TARGET_ )
   set(TARGET_NAME_ "gd_defender")

   add_executable(${TARGET_NAME_} ${external_gd}
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp

      "defender_main.cpp"
      "defender/Application.cpp"
   )
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   set_compiler_options()
endif()

set( USE_TARGET_ ON ) # ========================================================= ${main_paint}
if( USE_TARGET_ )
   set(TARGET_NAME_ "gd_paint")

   add_executable(${TARGET_NAME_} ${external_gd}
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp

      "paint_main.cpp"
      "paint/Application.cpp"
   )
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   set_compiler_options()
endif()