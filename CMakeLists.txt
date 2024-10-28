# CMakeList.txt : CMake project for ERD, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# -----------------------------------------------------------------------------
# Add path to cmake scripts
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")


set(CMAKE_CXX_STANDARD 20)

# TODO: Add tests and install targets if needed.
add_subdirectory( "target/DOD" )
add_subdirectory( "target/server/socket" )
add_subdirectory( "target/GD-samples" )