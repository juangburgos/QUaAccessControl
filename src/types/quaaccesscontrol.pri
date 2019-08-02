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
    $$PWD/quaaccesscontrol.h \
    $$PWD/quafolderobjectprotected.h \
    $$PWD/quabaseobjectprotected.h

SOURCES += \
    $$PWD/quauserlist.cpp \
    $$PWD/quauser.cpp \
    $$PWD/quarolelist.cpp \
    $$PWD/quarole.cpp \
    $$PWD/quapermissionslist.cpp \
    $$PWD/quapermissions.cpp \
    $$PWD/quaaccesscontrol.cpp \
    $$PWD/quafolderobjectprotected.cpp \
    $$PWD/quabaseobjectprotected.cpp

DEFINES += QUA_ACCESS_CONTROL