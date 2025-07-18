# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project("cleaner" LANGUAGES CXX C)

include(include_external)

# add_subdirectory("playground") # use this to test code for later use in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- backup targets" )
message( STATUS )

file(GLOB TARGET_FILES_ "Application.*" "Document.*" "command/*.h" "command/*.cpp" )

# add_subdirectory("tests") # tests to check logic in web server

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )

set( USE_TARGET_ ON ) # ========================================================= ${cleaner}
if( USE_TARGET_ )

   set(TARGET_NAME_ "backup")

   add_executable(${TARGET_NAME_}
      ${GD_SOURCES_ALL}
      ${TARGET_FILES_}   
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp
      "main.cpp"
   )

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_LOG_SIMPLE)

endif()