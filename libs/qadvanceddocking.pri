#CONFIG += c++14

LIBS += -L$$PWD/QAdvancedDocking.git/lib

# shared
CONFIG(debug, debug|release) {
    win32 {
        LIBS += -lqtadvanceddockingd
    }
    mac {
        LIBS += -lqtadvanceddocking_debug
    }
    linux-g++ {
        LIBS += -lqtadvanceddocking
    }
} else {
    LIBS += -lqtadvanceddocking
}

INCLUDEPATH += $$PWD/QAdvancedDocking.git/src
DEPENDPATH  += $$PWD/QAdvancedDocking.git/src