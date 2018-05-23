TEMPLATE = subdirs

# usage:
# qmake
# qmake "COMPONENTS = mountainview"

isEmpty(COMPONENTS) {
    COMPONENTS = mvcommon mountainview
}

isEmpty(GUI) {
    GUI = on
}

CONFIG += ordered

defineReplace(ifcomponent) {
  contains(COMPONENTS, $$1) {
    message(Enabling $$1)
    return($$2)
  }
  return("")
}

SUBDIRS += cpp/mlcommon/src/mlcommon.pro
equals(GUI,"on") {
  SUBDIRS += $$ifcomponent(mvcommon,cpp/mvcommon/src/mvcommon.pro)
  SUBDIRS += $$ifcomponent(mountainview,cpp/mountainview/src/mountainview.pro)
}
SUBDIRS += packages/mv/mv.pro

DISTFILES += features/*
DISTFILES += debian/*

deb.target = deb
deb.commands = debuild $(DEBUILD_OPTS) -us -uc

QMAKE_EXTRA_TARGETS += deb

# added 5/21/2018 by tjd to support Qt-included distribution of mv as a mountainlab-js package
standalone:unix{
    macx: {
        # bugs in earlier versions of macdeployqt
        MAJ_REQ = 5
        MIN_REQ = 7
        isEmpty(QT.core.frameworks){
            error("The Qt libs at $$[QT_INSTALL_LIBS] were not built as Mac frameworks, so can't be used with the macdeployqt tool to build standalone Mountainview (conda/brew Qt?). Use the installer from qt.io.")
        }
    } else {
        MAJ_REQ = 5
        MIN_REQ = 5
    }
    lessThan(QT_MAJOR_VERSION, $$MAJ_REQ) | \
    if(equals(QT_MAJOR_VERSION, $$MAJ_REQ):lessThan(QT_MINOR_VERSION, $$MIN_REQ)) {
        error("Building standalone on $${QMAKE_HOST.os} requires Qt $${MAJ_REQ}.$${MIN_REQ} or greater, but Qt $$[QT_VERSION] was detected.")
    }

    # (Linux standalone handled using ml_deployqtlinux feature)

    macx: {
        # Need to do this at top-level since we need to move mv.mp into qt-mountainview app bundle
        QtDeploy.commands += "cp $${ML_PACKAGESDIR}/mv/bin/mv.mp $${ML_BINDIR}/qt-mountainview.app/Contents/MacOS/ ;"
        QtDeploy.commands += "$$[QT_INSTALL_BINS]/macdeployqt $${ML_BINDIR}/qt-mountainview.app -always-overwrite -executable=$$ML_BINDIR/qt-mountainview.app/Contents/MacOS/mv.mp ;"
        QtDeploy.commands += "cd $$ML_BINDIR ; ditto -c -k --sequesterRsrc --keepParent $$ML_BINDIR/qt-mountainview.app qt-mountainview-standalone-$${GIT_HASH}.zip;"
        QtDeploy.path = / # dummy path required for target to be valid

        message(QtDeploy.commands=$$QtDeploy.commands)
        INSTALLS += QtDeploy
    }
}
