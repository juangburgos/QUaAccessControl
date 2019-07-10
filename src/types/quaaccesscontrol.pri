QT += core network xml

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quauserlist.h \
    $$PWD/quauser.h \
    $$PWD/quarolelist.h \
    $$PWD/quarole.h \
    $$PWD/quapermissionslist.h \
    $$PWD/quapermissions.h \
    $$PWD/quaaccesscontrol.h

SOURCES += \
    $$PWD/quauserlist.cpp \
    $$PWD/quauser.cpp \
    $$PWD/quarolelist.cpp \
    $$PWD/quarole.cpp \
    $$PWD/quapermissionslist.cpp \
    $$PWD/quapermissions.cpp \
    $$PWD/quaaccesscontrol.cpp
