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
   INCLUDEPATH += /usr/include/okteta/
  LIBS += -lKF5Plotting -lokteta2core -lokteta2gui
}



CONFIG += debug qt warn_off thread


SOURCES += main.cpp  GalapagosSetup.cpp GalapagosGui.cpp  GalChannelWidget.cpp GalSequenceEditor.cpp GalSequenceWidget.cpp GalPatternEditor.cpp GalPatternWidget.cpp 
HEADERS += GalapagosGui.h GalChannelWidget.h GalSequenceEditor.h GalSequenceWidget.h GalapagosObjects.h GalapagosSetup.h GalPatternEditor.h GalPatternWidget.h


FORMS = GalChannelWidget.ui GalSequenceEditor.ui GalPatternEditor.ui
