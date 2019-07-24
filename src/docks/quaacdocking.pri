QT += core gui

CONFIG += c++11

include($$PWD/../../libs/qadvanceddocking.pri)

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quaacdocking.h \
    $$PWD/qaddockwidgetwrapper.h

SOURCES += \
    $$PWD/quaacdocking.cpp \
    $$PWD/qaddockwidgetwrapper.cpp

FORMS += \
    $$PWD/qaddockwidgetwrapper.ui

include($$PWD/../../libs/add_qad_path_win.pri)