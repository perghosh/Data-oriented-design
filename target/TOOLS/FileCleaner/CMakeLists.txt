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

file(GLOB FILES_ "cli/*.h" "cli/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)

file(GLOB FILES_ "configuration/*.h" "configuration/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)



add_subdirectory("playground") # playground cleaner logic
add_subdirectory("tests") # tests cleaner logic

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


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

   target_compile_features(${TARGET_NAME_} PRIVATE cxx_std_23)
   set_target_properties(${TARGET_NAME_} PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_LOG_SIMPLE)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)

   if(MSVC)
   else()

      # Linux-specific optimizations for full static linking
      target_compile_options(${TARGET_NAME_} PRIVATE 
         -O3                    # Optimize for size
         -ffunction-sections    # Place each function in its own section
         -fdata-sections        # Place each data item in its own section
         -fno-stack-protector   # Remove stack protection overhead
         -fPIC                  # Position independent code (needed for static linking)
         -mavx2                 # Enable AVX2 SIMD instructions
         -mbmi                  # Enable bit manipulation instructions
      )
      
      # Full static linking - creates completely standalone binary
      target_link_options(${TARGET_NAME_} PRIVATE 
         -static                # Full static linking - links everything statically
         -static-libstdc++      # Static link C++ standard library
         -static-libgcc         # Static link GCC runtime
         -Wl,--gc-sections      # Remove unused sections
         -Wl,--strip-all        # Strip all symbols
         -Wl,--as-needed        # Only link needed libraries
         -s                     # Strip symbol table
         -pthread               # Static link pthread
      )


      # Link GLIBCXX statically
      # target_link_options(${TARGET_NAME_} PRIVATE -static-libstdc++)
   endif()
   
endif()