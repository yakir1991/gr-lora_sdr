# Creates the Liquid::liquid target if it doesn't exist, even without pkg-config/CMake file.

if (NOT TARGET Liquid::liquid)
  find_package(PkgConfig QUIET)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(LIQUID QUIET liquid-dsp)
  endif()

  # Headers
  find_path(LIQUID_INCLUDE_DIR
    NAMES liquid/liquid.h
    HINTS ${LIQUID_INCLUDE_DIRS}
    PATHS /usr/include /usr/local/include
  )

  # Library (some distros use the name liquid-dsp)
  find_library(LIQUID_LIBRARY
    NAMES liquid liquid-dsp
    HINTS ${LIQUID_LIBRARY_DIRS}
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
  )

  if (LIQUID_INCLUDE_DIR AND LIQUID_LIBRARY)
    add_library(Liquid::liquid UNKNOWN IMPORTED)
    set_target_properties(Liquid::liquid PROPERTIES
      IMPORTED_LOCATION "${LIQUID_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LIQUID_INCLUDE_DIR}"
    )
    message(STATUS "Liquid fallback: include=${LIQUID_INCLUDE_DIR}, lib=${LIQUID_LIBRARY}")
  endif()
endif()
