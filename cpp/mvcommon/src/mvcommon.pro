QT = core network
QT += widgets
QT += qml

CONFIG += mlcommon taskprogress

CONFIG += c++11
CONFIG += staticlib

DESTDIR = ../lib
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mvcommon
TEMPLATE = lib

INCLUDEPATH += ../include
VPATH += ../include
HEADERS += prvmanagerdialog.h resolveprvsdialog.h \
    ../include/toolbox.h
SOURCES += prvmanagerdialog.cpp resolveprvsdialog.cpp \
    toolbox.cpp

FORMS += prvmanagerdialog.ui resolveprvsdialog.ui

INCLUDEPATH += ../include/misc
VPATH += misc ../include/misc
HEADERS += \
clustermerge.h \
mvmisc.h mvutils.h paintlayer.h paintlayerstack.h renderablewidget.h jscounter.h
SOURCES += \
clustermerge.cpp \
mvmisc.cpp mvutils.cpp paintlayer.cpp paintlayerstack.cpp renderablewidget.cpp jscounter.cpp

INCLUDEPATH += ../include/core
VPATH += core ../include/core
HEADERS += \
closemehandler.h flowlayout.h imagesavedialog.h \
mountainprocessrunner.h mvabstractcontextmenuhandler.h \
mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
mvcontrolpanel2.h mvstatusbar.h \
tabber.h tabberframe.h taskprogressview.h actionfactory.h mvabstractplugin.h \
mvabstractcontext.h mvmainwindow.h

SOURCES += \
closemehandler.cpp flowlayout.cpp imagesavedialog.cpp \
mountainprocessrunner.cpp mvabstractcontextmenuhandler.cpp \
mvabstractcontrol.cpp mvabstractview.cpp mvabstractviewfactory.cpp \
mvcontrolpanel2.cpp mvstatusbar.cpp \
tabber.cpp tabberframe.cpp taskprogressview.cpp actionfactory.cpp mvabstractplugin.cpp \
mvabstractcontext.cpp mvmainwindow.cpp

DISTFILES += \
    ../mvcommon.pri

RESOURCES += mvcommon.qrc
