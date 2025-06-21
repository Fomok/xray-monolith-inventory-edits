# Disable CXX exceptions for the current scope
macro(disable_exceptions)
  string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endmacro()
