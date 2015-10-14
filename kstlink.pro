#-------------------------------------------------
#
# Project created by QtCreator 2013-01-10T17:36:44
#
#-------------------------------------------------

QT       += core network

QT       += gui

TARGET = kstlink
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#DBUS_ADAPTORS += kstlink_dbus.xml
DBUS_INTERFACES += kstlink_dbus.xml

include (gui/gui.pro)

SOURCES += main.cpp \
    qlibusb.cpp \
    qlog.cpp \
    qstlink.cpp \
    cm3/stm100.cpp \
    gdbserver.cpp \
    flasher.cpp \
    chips.cpp \
    cm3/stm407.cpp \
    kelnet.cpp \
    qstlinkadaptor.cpp \
    cm3/stmabstract.cpp \
    cm3debugregs.cpp

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
    #progressbar.h \
    kelnet.h \
    qstlinkadaptor.h \
    cm3/stmabstract.h \
    cm3debugregs.h

INCLUDEPATH += cm3

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    kstlink_dbus.xml


unix {
    #CONFIG += qdbus
    #DEFINES += KSTLINK_DBUS
    INCLUDEPATH += /usr/include/libusb-1.0
    LIBS += -lusb-1.0
}

win32 {

}
