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

SOURCES += main.cpp AwagsGui.cpp AwagsWidget.cpp AwagsGuiSlots.cpp AwagsGuiIO.cpp AwagsSetup.cpp AwagsTest.cpp GainSetup.cpp BoardSetup.cpp AwagsTestResults.cpp AdcSample.cpp DacWorkCurve.cpp

HEADERS += AwagsGui.h AwagsWidget.h AwagsSetup.h AwagsTest.h GainSetup.h BoardSetup.h AwagsTestResults.h AdcSample.h DacWorkCurve.h


FORMS = AwagsWidget.ui 
