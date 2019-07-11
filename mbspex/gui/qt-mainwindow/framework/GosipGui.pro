TEMPLATE	= lib
LANGUAGE	= C++

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread
CONFIG += staticlib

RESOURCES += ../gosipicons.qrc


#SOURCES += main.cpp GosipGui.cpp
SOURCES += GosipGui.cpp 

HEADERS += GosipGui.h 

#FORMS = GosipGui.ui
FORMS = GosipMainwindow.ui

