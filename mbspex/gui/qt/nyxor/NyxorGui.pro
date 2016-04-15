TEMPLATE	= app
LANGUAGE	= C++

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread

SOURCES += main.cpp NyxorGui.cpp nxyterwidget.cpp generalwidget.cpp dacwidget.cpp adcwidget.cpp NxContext.cxx NxI2c.cxx

HEADERS += NyxorGui.h  nxyterwidget.h generalwidget.h dacwidget.h adcwidget.h NxContext.h NxI2c.h


FORMS = NyxorGui.ui nxyterwidget.ui generalwidget.ui dacwidget.ui adcwidget.ui
