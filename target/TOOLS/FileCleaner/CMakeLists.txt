# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("cleaner" LANGUAGES CXX C)

include(include_external)

# add_subdirectory("playground") # use this to test code for later use in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- file targets" )
message( STATUS )

set( SOURCE_DIRECTORY_ ${CMAKE_SOURCE_DIR}/source )
set( APPLICATION_DIRECTORY_ ${SOURCE_DIRECTORY_}/application )

file(GLOB TARGET_FILES_ "*.cpp" )
set( TARGET_SOURCE_FILES_
   ${APPLICATION_DIRECTORY_}/ApplicationBasic.cpp
   ${APPLICATION_DIRECTORY_}/database/Metadata_Statements.cpp
)

if(WIN32)
   # Add Windows specific code
   file(GLOB FILES_ "win/*.h" "win/*.cpp" )
   list(APPEND TARGET_FILES_ ${FILES_})
   unset(FILES_)
else()
endif()



add_subdirectory("playground") # playground cleaner logic
add_subdirectory("tests") # tests cleaner logic



set( USE_TARGET_ ON ) # ========================================================= ${cleaner}

if(MSVC)
   # Use static runtime for MSVC
   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
else()
   # add_compile_options(-gdwarf-4) # Use DWARF 4 for compatibility with older gdb versions
endif()

if( USE_TARGET_ )

   set(TARGET_NAME_ "cleaner")

   add_executable(${TARGET_NAME_}
      ${external_gd} ${external_pugixml} ${external_sqlite}
      ${TARGET_FILES_}   
      ${TARGET_SOURCE_FILES_}

      "main.cpp"
   )

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_LOG_SIMPLE)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)

   if(MSVC)
   else()
      # Link GLIBCXX statically
      target_link_options(${TARGET_NAME_} PRIVATE -static-libstdc++)
   endif()
   
endif()