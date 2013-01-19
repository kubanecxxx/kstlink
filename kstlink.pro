#-------------------------------------------------
#
# Project created by QtCreator 2013-01-10T17:36:44
#
#-------------------------------------------------

QT       += core network gui

#QT       -= gui

TARGET = kstlink
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    qlibusb.cpp \
    qlog.cpp \
    qstlink.cpp \
    cm3/stm100.cpp \
    gdbserver.cpp \
    flasher.cpp \
    chips.cpp \
    cm3/stm407.cpp \
    progressbar.cpp

HEADERS += qlibusb.h \
    qlog.h \
    include.h \
    qstlink.h \
    stlinkCommands.h \
    armConstants.h \
    cm3/stm100.h \
    gdbserver.h \
    flasher.h \
    chips.h \
    cm3/stm407.h \
    progressbar.h

INCLUDEPATH += cm3

unix:INCLUDEPATH += /usr/include/libusb-1.0
unix:LIBS += -lusb-1.0

RESOURCES += \
    resources.qrc

FORMS += \
    progressbar.ui
