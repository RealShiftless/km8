#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "km8::km8" for configuration "Debug"
set_property(TARGET km8::km8 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(km8::km8 PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/libkm8.dll.a"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/libkm8.dll"
  )

list(APPEND _cmake_import_check_targets km8::km8 )
list(APPEND _cmake_import_check_files_for_km8::km8 "${_IMPORT_PREFIX}/lib/libkm8.dll.a" "${_IMPORT_PREFIX}/bin/libkm8.dll" )

# Import target "km8::glad" for configuration "Debug"
set_property(TARGET km8::glad APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(km8::glad PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libglad.a"
  )

list(APPEND _cmake_import_check_targets km8::glad )
list(APPEND _cmake_import_check_files_for_km8::glad "${_IMPORT_PREFIX}/lib/libglad.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
