message(STATUS)
message(STATUS "X-Ray Engine")
message(STATUS "      Version: 1.6")

cmake_policy(SET CMP0140 NEW)

# Setup platform configurations
include(XRay.Platform)

# Setup build configurations
include(XRay.Configs)

# Setup AVX support
include(XRay.AVX)

# Setup compiler
include(XRay.Compiler)

# Setup linker
include(XRay.Linker)

# Define paths
include(XRay.Paths)

# Configure IDE folders
include(XRay.Folders)

# Setup module definition machinery
include(XRay.Modules)

# Setup source utility machinery
include(XRay.Sources)

message(STATUS)
