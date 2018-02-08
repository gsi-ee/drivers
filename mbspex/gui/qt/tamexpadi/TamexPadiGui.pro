TEMPLATE	= app
LANGUAGE	= C++

DEPENDPATH += . ../framework
INCLUDEPATH += ../framework
LIBS += -L../framework -lGosipGui


greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread

SOURCES += main.cpp TamexPadiGui.cpp TamexPadiWidget.cpp

HEADERS += TamexPadiGui.h TamexPadiWidget.h TamexPadiSetup.h


FORMS = TamexPadiWidget.ui 
