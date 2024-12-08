cmake_minimum_required(VERSION 3.8)

project("playground_DOD")

include(include_external)

message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- DOD PLAYGROUND" )
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



set( USE_TEST_ OFF )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_01")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd_core} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_console")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_formula")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS GD_LOG_SIMPLE)
endif()



set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_gd")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_logger")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS GD_LOG_SIMPLE)
endif()


set( USE_TEST_ OFF )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_fake_data")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()



set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_router")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ OFF )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_Kevin")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_KevinConsole")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ OFF )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_game_worm")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ OFF )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_KevinConsole2")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS)
endif()

set( USE_TEST_ ON )
if( USE_TEST_ )
   set(TEST_NAME_ "PLAY_KevinLogger")
   add_executable(${TEST_NAME_} "main.cpp" ${external_gd} ${external_catch2} 
      "${TEST_NAME_}.cpp"
   )
   target_include_directories(${TEST_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_compile_definitions(${TEST_NAME_} PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN _CRT_SECURE_NO_WARNINGS GD_LOG_SIMPLE)
endif()
