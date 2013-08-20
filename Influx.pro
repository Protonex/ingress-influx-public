#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T11:38:21
#
#-------------------------------------------------

QT       += core gui network webkit webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

PRECOMPILED_HEADER = pch.h

TARGET = Influx
TEMPLATE = app


SOURCES += main.cpp\
    dlglogin.cpp \
    api.cpp \
	influxwnd.cpp \
    datacfg.cpp \
    deviceinfo.cpp

HEADERS  += \
    pch.h \
    dlglogin.h \
    api.h \
	influxwnd.h \
    datacfg.h \
    SlippyMapWidget.h \
    deviceinfo.h

FORMS    += mainwindow.ui

OTHER_FILES +=

include(logger/logger.pri)
include(util/util.pri)
include(zlib/zlib.pri)
include(network/network.pri)
include(MapGraphics/MapGraphics.pri)
#include(s2-geometry/S2Geometry.pri)
include(decompiler/decompiler.pri)

INCLUDEPATH += $$PWD/../MapGraphics
DEPENDPATH += $$PWD/../MapGraphics


