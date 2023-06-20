# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\wave_function_collapse_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\wave_function_collapse_autogen.dir\\ParseCache.txt"
  "wave_function_collapse_autogen"
  )
endif()
