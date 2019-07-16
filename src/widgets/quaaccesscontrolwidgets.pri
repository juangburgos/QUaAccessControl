QT += core gui

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quausertable.h \
    $$PWD/quauserwidgetedit.h \
    $$PWD/quaaccommondialog.h  \
    $$PWD/quaaccommonwidgets.h 

SOURCES += \
    $$PWD/quausertable.cpp \
    $$PWD/quauserwidgetedit.cpp \
    $$PWD/quaaccommondialog.cpp  \
    $$PWD/quaaccommonwidgets.cpp  

FORMS += \
    $$PWD/quausertable.ui \
    $$PWD/quauserwidgetedit.ui \
    $$PWD/quaaccommondialog.ui 