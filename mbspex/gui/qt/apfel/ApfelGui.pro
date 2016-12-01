TEMPLATE	= app
LANGUAGE	= C++

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG += debug qt warn_off thread

SOURCES += main.cpp ApfelGui.cpp ApfelGuiSlots.cpp ApfelGuiIO.cpp ApfelSetup.cpp ApfelTest.cpp GainSetup.cpp BoardSetup.cpp ApfelTestResults.cpp AdcSample.cpp DacWorkCurve.cpp

HEADERS += ApfelGui.h ApfelSetup.h ApfelTest.h GainSetup.h BoardSetup.h ApfelTestResults.h AdcSample.h DacWorkCurve.h


FORMS = ApfelGui.ui 
