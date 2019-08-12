#CONFIG += c++14

# shared
CONFIG(debug, debug|release) {
    win32 {
        LIBS += -L$$PWD/QAdvancedDocking.git/lib
        LIBS += -lqtadvanceddockingd
    }
    mac {
        LIBS += -L$${OUT_PWD}/../../libs/QAdvancedDocking.git/lib
        LIBS += -lqtadvanceddocking_debug
    }
    linux-g++ {
        LIBS += -L$${OUT_PWD}/../../libs/QAdvancedDocking.git/lib
        LIBS += -lqtadvanceddocking
    }
} else {
    LIBS += -lqtadvanceddocking
}

INCLUDEPATH += $$PWD/QAdvancedDocking.git/src
DEPENDPATH  += $$PWD/QAdvancedDocking.git/src
