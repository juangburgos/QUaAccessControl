QT += core gui

CONFIG += c++11

include($$PWD/../../libs/qadvanceddocking.pri)

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quaacdocking.h

SOURCES += \
    $$PWD/quaacdocking.cpp

FORMS += \

include($$PWD/../../libs/add_qad_path_win.pri)