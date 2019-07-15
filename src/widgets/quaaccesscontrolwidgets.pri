QT += core gui

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quausertable.h \
    $$PWD/quauserwidgetedit.h \
    $$PWD/quaaccommondialog.h

SOURCES += \
    $$PWD/quausertable.cpp \
    $$PWD/quauserwidgetedit.cpp \
    $$PWD/quaaccommondialog.cpp

FORMS += \
    $$PWD/quausertable.ui \
    $$PWD/quauserwidgetedit.ui \
    $$PWD/quaaccommondialog.ui