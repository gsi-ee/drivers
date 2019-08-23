TEMPLATE	= app
LANGUAGE	= C++

DEPENDPATH += . ../framework
INCLUDEPATH += ../framework
LIBS += -L../framework -lBasicGui


greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

equals(QT_MAJOR_VERSION, 4) {
 	 LIBS += -lkdeui
}
equals(QT_MAJOR_VERSION, 5) {
  QT += widgets KWidgetsAddons
  INCLUDEPATH += /usr/include/KF5/KPlotting/
   INCLUDEPATH += /usr/include/KF5/KWidgetsAddons/
  LIBS += -lKF5Plotting 
}



CONFIG += debug qt warn_off thread


SOURCES += main.cpp GalapagosGui.cpp  GalapagosGuiSlots.cpp GalChannelWidget.cpp GalSequenceWidget.cpp 
HEADERS += GalapagosGui.h GalChannelWidget.h GalSequenceWidget.h GalapagosSetup.h


FORMS = GalChannelWidget.ui GalSequenceWidget.ui
