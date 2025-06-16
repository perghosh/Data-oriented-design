cmake_minimum_required(VERSION 3.10)

project("tests_DOD")

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- DOD TESTS" )
message( STATUS )


#[[
======================== PLAY Code executables ================================
INFO: Add your test executables as you want when new functionality are added.
Code here is not that important, do not depend on that the code you write in
playground will be kept. It could be deleted by any in the team.

Playcode is just for development, to improve the quality of code added later to
production. To simplify working with new or modified code because it sometimes
is hard to work in production code doing new stuff.

TEMPLATE: Sample template to add executable where you can do development.
```
set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_YourPlayCodeExecutableName")
   add_executable(${TEST_NAME_} ${source_all} "TEST_File1.cpp" "TEST_File2.cpp" "TEST_File3.cpp"  ${all_in_external})
   target_link_libraries(${TEST_NAME_} ${CMAKE_DL_LIBS})
   target_include_directories(${TEST_NAME_} PRIVATE ${directories_default_test})
   target_compile_definitions(${TEST_NAME_} PUBLIC MACRO_NAME)
endif()
```
]]
