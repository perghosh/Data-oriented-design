cmake_minimum_required(VERSION 3.10)

project("tests_cleaner")

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- cleaner tests" )
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

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )

set(TARGET_TEST_ ${TARGET_FILES_})
list(FILTER TARGET_TEST_ EXCLUDE REGEX ".*main.*")

# -- playground source code
set(SOURCE_TEST_ "main.h" "main.cpp")

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_Arguments")
   add_executable(${TEST_NAME_} ${SOURCE_PLAYGROUND_} ${GD_SOURCES_ALL} ${external_catch2} 
      "${TEST_NAME_}.cpp"
      "main.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_Expression")
   add_executable(${TEST_NAME_} ${SOURCE_PLAYGROUND_} ${GD_SOURCES_ALL} ${external_pugixml} ${external_sqlite} ${external_catch2} 
      "${TARGET_SOURCE_FILES_}"
      "${TARGET_TEST_}"
      "${TEST_NAME_}.cpp"
      "main.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()


set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_File")
   add_executable(${TEST_NAME_} ${SOURCE_PLAYGROUND_} ${GD_SOURCES_ALL} ${external_pugixml} ${external_sqlite} ${external_catch2} 
      "${TARGET_SOURCE_FILES_}"
      "${TARGET_TEST_}"
      "${TEST_NAME_}.cpp"
      "main.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_File2")
   add_executable(${TEST_NAME_} ${SOURCE_PLAYGROUND_} ${GD_SOURCES_ALL} ${external_catch2} 
      "${TARGET_SOURCE_FILES_}"
      "${TARGET_TEST_}"
      "${TEST_NAME_}.cpp"
      "main.cpp"
      "History.cpp"
      "RowCount.cpp"
      "List.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_Repository")
   add_executable(${TEST_NAME_} ${SOURCE_PLAYGROUND_} ${GD_SOURCES_ALL} ${external_catch2} 
      "${TARGET_SOURCE_FILES_}"
      "${TARGET_TEST_}"
      "${TEST_NAME_}.cpp"
      "main.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "TEST_History")
   add_executable(${TEST_NAME_} ${SOURCE_PLAYGROUND_} ${GD_SOURCES_ALL} ${external_catch2} 
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp
      "${TARGET_FILES_}"
      "${TEST_NAME_}.cpp"
      "main.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()