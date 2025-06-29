cmake_minimum_required(VERSION 3.10)

project("playground_DOD")

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- FileCleaner PLAYGROUND" )
message( STATUS )

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()


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
   set(TEST_NAME_ "PLAY_YourPlayCodeExecutableName")
   add_executable(${TEST_NAME_} ${source_all} "PLAY_File1.cpp" "PLAY_File2.cpp" "PLAY_File3.cpp"  ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp" 
   )
   target_link_libraries(${TEST_NAME_} ${CMAKE_DL_LIBS})
   target_include_directories(${TEST_NAME_} PRIVATE ${directories_default_test})
   target_compile_definitions(${TEST_NAME_} PUBLIC MACRO_NAME)
endif()
```
]]

# -- playground source code
set(SOURCE_PLAYGROUND_ "main.h" "main.cpp")
set(TARGET_PLAYGROUND_ ${TARGET_FILES_})
list(FILTER TARGET_PLAYGROUND_ EXCLUDE REGEX ".*main.*")
# message("Filtered list: ${TARGET_PLAYGROUND_}")

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_convert_file")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp" ${external_gd_core} ${external_catch2} ${external_sqlite}
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()


set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_Database")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp" ${external_gd_core} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_read_file")
   add_executable(${TEST_NAME_}
      ${external_gd} 
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp" ${external_gd_core} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()


set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_test")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite} ${external_catch2}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp"  
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_pugixml")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp" ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()


set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_rowcounter")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_catch2}
      ${SOURCE_PLAYGROUND_}
   
   
      "main.cpp"  
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_dir")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite} ${external_catch2}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
      "main.cpp"  
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_ignore")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite} ${external_catch2}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
      "main.cpp"  
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_kevinstring")
   add_executable(${TEST_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite} ${external_catch2}
      ${TARGET_PLAYGROUND_}   
      ${TARGET_SOURCE_FILES_}
      ${SOURCE_PLAYGROUND_}
   
   
      "main.cpp" 
      "string.cpp"
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TEST_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_table")
   add_executable(${TEST_NAME_}
      ${external_gd} 
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp" ${external_gd_core} ${external_catch2} 
   "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_folder")
   add_executable(${TEST_NAME_}
      ${external_gd} 
      ${SOURCE_PLAYGROUND_}
   
   
   "main.cpp" ${external_gd_core} ${external_catch2} 
   "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()
