if(TARGET liquid)
  add_library(Liquid::liquid ALIAS liquid)
  set(Liquid_FOUND TRUE)
  return()
endif()

find_path(Liquid_INCLUDE_DIR liquid/liquid.h)
find_library(Liquid_LIBRARY liquid)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Liquid REQUIRED_VARS Liquid_INCLUDE_DIR Liquid_LIBRARY)

if(Liquid_FOUND AND NOT TARGET Liquid::liquid)
  add_library(Liquid::liquid UNKNOWN IMPORTED)
  set_target_properties(Liquid::liquid PROPERTIES
    IMPORTED_LOCATION "${Liquid_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Liquid_INCLUDE_DIR}")
endif()
