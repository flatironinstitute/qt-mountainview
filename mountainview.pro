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
