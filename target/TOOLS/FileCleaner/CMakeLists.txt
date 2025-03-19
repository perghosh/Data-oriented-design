# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("file" LANGUAGES CXX C)

include(include_external)

# add_subdirectory("playground") # use this to test code for later use in web server
# add_subdirectory("tests") # tests to check logic in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- file targets" )
message( STATUS )

set(TARGET_NAME_ "cleaner")

add_executable(${TARGET_NAME_}
   main.cpp                                                           # main file where method main is located
 "main.h")