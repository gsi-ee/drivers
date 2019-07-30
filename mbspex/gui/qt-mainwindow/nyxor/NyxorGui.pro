TEMPLATE	= app
LANGUAGE	= C++

DEPENDPATH += . ../framework
INCLUDEPATH += ../framework
LIBS += -L../framework -lGosipGui


greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread

SOURCES += main.cpp NyxorGui.cpp NyxorWidget.cpp nxyterwidget.cpp generalwidget.cpp dacwidget.cpp adcwidget.cpp NxContext.cxx NxI2c.cxx

HEADERS += NyxorGui.h  NyxorWidget.h nxyterwidget.h generalwidget.h dacwidget.h adcwidget.h NxContext.h NxI2c.h


FORMS = NyxorWidget.ui nxyterwidget.ui generalwidget.ui dacwidget.ui adcwidget.ui
