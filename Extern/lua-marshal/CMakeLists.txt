add_library(lua-marshal STATIC)

target_folder(lua-marshal ${FOLDER_LUA})

set(LUAMARSHAL_DIR ../lua-marshal-git)

# Patch lmarshal.c with named library initialization
set(SOURCE_PAT "int luaopen_marshal.*")

set(DEST_STR
"int luaopen_marshal(lua_State *L)
{
    luaL_register(L, \"marshal\", R);
    return 1;
}")

file(READ ${LUAMARSHAL_DIR}/lmarshal.c LMARSHAL_C)
string(REGEX REPLACE "${SOURCE_PAT}" "${DEST_STR}" LMARSHAL_C "${LMARSHAL_C}")
file(GENERATE OUTPUT lmarshal.c CONTENT "${LMARSHAL_C}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS lmarshal.c)

# Add modified source
target_sources(lua-marshal
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/lmarshal.c
)

target_link_libraries(lua-marshal
  PRIVATE
  LuaJIT
)