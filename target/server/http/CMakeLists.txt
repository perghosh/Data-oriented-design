# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project("HTTP" LANGUAGES CXX C)

include(include_external)

# add_subdirectory("playground") # use this to test code for later use in web server
# add_subdirectory("tests") # tests to check logic in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- HTTP Server" )
message( STATUS )

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )

file(GLOB TARGET_FILES_ "*.cpp" )                                             # all cpp files in root directory
set( TARGET_SOURCE_FILES_
   ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp
   ${APPLICATION_DIRECTORY_}/database/Metadata_Statements.cpp
)

add_subdirectory("playground") # playground http logic
# add_subdirectory("tests") # tests cleaner logic



# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET_NAME_} PRIVATE "/bigobj")
      target_compile_definitions(${TARGET_NAME_} PRIVATE _WIN32_WINNT=0x0601)
   endif()
endfunction()

set( USE_TARGET_ ON ) # ========================================================= ${http}
if( USE_TARGET_ )
   file(GLOB HTTP_FILES_ *.cpp )                                              # all cpp files in root directory
   file(GLOB HTTP_STATE_FILES_  state/*.cpp )                                 # all cpp files in state subdirectory
   set(TARGET_NAME_ "http")

   add_executable(${TARGET_NAME_}
      ${external_sqlite}
      ${external_pugixml}
      ${GD_SOURCES_ALL}
      ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp

     ${HTTP_FILES_}                                                           # current directory files
     ${HTTP_STATE_FILES_}                                                     # files from state subdirectory
      "main.cpp"                                                              # main file where method main is located
   )
   
   target_compile_features(${TARGET_NAME_} PRIVATE cxx_std_23)

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_LOG_SIMPLE)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_DATABASE_SQLITE_USE GD_DATABASE_ODBC_USE)
   set_compiler_options()
   if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_link_libraries(${TARGET_NAME_} ${CMAKE_DL_LIBS})
      target_link_libraries(${TARGET_NAME_} ${CMAKE_SOURCE_DIR}/external/mariadb/libmaodbc.so)
   elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
   else()
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601 ") # GCC needs this to compile boost
      target_link_libraries(${TARGET_NAME_} PRIVATE odbc32.lib)
   endif()
endif()