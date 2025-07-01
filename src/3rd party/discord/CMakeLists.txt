add_library(discord INTERFACE)

set_property(
  TARGET discord
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(discord
  INTERFACE
  achievement_manager.cpp
  achievement_manager.h
  activity_manager.cpp
  activity_manager.h
  application_manager.cpp
  application_manager.h
  core.cpp
  core.h
  discord.h
  event.h
  ffi.h
  image_manager.cpp
  image_manager.h
  lobby_manager.cpp
  lobby_manager.h
  network_manager.cpp
  network_manager.h
  overlay_manager.cpp
  overlay_manager.h
  relationship_manager.cpp
  relationship_manager.h
  storage_manager.cpp
  storage_manager.h
  store_manager.cpp
  store_manager.h
  types.cpp
  types.h
  user_manager.cpp
  user_manager.h
  voice_manager.cpp
  voice_manager.h
)

target_include_directories(discord
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(discord_game_sdk SHARED IMPORTED)
set_property(
  TARGET discord_game_sdk
  PROPERTY IMPORTED_LOCATION
  ${CMAKE_SOURCE_DIR}/sdk/binaries/discord_game_sdk.dll
)
set_property(
  TARGET discord_game_sdk
  PROPERTY
  IMPORTED_IMPLIB
  ${CMAKE_SOURCE_DIR}/sdk/libraries/x64.old/discord_game_sdk.lib
)

target_link_libraries(discord
  INTERFACE
  discord_game_sdk
)
