set(KM8_EMULATOR_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

if(WIN32)
  list(APPEND KM8_EMULATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platform/platform_win32.c)
elseif(UNIX AND NOT APPLE)
  list(APPEND KM8_EMULATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platform/platform_linux.c)
else()
  list(APPEND KM8_EMULATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platform/platform_stub.c)
endif()

add_executable(km8_emulator ${KM8_EMULATOR_SRC})
set_target_properties(km8_emulator PROPERTIES OUTPUT_NAME "km8")

target_include_directories(km8_emulator
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(km8_emulator
  PRIVATE
    km8::km8
)

if(WIN32)
  target_link_libraries(km8_emulator PRIVATE user32 gdi32)
elseif(UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
  target_include_directories(km8_emulator PRIVATE ${X11_INCLUDE_DIR})
  target_link_libraries(km8_emulator PRIVATE ${X11_LIBRARIES})
endif()
