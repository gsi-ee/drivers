TEMPLATE	= app
LANGUAGE	= C++

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread



SOURCES += main.cpp PolandGui.cpp

HEADERS += PolandGui.h 

FORMS = PolandGui.ui
