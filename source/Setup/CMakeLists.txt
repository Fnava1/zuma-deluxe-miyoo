project(ZumaSetup LANGUAGES C)

set(ZUMA_SETUP_SOURCES
    zuma_setup.c
    pack_pak_native.c
    mo3_decode.c
    font8x8.c
)

add_executable(zuma_setup ${ZUMA_SETUP_SOURCES})

target_include_directories(zuma_setup PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/include/SDL2
)

target_link_libraries(zuma_setup PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/lib/libSDL2.so
    dl
    m
)

set_target_properties(zuma_setup PROPERTIES
    BUILD_RPATH "\$ORIGIN/libs;\$ORIGIN/lib;\$ORIGIN;."
    INSTALL_RPATH "\$ORIGIN/libs;\$ORIGIN/lib;\$ORIGIN;."
)
