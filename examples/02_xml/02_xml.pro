QT += core
QT -= gui

TARGET  = 02_xml
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

include($$PWD/../../src/types/quaaccesscontrol.pri)
include($$PWD/../../libs/QUaServer.git/src/helper/add_qt_path_win.pri)