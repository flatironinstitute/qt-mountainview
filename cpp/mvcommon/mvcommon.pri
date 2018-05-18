INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/core
INCLUDEPATH += $$PWD/include/misc
INCLUDEPATH += $$PWD/3rdparty/qaccordion/include
LIB0 = $$PWD/lib/libmvcommon.a
LIBS += -L$$PWD/lib $$LIB0
unix:PRE_TARGETDEPS += $$LIB0

RESOURCES += $$PWD/src/mvcommon.qrc \
    $$PWD/3rdparty/qaccordion/icons/qaccordionicons.qrc
