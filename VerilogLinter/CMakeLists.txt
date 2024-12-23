cmake_minimum_required(VERSION 3.16)

project(VerilogLinter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(appVerilogLinter
    main.cpp
)

qt_add_qml_module(appVerilogLinter
    URI VerilogLinter
    VERSION 1.0
    QML_FILES main.qml
    SOURCES VerilogLintChecker.h VerilogLintChecker.cpp

    RESOURCES
    assets/logo.png
    SOURCES FileDisplayer.h FileDisplayer.cpp
    SOURCES ArithmeticOverflowChecker.h ArithmeticOverflowChecker.cpp
    SOURCES
    SOURCES FSMChecker.h FSMChecker.cpp
    SOURCES CaseChecker.h CaseChecker.cpp
    SOURCES BlockChecker.h BlockChecker.cpp
    SOURCES LatchChecker.h LatchChecker.cpp
    SOURCES UninitialisedChecker.h UninitialisedChecker.cpp
    SOURCES ParserEngine.h ParserEngine.cpp

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appVerilogLinter PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appVerilogLinter
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appVerilogLinter
    PRIVATE Qt6::Quick)

include(GNUInstallDirs)
install(TARGETS appVerilogLinter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
