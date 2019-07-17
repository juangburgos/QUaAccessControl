#-------------------------------------------------
#
# Project created by QtCreator 2019-05-10T12:39:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 04_quaroletable
TEMPLATE = app

include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../../libs/Utils/utils.pri)

SOURCES += main.cpp \
    quaroletabletestdialog.cpp

HEADERS += \
    quaroletabletestdialog.h

FORMS += \
    quaroletabletestdialog.ui

include($$PWD/../../src/types/quaaccesscontrol.pri)
include($$PWD/../../src/widgets/quaaccesscontrolwidgets.pri)
include($$PWD/../../libs/QUaServer.git/src/helper/add_qt_path_win.pri)
