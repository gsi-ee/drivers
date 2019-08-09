TEMPLATE	= lib
LANGUAGE	= C++

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread
CONFIG += staticlib

RESOURCES += ../galapicons.qrc


#SOURCES += main.cpp BasicGui.cpp
SOURCES += BasicGui.cpp 

HEADERS += BasicGui.h 

FORMS = BasicGui.ui
#FORMS = BasicMainwindow.ui

