#-------------------------------------------------
#
# Project created by QtCreator 2019-05-10T12:39:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 06_quaacfullui
TEMPLATE = app

include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../../libs/Utils/utils.pri)
include($$PWD/../../libs/qadvanceddocking.pri)

SOURCES += main.cpp \
    quaacfullui.cpp

HEADERS += \
    quaacfullui.h

FORMS += \
    quaacfullui.ui

include($$PWD/../../src/types/quaaccesscontrol.pri)
include($$PWD/../../src/widgets/quaaccesscontrolwidgets.pri)
include($$PWD/../../src/docks/quaacdocking.pri)

include($$PWD/../../libs/add_qad_path_win.pri)