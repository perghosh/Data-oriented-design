# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("socket" LANGUAGES CXX C)

include(include_external)

# add_subdirectory("playground") # use this to test code for later use in web server
# add_subdirectory("tests") # tests to check logic in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- socket targets" )
message( STATUS )

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )

file(GLOB HTTP01_FILES_ *.cpp *.h ) # all cpp files in current directory


message( STATUS HTTP01_FILES_= ${HTTP01_FILES_} )

# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET_NAME_} PRIVATE "/bigobj")
   endif()
endfunction()

set( USE_TARGET_ ON ) # ========================================================= ${http01}
if( USE_TARGET_ )
   set(TARGET_NAME_ "http01")

   add_executable(${TARGET_NAME_}
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp

      ${HTTP01_FILES_}                                                         # target files

   )
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_DATABASE_SQLITE_USE _CRT_SECURE_NO_WARNINGS)
   set_compiler_options()
endif()

set( USE_TARGET_ ON ) # ========================================================= ${http_file}
if( USE_TARGET_ )
   file(GLOB HTTPF_FILES_ http_file/*.cpp http_file/*.h )                      # all cpp files in http file directory
   set(TARGET_NAME_ "httpfile")

   add_executable(${TARGET_NAME_}
      ${external_gd}
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp

      ${HTTPF_FILES_}                                                          # target files
      
      main.cpp                                                                 # main file where method main is located
   )
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TARGET_NAME_} PRIVATE TARGET_HTTP_FILES)
   set_compiler_options()
endif()