TEMPLATE	= lib
LANGUAGE	= C++

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread
CONFIG += staticlib

RESOURCES += ../galapicons.qrc


#SOURCES += main.cpp BasicGui.cpp
SOURCES += BasicSubWidget.cpp BasicObjectEditorWidget.cpp BasicGui.cpp 

HEADERS += BasicObject.h BasicObjectManager.h BasicSubWidget.h BasicObjectEditorWidget.h BasicGui.h 

FORMS = BasicObjectEditorWidget.ui BasicGui.ui

