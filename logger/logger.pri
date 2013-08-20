INCLUDEPATH += $$PWD
#DEFINES += QS_LOG_LINE_NUMBERS
SOURCES += $$PWD/QsLogDest.cpp \
    $$PWD/QsLog.cpp \
    $$PWD/QsDebugOutput.cpp \
    logger/loggerwnd.cpp

HEADERS += $$PWD/QSLogDest.h \
    $$PWD/QsLog.h \
    $$PWD/QsDebugOutput.h \
    logger/loggerwnd.h

FORMS += \
    logger/loggerwnd.ui