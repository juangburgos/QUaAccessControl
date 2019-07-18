QT += core gui

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quausertable.h \
    $$PWD/quauserwidgetedit.h \
    $$PWD/quaaccommondialog.h  \
    $$PWD/quaaccommonwidgets.h \ 
    $$PWD/quaroletable.h \
    $$PWD/quarolewidgetedit.h \
    $$PWD/quapermissionstable.h \
    $$PWD/quapermissionswidgetedit.h

SOURCES += \
    $$PWD/quausertable.cpp \
    $$PWD/quauserwidgetedit.cpp \
    $$PWD/quaaccommondialog.cpp  \
    $$PWD/quaaccommonwidgets.cpp \  
    $$PWD/quaroletable.cpp \
    $$PWD/quarolewidgetedit.cpp \
    $$PWD/quapermissionstable.cpp \
    $$PWD/quapermissionswidgetedit.cpp

FORMS += \
    $$PWD/quausertable.ui \
    $$PWD/quauserwidgetedit.ui \
    $$PWD/quaaccommondialog.ui \ 
    $$PWD/quaroletable.ui \
    $$PWD/quarolewidgetedit.ui \
    $$PWD/quapermissionstable.ui \
    $$PWD/quapermissionswidgetedit.ui