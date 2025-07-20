include_guard()

cmake_minimum_required(VERSION 3.12)

# Shared compiler flags
set(XRAY_COMPILER_FLAGS
    # Enable rich diagnostics
    /diagnostics:column
    # Disable minimal rebuild
    /Gm-
    # Precise floating-point mode
    /fp:precise
    # Enable multi-process compilation
    /MP
    # Treat warnings as errors
    /WX
    # Use full paths in diagnostic messages
    /FC
    # Store debug information in PDB
    /Zi
    # Use multi-threaded DLL
    /MD
    # Use full paths in diagnostics
    /FC
    # Suppress 'Use /EHsc' warning
    /wd4530
)

# Debug flags
set(XRAY_COMPILER_FLAGS_DEBUG
    # Don't omit frame pointers
    /Oy-
    # Warning level 4
    /W4
    # Enable security check
    /GS
    # Enable exceptions
    /EHsc
)

# Release flags
set(XRAY_COMPILER_FLAGS_RELEASE
    # Aggressive function inlining
    /Ob3
    # Omit frame pointers
    /Oy
    # Favor code speed over size
    /Ot
    # Fiber-safe optimizations
    /GT
    # Function-level linking
    /Gy
    # Intrinsic functions
    /Oi
    # String pooling
    /GF
    # Whole-program optimization
    /GL
    # Disable security check
    /GS-
    # Not Debug
    /DNDEBUG
    # Disable iterator debugging
    /D_HAS_ITERATOR_DEBUGGING=0
    /D_SECURE_SCL=0
)

# Shared linker options
set(XRAY_LINKER_FLAGS
    # Mark verified with data execution prevention
    /NXCOMPAT
    # Enable large-address awareness
    /LARGEADDRESSAWARE
    # Enable error reporting prompt
    /ERRORREPORT:PROMPT
    # Don't use a dynamic base address
    /DYNAMICBASE:NO
    # Allow larger PDBs
    /PDBPAGESIZE:16384
    # Treat warnings as errors
    /WX
)

# Debug linker options
set(XRAY_LINKER_FLAGS_DEBUG
    # Disable COMDAT folding
    /OPT:NOICF
    # Don't eliminate unreferenced functions and data
    /OPT:NOREF
    # Don't use a fixed address space
    /FIXED:NO
)

# Release linker options
set(XRAY_LINKER_FLAGS_RELEASE
    # Enable COMDAT folding
    /OPT:ICF
    # Eliminate unreferenced functions and data
    /OPT:REF
    # Enable link-time code generation
    /LTCG
)
