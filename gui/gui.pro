#-------------------------------------------------
#
# Project created by QtCreator 2013-09-01T10:12:58
#
#-------------------------------------------------

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
VPATH += $$PWD

#include (gui.pro)
#include (tray.pro)
include (treePages/widgetTreePages.pro)

HEADERS += \
    communication.h \
    ktray.h

SOURCES += \
    communication.cpp \
    mainwindow.cpp \
    info.cpp \
    flash.cpp \
    ktray.cpp

HEADERS  += \
    mainwindow.h \
    info.h \
    flash.h

FORMS += \
    mainwindow.ui \
    info.ui \
    flash.ui


SOURCES +=  \
    bar.cpp

HEADERS  += \
    bar.h

RESOURCES += \
    resourcesGUI.qrc

FORMS += \
    bar.ui
