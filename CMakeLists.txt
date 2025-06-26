cmake_minimum_required(VERSION 3.19)
project(Busbomproject LANGUAGES CXX)

# ✅ Qt6 모듈 한 번에 묶기
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network LinguistTools)

qt_standard_project_setup()

qt_add_executable(Busbomproject
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    settingsdialog.h
    settingsdialog.cpp
    img.qrc
)

qt_add_translations(
    TARGETS Busbomproject
    TS_FILES Busbomproject_ko_KR.ts
)

target_link_libraries(Busbomproject
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS Busbomproject
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Busbomproject
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
