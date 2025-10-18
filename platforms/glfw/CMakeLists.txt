cmake_minimum_required(VERSION 3.16)
project(km8_glfw LANGUAGES C)

find_package(OpenGL REQUIRED)

set(KM8_GLFW_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(KM8_GLFW_GLAD_DIR "${KM8_GLFW_THIRD_PARTY_DIR}/glad")
set(KM8_GLFW_GLFW_DIR "${KM8_GLFW_THIRD_PARTY_DIR}/glfw")

if(NOT TARGET glad)
  set(_km8_glad_source "${KM8_GLFW_GLAD_DIR}/src/glad.c")
  if(NOT EXISTS "${_km8_glad_source}")
    message(FATAL_ERROR
      "Unable to locate GLAD sources at ${_km8_glad_source}. "
      "Make sure platforms/glfw/third_party/glad is present or provide your own glad target.")
  endif()
  add_library(glad "${_km8_glad_source}")
  target_include_directories(glad PUBLIC
    $<BUILD_INTERFACE:${KM8_GLFW_GLAD_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
endif()

set(_km8_glfw_target glfw)

if(KM8_USE_SYSTEM_GLFW)
  find_package(glfw3 3.3 REQUIRED)
  if(TARGET glfw::glfw)
    set(_km8_glfw_target glfw::glfw)
  elseif(TARGET glfw)
    set(_km8_glfw_target glfw)
  else()
    message(FATAL_ERROR
      "glfw3 package found but did not provide a known target (expected 'glfw' or 'glfw::glfw').")
  endif()
elseif(KM8_BUILD_GLFW_BUNDLED)
  if(NOT EXISTS "${KM8_GLFW_GLFW_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
      "KM8_BUILD_GLFW_BUNDLED is ON but ${KM8_GLFW_GLFW_DIR} is missing. "
      "Either fetch platforms/glfw/third_party/glfw or enable KM8_USE_SYSTEM_GLFW.")
  endif()

  # disable extra GLFW bits we don't ship with the app
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

  add_subdirectory("${KM8_GLFW_GLFW_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/glfw")
  if(NOT TARGET glfw)
    message(FATAL_ERROR "Bundled GLFW did not define the expected 'glfw' target.")
  endif()
else()
  message(FATAL_ERROR
    "GLFW not provided. Set KM8_USE_SYSTEM_GLFW=ON or KM8_BUILD_GLFW_BUNDLED=ON.")
endif()

# Build an executable that presents the km8 framebuffer via OpenGL
file(GLOB KM8_GLFW_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(km8_glfw ${KM8_GLFW_SRC})

target_include_directories(km8_glfw PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(km8_glfw PRIVATE
  km8::km8
  ${_km8_glfw_target}
  glad
  OpenGL::GL
  ${CMAKE_DL_LIBS}
)

if(UNIX AND NOT APPLE)
  target_link_libraries(km8_glfw PRIVATE m)
endif()

# macOS: silence deprecation as needed
if(APPLE)
  target_compile_definitions(km8_glfw PRIVATE GL_SILENCE_DEPRECATION)
endif()
