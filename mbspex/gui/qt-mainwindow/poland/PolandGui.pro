TEMPLATE	= app
LANGUAGE	= C++

DEPENDPATH += . ../framework
INCLUDEPATH += ../framework
LIBS += -L../framework -lGosipGui


greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}


equals(QT_MAJOR_VERSION, 4) {
 	 LIBS += -lkdeui
}
equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
  QT += network
  INCLUDEPATH += /usr/include/KF5/KPlotting/
  LIBS += -lKF5Plotting
}


CONFIG += debug qt warn_off thread



SOURCES += main.cpp PolandGui.cpp PolandWidget.cpp PolandViewpanelWidget.cpp PolandCSAWidget.cpp

HEADERS += PolandGui.h PolandWidget.h PolandViewpanelWidget.h PolandCSAWidget.h PolandSetup.h 

FORMS = PolandWidget.ui PolandViewpanelWidget.ui PolandCSAWidget.ui
