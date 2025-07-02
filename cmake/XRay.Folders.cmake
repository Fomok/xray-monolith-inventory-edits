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
set(FOLDER_XRAY X-Ray)
set(FOLDER_XRAY_COLLISION X-Ray/Collision)
set(FOLDER_XRAY_CORE X-Ray/Core)
set(FOLDER_XRAY_CORE_COMPRESSION X-Ray/Core/Compression)
set(FOLDER_XRAY_CORE_MEMORY X-Ray/Core/Memory)
set(FOLDER_XRAY_CORE_DEBUG X-Ray/Core/Debug)
set(FOLDER_XRAY_CPU_PIPE "X-Ray/CPU Pipe")
set(FOLDER_XRAY_NET_SERVER "X-Ray/Net Server")
set(FOLDER_XRAY_PARTICLES X-Ray/Particles)
set(FOLDER_XRAY_PHYSICS X-Ray/Physics)
set(FOLDER_XRAY_SOUND X-Ray/Sound)
set(FOLDER_XRAY_GAME X-Ray/Game)
set(FOLDER_XRAY_GAME_AI X-Ray/Game/AI)
set(FOLDER_XRAY_GAME_AI_COMPONENTS X-Ray/Game/AI/Components)
set(FOLDER_XRAY_XML_PARSER "X-Ray/XML Parser")
set(FOLDER_RENDER X-Ray/Render)
set(FOLDER_GAMEDATA X-Ray/Gamedata)
set(FOLDER_ENGINE X-Ray/Engine)
set(FOLDER_RELEASE Release)

macro(target_folder TARGET FOLDER)
  set_property(
    TARGET ${TARGET}
    PROPERTY FOLDER
    ${FOLDER}
  )
endmacro()