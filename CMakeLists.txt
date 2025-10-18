cmake_minimum_required(VERSION 3.16)
project(km8 LANGUAGES C)

# ---- options ---------------------------------------------------------------
option(KM8_BUILD_SHARED        "Build km8 as a shared library"           ON)
option(KM8_BUILD_GLFW_APP      "Build the GLFW/OpenGL frontend app"      ON)
option(KM8_USE_SYSTEM_GLFW     "Use system GLFW (find_package)"          OFF)
option(KM8_BUILD_GLFW_BUNDLED  "Build bundled third_party/glfw"          ON)
option(KM8_INSTALL             "Install/export km8 package"              ON)

if(NOT PROJECT_IS_TOP_LEVEL)
  # When embedded in another project, don't force apps or vendoring by default
  set(KM8_BUILD_GLFW_APP     OFF CACHE BOOL "" FORCE)
  set(KM8_BUILD_GLFW_BUNDLED OFF CACHE BOOL "" FORCE)
endif()

# ---- language/flags --------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
  add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# neat build dirs (only at top level)
if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
      string(TOUPPER "${cfg}" CFGU)
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/bin/${cfg})
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/bin/${cfg})
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/lib/${cfg})
    endforeach()
  endif()
endif()

# ---- core library: km8 -----------------------------------------------------
file(GLOB_RECURSE KM8_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"      # core
  "${CMAKE_CURRENT_SOURCE_DIR}/emulator/*.c" # your implementation (optional now)
)

set(_libtype STATIC)
if(KM8_BUILD_SHARED)
  set(_libtype SHARED)
endif()

add_library(km8 ${_libtype} ${KM8_SRC})
add_library(km8::km8 ALIAS km8)

target_include_directories(km8
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
)

# if your core needs math/dl on *nix, add here; keep GLFW/GL out of the core
if(UNIX AND NOT APPLE)
  target_link_libraries(km8 PRIVATE m)
endif()

# ---- frontend app(s) -------------------------------------------------------
if(KM8_BUILD_GLFW_APP)
  add_subdirectory(platforms/glfw)
endif()

# ---- install/export (optional) ---------------------------------------------
if(KM8_INSTALL AND PROJECT_IS_TOP_LEVEL)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  set(_km8_install_targets km8)
  if(TARGET glad)
    list(APPEND _km8_install_targets glad)
  endif()

  install(TARGETS ${_km8_install_targets}
    EXPORT km8Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/km8Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/km8Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/km8
  )
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/km8ConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/km8Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/km8ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/km8
  )
  install(EXPORT km8Targets NAMESPACE km8:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/km8)
endif()
