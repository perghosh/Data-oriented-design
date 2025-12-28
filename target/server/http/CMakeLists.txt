# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project("HTTP" LANGUAGES CXX C)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(include_external)

# add_subdirectory("playground") # use this to test code for later use in web server
# add_subdirectory("tests") # tests to check logic in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- HTTP Server" )
message( STATUS )

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )

file(GLOB TARGET_FILES_ "*.cpp" "*.h" )                                       # all cpp files in root directory
set( TARGET_SOURCE_FILES_
   ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp
   ${APPLICATION_DIRECTORY_}/database/Metadata_Statements.cpp
)

file(GLOB FILES_ "api/*.h" "api/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)

file(GLOB FILES_ "dto/*.h" "dto/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)



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
      # ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp

      ${TARGET_FILES_}
      ${TARGET_SOURCE_FILES_}
     # ${HTTP_FILES_}                                                           # current directory files
     # ${HTTP_STATE_FILES_}                                                     # files from state subdirectory
      "main.cpp"                                                              # main file where method main is located
   )
   
   set_target_properties(${TARGET_NAME_} PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF )

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS GD_LOG_SIMPLE GD_DATABASE_SQLITE_USE GD_DATABASE_ODBC_USE)
   set_compiler_options()


#   if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#      target_link_libraries(${TARGET_NAME_} ${CMAKE_DL_LIBS})
#      target_link_libraries(${TARGET_NAME_} ${CMAKE_SOURCE_DIR}/external/mariadb/libmaodbc.so)
#   elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#   else()
#      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601 ") # GCC needs this to compile boost
#      target_link_libraries(${TARGET_NAME_} PRIVATE odbc32.lib)
#   endif()

# Platform-specific linking
   if(WIN32)
      # Windows linking (both MSVC and clang-cl)
      target_link_libraries(${TARGET_NAME_} PRIVATE  odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib )
      
   elseif(APPLE)
      # macOS linking
      find_library(FOUNDATION_LIBRARY Foundation)
      find_library(COCOA_LIBRARY Cocoa)
      find_library(IOKIT_LIBRARY IOKit)
      find_library(CORESERVICES_LIBRARY CoreServices)
      
      target_link_libraries(${TARGET_NAME_} PRIVATE
         ${FOUNDATION_LIBRARY}
         ${COCOA_LIBRARY}
         ${IOKIT_LIBRARY}
         ${CORESERVICES_LIBRARY}
      )
      
      # macOS ODBC (if installed via Homebrew)
      if(GD_DATABASE_ODBC_USE)
         find_package(ODBC QUIET)
         if(ODBC_FOUND)
            target_link_libraries(${TARGET_NAME_} PRIVATE ${ODBC_LIBRARIES})
         else()
            # Try common ODBC library names on macOS
            find_library(ODBC_LIB iodbc)
            find_library(ODBCINST_LIB odbcinst)
            if(ODBC_LIB AND ODBCINST_LIB)
               target_link_libraries(${TARGET_NAME_} PRIVATE ${ODBC_LIB} ${ODBCINST_LIB})
            else()
               message(WARNING "ODBC libraries not found on macOS. Install via: brew install unixodbc")
            endif()
         endif()
      endif()
      
   elseif(UNIX AND NOT APPLE)
      # Linux linking
      target_link_libraries(${TARGET_NAME_} PRIVATE ${CMAKE_DL_LIBS} pthread rt )
      
      # Linux ODBC
      if(GD_DATABASE_ODBC_USE)
         # Try to find ODBC
         find_package(ODBC QUIET)
         if(ODBC_FOUND)
            target_link_libraries(${TARGET_NAME_} PRIVATE ${ODBC_LIBRARIES})
         else()
            # Try common ODBC library names on Linux
            find_library(ODBC_LIB odbc)
            find_library(ODBCINST_LIB odbcinst)
            if(ODBC_LIB AND ODBCINST_LIB)
               target_link_libraries(${TARGET_NAME_} PRIVATE ${ODBC_LIB} ${ODBCINST_LIB})
            else()
               # Try MariaDB ODBC
               find_library(MARIADB_ODBC_LIB libmaodbc.so PATHS ${CMAKE_SOURCE_DIR}/external/mariadb)
               if(MARIADB_ODBC_LIB)
                  target_link_libraries(${TARGET_NAME_} PRIVATE ${MARIADB_ODBC_LIB})
               else()
                  message(WARNING "ODBC libraries not found on Linux. Install via: sudo apt-get install unixodbc-dev")
               endif()
            endif()
         endif()
      endif()
   endif()


endif()