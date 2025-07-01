include_guard()

# Rename built-in CMake folder
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)

# Enable folder management of modules
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define folder names
set(FOLDER_XRAY X-Ray)
set(FOLDER_EXTERNAL External)
set(FOLDER_XRAY_CDB "X-Ray/CDB")
set(FOLDER_XRAY_GAME "X-Ray/Game")
set(FOLDER_XRAY_XML_PARSER "X-Ray/XML Parser")
set(FOLDER_LUA X-Ray/Lua)
set(FOLDER_LUA_BUILD X-Ray/Lua/Build)
set(FOLDER_RENDER X-Ray/Render)
set(FOLDER_SDK X-Ray/SDK)
set(FOLDER_GAMEDATA X-Ray/Gamedata)
set(FOLDER_TARGETS Targets)
set(FOLDER_RELEASE Release)
