QT += core gui

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quaacdocking.h \
    $$PWD/qaddockwidgetwrapper.h \
    $$PWD/quadockwidgetperms.h \
    $$PWD/qaddocklayoutbar.h

SOURCES += \
    $$PWD/quaacdocking.cpp \
    $$PWD/qaddockwidgetwrapper.cpp \
    $$PWD/quadockwidgetperms.cpp \
    $$PWD/qaddocklayoutbar.cpp \

FORMS += \
    $$PWD/qaddockwidgetwrapper.ui \
    $$PWD/quadockwidgetperms.ui \
    $$PWD/qaddocklayoutbar.ui

HEADERS += \
    $$PWD/quaacdockwidgets.hpp

