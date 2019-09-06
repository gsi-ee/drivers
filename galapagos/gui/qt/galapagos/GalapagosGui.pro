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


SOURCES += main.cpp  GalapagosObjects.cpp GalapagosSetup.cpp GalapagosGui.cpp  GalPackageEditor.cpp GalPackageWidget.cpp GalKernelEditor.cpp GalKernelWidget.cpp GalPatternEditor.cpp GalPatternWidget.cpp 
HEADERS += GalapagosGui.h  GalapagosObjects.h GalapagosSetup.h GalPackageEditor.h GalPackageWidget.h GalKernelEditor.h GalKernelWidget.h  GalPatternEditor.h GalPatternWidget.h


FORMS = GalPackageEditor.ui GalKernelEditor.ui GalPatternEditor.ui
