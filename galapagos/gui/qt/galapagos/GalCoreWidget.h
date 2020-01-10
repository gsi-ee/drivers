#ifndef GAPG_GALCOREWIDGET_H
#define GAPG_GALCOREWIDGET_H


#include <QWidget>



#include "GalapagosObjects.h"
#include "ui_GalCoreWidget.h"



namespace gapg{

class GalPackageWidget;

class GalCoreWidget: public QWidget, public Ui::GalCoreWidget
{
  Q_OBJECT

protected:

  /** current index of the kernel*/
  int fIndex;

  /** our controlling package editor widget*/
  GalPackageWidget* fPackageWidget;


public:
 GalCoreWidget (int index, GalPackageWidget* master, QWidget* parent = 0);
  virtual ~GalCoreWidget ();


  void RefreshCoreType(gapg::CoreType_t type);



  public slots:

 virtual void CoreEnabled_toggled(bool on);
 virtual void CoreKernel_changed (int ix);


};

} // gapg

#endif
