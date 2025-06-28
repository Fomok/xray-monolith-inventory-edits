include_guard()

# Remove built-in CMake flags
string(
    REPLACE
    "/Ob2" ""
    CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}"
)

string(
    REPLACE "/Ob1" ""
    CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
)

# Configure compiler options
add_compile_options(
    ## Shared config
    # Enable rich diagnostics
    /diagnostics:column
    # Disable minimal rebuild
    /Gm-
    # Precise floating-point mode
    /fp:precise
    # Enable multi-process compilation
    /MP
    # Don't treat warnings as errors
    /WX-
    # Use full paths in diagnostic messages
    /FC
    # Store debug information in PDB
    /Zi
    # Use multi-threaded DLL
    /MD

    ## Debug config
    # Don't omit frame pointers
    $<$<CONFIG:Debug>:/Oy->
    $<$<CONFIG:Verified>:/Oy->
    # Warning level 4
    $<$<CONFIG:Debug>:/W4>
    $<$<CONFIG:Verified>:/W4>
    # Enable security check
    $<$<CONFIG:Debug>:/GS>
    $<$<CONFIG:Verified>:/GS>

    ## Release configs
    # Aggressive function inlining
    $<$<CONFIG:Release>:/Ob3>
    $<$<CONFIG:RelWithDebInfo>:/Ob3>
    # Omit frame pointers
    $<$<CONFIG:Release>:/Oy>
    $<$<CONFIG:RelWithDebInfo>:/Oy>
    # Fiber-safe optimizations
    $<$<CONFIG:Release>:/GT>
    $<$<CONFIG:RelWithDebInfo>:/GT>
    # Function-level linking
    $<$<CONFIG:Release>:/Gy>
    $<$<CONFIG:RelWithDebInfo>:/Gy>
    # Intrinsic functions
    $<$<CONFIG:Release>:/Oi>
    $<$<CONFIG:RelWithDebInfo>:/Oi>
    # String pooling
    $<$<CONFIG:Release>:/GF>
    $<$<CONFIG:RelWithDebInfo>:/GF>
    # Whole-program optimization
    $<$<CONFIG:Release>:/GL>
    $<$<CONFIG:RelWithDebInfo>:/GL>
)

# Configure linker options
add_link_options(
    ## Shared config
    # Mark verified with data execution prevention
    /NXCOMPAT
    # Enable large-address awareness
    /LARGEADDRESSAWARE
    # Enable error reporting prompt
    /ERRORREPORT:PROMPT
    # Don't use a dynamic base address
    /DYNAMICBASE:NO

    ## Debug config
    # Disable COMDAT folding
    $<$<CONFIG:Debug>:/OPT:NOICF>
    $<$<CONFIG:Verified>:/OPT:NOICF>
    # Link incrementally
    $<$<CONFIG:Debug>:/INCREMENTAL>
    $<$<CONFIG:Verified>:/INCREMENTAL>
    # Don't eliminate unreferenced functions and data
    $<$<CONFIG:Debug>:/OPT:NOREF>
    $<$<CONFIG:Verified>:/OPT:NOREF>
    # Don't use a fixed address space
    $<$<CONFIG:Debug>:/FIXED:NO>
    $<$<CONFIG:Verified>:/FIXED:NO>

    ## Release / RelWithDebInfo config
    # Enable COMDAT folding
    $<$<CONFIG:Release>:/OPT:ICF>
    $<$<CONFIG:RelWithDebInfo>:/OPT:ICF>
    # Don't link incrementally
    $<$<CONFIG:Release>:/INCREMENTAL:NO>
    $<$<CONFIG:RelWithDebInfo>:/INCREMENTAL:NO>
    # Eliminate unreferenced functions and data
    $<$<CONFIG:Release>:/OPT:REF>
    $<$<CONFIG:RelWithDebInfo>:/OPT:REF>
    # Enable link-time code generation
    $<$<CONFIG:Release>:/LTCG>
    $<$<CONFIG:RelWithDebInfo>:/LTCG>
)

# Configure exceptions
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " /EHsc")
