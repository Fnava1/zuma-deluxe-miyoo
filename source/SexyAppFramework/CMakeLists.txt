project(SexyAppFramework LANGUAGES C CXX)

set(SEXYAPP_SOURCES
    Buffer.cpp
    ButtonWidget.cpp
    Checkbox.cpp
    Color.cpp
    Common.cpp
    CritSect.cpp
    CursorWidget.cpp
    DDImage.cpp
    DDInterface.cpp
    Debug.cpp
    DescParser.cpp
    DialogButton.cpp
    Dialog.cpp
    EditWidget.cpp
    Flags.cpp
    Font.cpp
    Graphics.cpp
    HyperlinkWidget.cpp
    Image.cpp
    ImageFont.cpp
    Insets.cpp
    KeyCodes.cpp
    ListWidget.cpp
    MemoryImage.cpp
    ModVal.cpp
    MTRand.cpp
    MusicInterface.cpp
    NativeDisplay.cpp
    PerfTimer.cpp
    PropertiesParser.cpp
    Quantize.cpp
    Ratio.cpp
    ResourceManager.cpp
    ScrollbarWidget.cpp
    ScrollbuttonWidget.cpp
    SexyAppBase.cpp
    SexyMatrix.cpp
    SharedImage.cpp
    Slider.cpp
    SWTri.cpp
    SWTri_Pixel555.cpp
    SWTri_Pixel565.cpp
    SWTri_Pixel8888.cpp
    SWTri_Pixel888.cpp
    SysFont.cpp
    TextWidget.cpp
    WidgetContainer.cpp
    Widget.cpp
    WidgetManager.cpp
    XMLParser.cpp
)

if(WIN32)
    list(APPEND SEXYAPP_SOURCES
        BassLoader.cpp
        BassMusicInterface.cpp
        D3D8Helper.cpp
        D3DInterface.cpp
        D3DTester.cpp
        DirectXErrorString.cpp
        DSoundInstance.cpp
        DSoundManager.cpp
        HTTPTransfer.cpp
        SEHCatcher.cpp
    )
else()
    list(APPEND SEXYAPP_SOURCES
        D3DInterface_stub.cpp
    )
endif()

list(APPEND SEXYAPP_SOURCES
    ogg/bitwise.c
    ogg/block.c
    ogg/codebook.c
    ogg/floor0.c
    ogg/floor1.c
    ogg/framing.c
    ogg/info.c
    ogg/mapping0.c
    ogg/mdct.c
    ogg/registry.c
    ogg/res012.c
    ogg/sharedbook.c
    ogg/synthesis.c
    ogg/vorbisfile.c
    ogg/window.c
)

add_library(SexyAppFramework STATIC ${SEXYAPP_SOURCES})

target_include_directories(SexyAppFramework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../compat
    ${CMAKE_CURRENT_SOURCE_DIR}/ogg
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/include/SDL2
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/libxmp-lite/include/libxmp-lite
)

if(WIN32)
    target_link_libraries(SexyAppFramework ImageLib PakLib ws2_32 winmm gdi32 ole32)
else()
    target_link_libraries(SexyAppFramework ImageLib PakLib xmp_lite_static pthread dl m)
endif()