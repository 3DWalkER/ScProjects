#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ScUtils::ScUtils" for configuration "Debug"
set_property(TARGET ScUtils::ScUtils APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(ScUtils::ScUtils PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/scutilsd.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/scutilsd.dll"
  )

list(APPEND _cmake_import_check_targets ScUtils::ScUtils )
list(APPEND _cmake_import_check_files_for_ScUtils::ScUtils "${_IMPORT_PREFIX}/lib/scutilsd.lib" "${_IMPORT_PREFIX}/bin/scutilsd.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
