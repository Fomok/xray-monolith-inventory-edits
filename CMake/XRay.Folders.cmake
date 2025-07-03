include_guard()

# Rename built-in CMake folder
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)

# Enable folder management of modules
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define folder names
set(FOLDER_LUA External/Lua)
set(FOLDER_EXTERNAL External)
set(FOLDER_LUA_BUILD External/Lua/Build)
set(FOLDER_SDK External/SDK)
set(FOLDER_CI CI)
set(FOLDER_EXECUTABLES Executables)

macro(target_folder TARGET FOLDER)
  set_property(
    TARGET ${TARGET}
    PROPERTY FOLDER
    ${FOLDER}
  )
endmacro()