# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project("cleaner" VERSION 1.0.2 LANGUAGES CXX C)

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

file(GLOB TARGET_AUTOMATION_FILES_ "automation/*.cpp")

if(WIN32)
   # Add Windows specific code
   file(GLOB FILES_ "win/*.h" "win/*.cpp" )
   list(APPEND TARGET_FILES_ ${FILES_})
   unset(FILES_)
elseif(APPLE)
   # Add macOS specific code if needed
   # file(GLOB FILES_ "mac/*.h" "mac/*.cpp" )
   # list(APPEND TARGET_FILES_ ${FILES_})
   # unset(FILES_)
else()
   # Linux specific code
endif()

file(GLOB FILES_ "cli/*.h" "cli/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)

file(GLOB FILES_ "configuration/*.h" "configuration/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)

file(GLOB FILES_ "automation/*.h" "automation/*.cpp" "automation/code-analysis/*.h" "automation/code-analysis/*.cpp" )
list(APPEND TARGET_FILES_ ${FILES_})
unset(FILES_)




add_subdirectory("playground") # playground cleaner logic
add_subdirectory("tests") # tests cleaner logic

# set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to control cleaner installation
option(INSTALL_TARGET_ "Install cleaner executable and man page" OFF)

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
      ${GD_SOURCES_ALL} ${external_pugixml} ${external_sqlite}
      ${TARGET_FILES_}   
      ${TARGET_SOURCE_FILES_}
      ${TARGET_AUTOMATION_FILES_}

      "main.cpp"
   )

   target_compile_features(${TARGET_NAME_} PRIVATE cxx_std_23)
   set_target_properties(${TARGET_NAME_} PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_SOURCE_DIR}/source)
   target_compile_definitions(${TARGET_NAME_} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${TARGET_NAME_} PRIVATE GD_LOG_SIMPLE)
   # target_compile_definitions(${TARGET_NAME_} PRIVATE GD_DATABASE_SQLITE_USE)

   if(MSVC)
      # Windows/MSVC specific settings with SIMD optimizations

      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
         # Debug build - minimal optimization but enable debugging
         target_compile_options(${TARGET_NAME_} PRIVATE
            /Od                    # Disable optimizations
            /Zi                    # Generate debug information
            /W3                    # Warning level 3
            /RTC1                  # Runtime error checks
            /MDd                   # Debug runtime library (or /MTd for static)
         )
      elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
         # Release with Debug Info - optimized but with asserts and debug info
         target_compile_options(${TARGET_NAME_} PRIVATE
            /O2                    # Optimize for speed
            /Zi                    # Generate debug information
            /Oi                    # Enable intrinsic functions
            /Ot                    # Favor fast code
            /Gy                    # Function-level linking
            /arch:AVX2             # Enable AVX2 SIMD instructions
            /favor:INTEL64         # Optimize for Intel 64-bit
            /fp:fast               # Fast floating-point model
            # Note: NO /DNDEBUG here - this keeps asserts enabled
         )
         # Less aggressive linking for RelWithDebInfo to preserve debugging capability
         target_link_options(${TARGET_NAME_} PRIVATE
            /OPT:REF               # Remove unreferenced functions/data
            /INCREMENTAL:NO        # Disable incremental linking
         )
      else()
         # Full Release build - maximum optimization, no asserts
         target_compile_options(${TARGET_NAME_} PRIVATE
            /O2                    # Optimize for speed
            /Oi                    # Enable intrinsic functions
            /Ot                    # Favor fast code
            /Oy                    # Omit frame pointers
            /GL                    # Whole program optimization
            /arch:AVX2             # Enable AVX2 SIMD instructions
            /favor:INTEL64         # Optimize for Intel 64-bit
            /fp:fast               # Fast floating-point model
            /GS-                   # Disable security checks for performance
            /Gy                    # Function-level linking
            /DNDEBUG               # Define NDEBUG for release builds
         )
      
         # Release linking with optimization
         target_link_options(${TARGET_NAME_} PRIVATE
            /LTCG                  # Link-time code generation
            /OPT:REF               # Remove unreferenced functions/data
            /OPT:ICF               # Enable COMDAT folding
            /INCREMENTAL:NO        # Disable incremental linking
         )
      endif()
   elseif(APPLE)
      # macOS specific settings - keep it simple
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
         target_compile_options(${TARGET_NAME_} PRIVATE
            -g                     # Generate debug info
            -O0                    # No optimization
            -Wall                  # Enable warnings
         )
      else()
         target_compile_options(${TARGET_NAME_} PRIVATE
            -O3                    # optimization
            -DNDEBUG               # Define NDEBUG for release builds
         )
      endif()
      
      # Link required system frameworks/libraries on macOS
      target_link_libraries(${TARGET_NAME_} PRIVATE pthread)
      
   else()
      # Linux/other Unix systems
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
         # Debug build - prioritize debugging over optimization
         target_compile_options(${TARGET_NAME_} PRIVATE
            -g                     # Generate debug info
            -O0                    # No optimization
            -fno-omit-frame-pointer # Keep frame pointers for better stack traces
            -fno-inline            # Don't inline functions
            -Wall                  # Enable warnings
            #-Wextra                # Extra warnings
         )
         
         # Debug linking - keep symbols and avoid stripping
         target_link_options(${TARGET_NAME_} PRIVATE
            -g                     # Keep debug symbols
            # Remove static linking for debug builds to avoid issues
            -pthread               # Link pthread
         )
      else()
         # Release build - optimize for performance
         target_compile_options(${TARGET_NAME_} PRIVATE
            -O3                    # Optimize for performance
            -ffunction-sections    # Place each function in its own section
            -fdata-sections        # Place each data item in its own section
            -fno-stack-protector   # Remove stack protection overhead
            -fPIC                  # Position independent code
            -mavx2                 # Enable AVX2 SIMD instructions
            -mbmi                  # Enable bit manipulation instructions
         )
         
         # Release linking - full static linking
         target_link_options(${TARGET_NAME_} PRIVATE
            -static                # Full static linking
            -static-libstdc++      # Static link C++ standard library
            -static-libgcc         # Static link GCC runtime
            -Wl,--gc-sections      # Remove unused sections
            -Wl,--strip-all        # Strip all symbols
            -Wl,--as-needed        # Only link needed libraries
            -s                     # Strip symbol table
            -pthread               # Static link pthread
         )
      endif()
   endif()

   # Installation rules for cleaner (only if INSTALL_TARGET_ is ON)
   if(INSTALL_TARGET_)
       if(WIN32)
           # Windows installation: Install cleaner executable to a program directory
           install(TARGETS ${TARGET_NAME_}
               RUNTIME DESTINATION bin
               COMPONENT cleaner
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ
                           WORLD_EXECUTE WORLD_READ
           )
           # Optional: Install additional Windows files (e.g., README, LICENSE)
           #[[
           install(FILES ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/LICENSE
               DESTINATION doc
               COMPONENT cleaner
               PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
               OPTIONAL
           )
           ]]
       elseif(APPLE)
           # macOS installation: Install cleaner executable
           install(TARGETS ${TARGET_NAME_}
               RUNTIME DESTINATION bin
               COMPONENT cleaner
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ
                           WORLD_EXECUTE WORLD_READ
           )
           # Optional: Install additional macOS files
           #[[
           install(FILES ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/LICENSE
               DESTINATION doc
               COMPONENT cleaner
               PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
               OPTIONAL
           )
           ]]
       else()
           # Linux installation: Install cleaner executable and man page
           install(TARGETS ${TARGET_NAME_}
               RUNTIME DESTINATION bin
               COMPONENT cleaner
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ
                           WORLD_EXECUTE WORLD_READ
           )
           # Install man page (Linux only)
           #[[
           install(FILES ${CMAKE_SOURCE_DIR}/docs/cleaner.1
               DESTINATION share/man/man1
               COMPONENT cleaner
               PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
               OPTIONAL
           )
           ]]
           # Compress man page if gzip is available
           find_program(GZIP gzip)
           if(GZIP)
               # install(CODE "execute_process(COMMAND ${GZIP} -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/cleaner.1)" COMPONENT cleaner)
           endif()
       endif()
   endif()

   # Custom target to install only cleaner
   add_custom_target(install-cleaner
       COMMAND ${CMAKE_COMMAND} --install . --component cleaner
       DEPENDS ${TARGET_NAME_}
       COMMENT "Installing cleaner executable and platform-specific files"
   )
endif()


# Optional: CPack for creating installers
include(CPack)
cpack_add_component(cleaner DISPLAY_NAME "Cleaner Executable and Documentation")