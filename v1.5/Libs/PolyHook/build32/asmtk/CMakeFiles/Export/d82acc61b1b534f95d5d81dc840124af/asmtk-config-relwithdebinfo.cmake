#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "asmjit::asmtk" for configuration "RelWithDebInfo"
set_property(TARGET asmjit::asmtk APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(asmjit::asmtk PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/asmtk.lib"
  )

list(APPEND _cmake_import_check_targets asmjit::asmtk )
list(APPEND _cmake_import_check_files_for_asmjit::asmtk "${_IMPORT_PREFIX}/lib/asmtk.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
