project(CircleShoot LANGUAGES CXX)

set(CIRCLESHOOT_SOURCES
    AdventureScreen.cpp
    Ball.cpp
    BlendedImage.cpp
    Board.cpp
    Bullet.cpp
    CircleButton.cpp
    CircleCheckbox.cpp
    CircleCommon.cpp
    CircleDialog.cpp
    CircleShootApp.cpp
    CreateUserDialog.cpp
    CreditsScreen.cpp
    CurveData.cpp
    CurveMgr.cpp
    DataSync.cpp
    Gun.cpp
    HelpScreen.cpp
    HighScoreMgr.cpp
    LevelParser.cpp
    LoadingScreen.cpp
    Localization.cpp
    Main.cpp
    MainMenu.cpp
    MoreGamesScreen.cpp
    OptionsDialog.cpp
    ParticleMgr.cpp
    PracticeScreen.cpp
    ProfileMgr.cpp
    Res.cpp
    SoundMgr.cpp
    SpriteMgr.cpp
    StatsDialog.cpp
    TransitionMgr.cpp
    UserDialog.cpp
    WayPoint.cpp
    WidgetMover.cpp
    WorkerThread.cpp
    ../compat/casepath.c
)

if(WIN32)
    add_executable(Zuma WIN32 ${CIRCLESHOOT_SOURCES} Zuma.rc)
    target_link_libraries(Zuma SexyAppFramework)
else()
    add_executable(Zuma ${CIRCLESHOOT_SOURCES})
    target_link_libraries(Zuma PRIVATE
        SexyAppFramework
        xmp_lite_static
        ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/lib/libSDL2.so
        ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/lib/libneonarmmiyoo.so
        stdc++fs
        pthread
        dl
        m
    )
    set_target_properties(Zuma PROPERTIES
        BUILD_RPATH "\$ORIGIN/libs;\$ORIGIN/lib;\$ORIGIN;."
        INSTALL_RPATH "\$ORIGIN/libs;\$ORIGIN/lib;\$ORIGIN;."
    )
endif()

target_include_directories(Zuma PRIVATE
    ${SEXYAPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../compat
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/SDL2/include/SDL2
)