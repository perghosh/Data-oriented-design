# compile_modes.cmake - Simple compile mode flags for multiple targets
#
# HOW TO USE:
#   1. Include this file: include(cmake/compile_modes.cmake)
#   2. Call setup_compile_flags with your target and what you want:
#
#   setup_compile_flags(my_target "DEVELOPER,VERBOSE,SAFE")
#   setup_compile_flags(my_target "DEVELOPER,MEMORY")
#   setup_compile_flags(my_target)  # No flags (production build)

#[[ Sample C++ code
// Check if Developer mode is enabled (bit 0)
#if (TARGET_COMPILE_MODE_ & 1)
    // Developer mode code
#endif

// Check if Verbose mode is enabled (bit 1)
#if (TARGET_COMPILE_MODE_ & 2)
    // Verbose logging
#endif

// Check if Memory tracking is enabled (bit 2)
#if (TARGET_COMPILE_MODE_ & 4)
    // Memory tracking code
#endif

// Check if Safe mode is enabled (bit 3)
#if (TARGET_COMPILE_MODE_ & 8)
    // Safety checks
#endif
]]

# These are the bit values - don't change these
set(FLAG_MODE_DEVELOPER_ 1)
set(FLAG_MODE_VERBOSE_   2)
set(FLAG_MODE_MEMORY_    4)
set(FLAG_MODE_SAFE_      8)

# Main function - give it a target name and optional list of flags
function(setup_compile_flags TARGET_NAME)
    # Start with no flags
    set(TARGET_COMPILE_MODE_ 0)
    
    # Check if user provided any flags
    if(ARGC GREATER 1)
        set(FLAG_LIST ${ARGV1})
        
        # Developer mode flag
        if(FLAG_LIST MATCHES "DEVELOPER")
            math(EXPR TARGET_COMPILE_MODE_ "${TARGET_COMPILE_MODE_} | ${FLAG_MODE_DEVELOPER_}")
            message(STATUS "  - Developer mode: ON")
        endif()
        
        # Verbose logging flag
        if(FLAG_LIST MATCHES "VERBOSE")
            math(EXPR TARGET_COMPILE_MODE_ "${TARGET_COMPILE_MODE_} | ${FLAG_MODE_VERBOSE_}")
            message(STATUS "  - Verbose mode: ON")
        endif()
        
        # Memory tracking flag
        if(FLAG_LIST MATCHES "MEMORY")
            math(EXPR TARGET_COMPILE_MODE_ "${TARGET_COMPILE_MODE_} | ${FLAG_MODE_MEMORY_}")
            message(STATUS "  - Memory tracking: ON")
        endif()
        
        # Safe mode flag (extra checks)
        if(FLAG_LIST MATCHES "SAFE")
            math(EXPR TARGET_COMPILE_MODE_ "${TARGET_COMPILE_MODE_} | ${FLAG_MODE_SAFE_}")
            message(STATUS "  - Safe mode: ON")
        endif()
    endif()
    
    # Store the final mask
    set_target_properties(${TARGET_NAME} PROPERTIES TARGET_COMPILE_MODE_ ${TARGET_COMPILE_MODE_})
    target_compile_definitions(${TARGET_NAME} PRIVATE TARGET_COMPILE_MODE_=${TARGET_COMPILE_MODE_})
    
    # Summary message
    if(TARGET_COMPILE_MODE_ EQUAL 0)
        message(STATUS "Target '${TARGET_NAME}': Production build (TARGET_COMPILE_MODE_ = 0)")
    else()
        message(STATUS "Target '${TARGET_NAME}': TARGET_COMPILE_MODE_ = ${TARGET_COMPILE_MODE_}")
    endif()
endfunction()