TEMPLATE	= app
LANGUAGE	= C++

DEPENDPATH += . ../framework
INCLUDEPATH += ../framework 
LIBS += -L../framework -lGosipGui 

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

SOURCES += main.cpp ApfelGui.cpp ApfelWidget.cpp ApfelGuiSlots.cpp ApfelGuiIO.cpp ApfelSetup.cpp ApfelTest.cpp GainSetup.cpp BoardSetup.cpp ApfelTestResults.cpp AdcSample.cpp DacWorkCurve.cpp

HEADERS += ApfelGui.h ApfelWidget.h ApfelSetup.h ApfelTest.h GainSetup.h BoardSetup.h ApfelTestResults.h AdcSample.h DacWorkCurve.h


FORMS = ApfelWidget.ui 
