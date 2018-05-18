QT += core gui network qml widgets concurrent

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

# CONFIG += openmp ## removed open by jfm 5/18/2018
CONFIG += mlcommon mvcommon taskprogress

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = qt-mountainview
TEMPLATE = app

RESOURCES += mountainview.qrc

INCLUDEPATH += msv/plugins msv/views
VPATH += msv/plugins msv/views

HEADERS += clusterdetailplugin.h clusterdetailview.h \
    clusterdetailviewpropertiesdialog.h \
    msv/views/matrixview.h \
    msv/views/isolationmatrixview.h \
    msv/plugins/isolationmatrixplugin.h \
    msv/views/clustermetricsview.h msv/views/clusterpairmetricsview.h \
    msv/plugins/clustermetricsplugin.h \
    msv/views/curationprogramview.h \
    msv/plugins/curationprogramplugin.h \
    msv/views/curationprogramcontroller.h \
    views/mvgridview.h \
    views/mvgridviewpropertiesdialog.h \
    controlwidgets/createtimeseriesdialog.h
SOURCES += clusterdetailplugin.cpp clusterdetailview.cpp \
    clusterdetailviewpropertiesdialog.cpp \
    msv/views/matrixview.cpp \
    msv/views/isolationmatrixview.cpp \
    msv/plugins/isolationmatrixplugin.cpp \
    msv/views/clustermetricsview.cpp msv/views/clusterpairmetricsview.cpp \
    msv/plugins/clustermetricsplugin.cpp \
    msv/views/curationprogramview.cpp \
    msv/plugins/curationprogramplugin.cpp \
    msv/views/curationprogramcontroller.cpp \
    views/mvgridview.cpp \
    views/mvgridviewpropertiesdialog.cpp \
    controlwidgets/createtimeseriesdialog.cpp
FORMS += clusterdetailviewpropertiesdialog.ui \
    controlwidgets/createtimeseriesdialog.ui

#INCLUDEPATH += ../../prv-gui/src
#HEADERS += ../../prv-gui/src/prvgui.h
#SOURCES += ../../prv-gui/src/prvgui.cpp

HEADERS += clipsviewplugin.h mvclipsview.h
SOURCES += clipsviewplugin.cpp mvclipsview.cpp

HEADERS += clustercontextmenuplugin.h
SOURCES += clustercontextmenuplugin.cpp


SOURCES += mountainviewmain.cpp \
    #guides/individualmergedecisionpage.cpp \
    views/mvspikespraypanel.cpp \
    views/firetrackview.cpp \
    views/ftelectrodearrayview.cpp \
    controlwidgets/mvmergecontrol.cpp \
    controlwidgets/mvprefscontrol.cpp \
    views/mvpanelwidget.cpp views/mvpanelwidget2.cpp \
    views/mvtemplatesview2.cpp \
    views/mvtemplatesview3.cpp \
    views/mvtemplatesview2panel.cpp \


HEADERS += mvcontext.h
SOURCES += mvcontext.cpp

INCLUDEPATH += multiscaletimeseries
VPATH += multiscaletimeseries
HEADERS += multiscaletimeseries.h
SOURCES += multiscaletimeseries.cpp

HEADERS += \
    #guides/individualmergedecisionpage.h \
    views/mvspikespraypanel.h \
    views/firetrackview.h \
    views/ftelectrodearrayview.h \
    controlwidgets/mvmergecontrol.h \
    controlwidgets/mvprefscontrol.h \
    views/mvpanelwidget.h views/mvpanelwidget2.h \
    views/mvtemplatesview2.h \
    views/mvtemplatesview3.h \
    views/mvtemplatesview2panel.h \


# to remove
#HEADERS += computationthread.h
#SOURCES += computationthread.cpp

#HEADERS += guides/guidev1.h guides/guidev2.h
#SOURCES += guides/guidev1.cpp guides/guidev2.cpp

INCLUDEPATH += views
VPATH += views
HEADERS += \
correlationmatrixview.h histogramview.h mvamphistview2.h mvamphistview3.h histogramlayer.h \
mvclipswidget.h \
mvclusterview.h mvclusterwidget.h mvcrosscorrelogramswidget3.h \
mvdiscrimhistview.h mvfiringeventview2.h mvhistogramgrid.h \
mvspikesprayview.h mvtimeseriesrendermanager.h mvtimeseriesview2.h \
mvtimeseriesviewbase.h spikespywidget.h \
#mvdiscrimhistview_guide.h \
mvclusterlegend.h
SOURCES += \
correlationmatrixview.cpp histogramview.cpp mvamphistview2.cpp mvamphistview3.cpp histogramlayer.cpp \
mvclipswidget.cpp \
mvclusterview.cpp mvclusterwidget.cpp mvcrosscorrelogramswidget3.cpp \
mvdiscrimhistview.cpp mvfiringeventview2.cpp mvhistogramgrid.cpp \
mvspikesprayview.cpp mvtimeseriesrendermanager.cpp mvtimeseriesview2.cpp \
mvtimeseriesviewbase.cpp spikespywidget.cpp \
#mvdiscrimhistview_guide.cpp \
mvclusterlegend.cpp

INCLUDEPATH += controlwidgets
VPATH += controlwidgets
HEADERS += \
mvclustervisibilitycontrol.h \
mvgeneralcontrol.h mvtimeseriescontrol.h mvopenviewscontrol.h mvclusterordercontrol.h
SOURCES += \
mvclustervisibilitycontrol.cpp \
mvgeneralcontrol.cpp mvtimeseriescontrol.cpp mvopenviewscontrol.cpp mvclusterordercontrol.cpp

INCLUDEPATH += controlwidgets/mvexportcontrol
VPATH += controlwidgets/mvexportcontrol
HEADERS += mvexportcontrol.h
SOURCES += mvexportcontrol.cpp

#INCLUDEPATH += guides
#VPATH += guides
#HEADERS += clustercurationguide.h
#SOURCES += clustercurationguide.cpp

## TODO: REMOVE THIS:
HEADERS += computationthread.h
SOURCES += computationthread.cpp

INCLUDEPATH += msv/contextmenuhandlers
VPATH += msv/contextmenuhandlers
HEADERS += clustercontextmenuhandler.h clusterpaircontextmenuhandler.h
SOURCES += clustercontextmenuhandler.cpp clusterpaircontextmenuhandler.cpp

INCLUDEPATH += utils
DEPENDPATH += utils
VPATH += utils
HEADERS += msmisc.h
SOURCES += msmisc.cpp
HEADERS += get_pca_features.h get_principal_components.h eigenvalue_decomposition.h
SOURCES += get_pca_features.cpp get_principal_components.cpp eigenvalue_decomposition.cpp
HEADERS += affinetransformation.h
SOURCES += affinetransformation.cpp
HEADERS += compute_templates_0.h
SOURCES += compute_templates_0.cpp
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp


#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

FORMS += \
    views/mvgridviewpropertiesdialog.ui

DISTFILES += \
    msv/views/curationprogram.js


include(../../../cpp/installbin.pri)
