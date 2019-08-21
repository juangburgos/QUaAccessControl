QT += core gui

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quaacdocking.h \
    $$PWD/qaddockwidgetwrapper.h \
    $$PWD/qaddockwidgetconfig.h \
    $$PWD/quadockwidgetperms.h \
    $$PWD/qaddocklayoutbar.h

SOURCES += \
    $$PWD/quaacdocking.cpp \
    $$PWD/qaddockwidgetwrapper.cpp \
    $$PWD/qaddockwidgetconfig.cpp \
    $$PWD/quadockwidgetperms.cpp \
    $$PWD/qaddocklayoutbar.cpp \

FORMS += \
    $$PWD/qaddockwidgetwrapper.ui \
    $$PWD/qaddockwidgetconfig.ui \
    $$PWD/quadockwidgetperms.ui \
    $$PWD/qaddocklayoutbar.ui

HEADERS += \
    $$PWD/quaacdockwidgets.hpp

