#ifndef GAPG_GALPATTERNDISPLAY_H
#define GAPG_GALPATTERNDISPLAY_H

#include "BasicSubWidget.h"
#include "ui_GalPatternDisplay.h"

#include <QByteArray>
#include <QString>

class KPlotObject;

namespace gapg
{

class GalPatternDisplay: public BasicSubWidget , public Ui::GalPatternDisplay
{
  Q_OBJECT

protected:

  QString fPatternName;

  KPlotObject *fPlot;

  /** control pick of limits*/
  int fPickCounter;

  /** left pattern boundary*/
  int fLowLimit;

  /** right pattern boundary*/
  int fHighLimit;

  virtual void    mousePressEvent(QMouseEvent *event);

public:
  GalPatternDisplay (QWidget* parent = 0);
  virtual ~GalPatternDisplay ();

  virtual void ConnectSlots ();

  virtual void RefreshView ();

  virtual void EvaluateView ();

  /** take values relevant for our widget from Qt settings file*/
  virtual void ReadSettings (QSettings* set);

  /** put values relevant for our widget from Qt settings file*/
  virtual void WriteSettings (QSettings* set);

  int GetLowLimit(){return fLowLimit;}

  int GetHighLimit(){return fHighLimit;}

  QString& GetPatternName (){return fPatternName;}



public slots:

virtual void UnzoomButton_clicked();
virtual void SetZoomButton_toggled(bool);
virtual void PatternLow_spinBox_changed(int);
virtual void PatternHi_spinBox_changed(int);



virtual void PlotPattern(QByteArray& pat, const QString& name);

};

}    // gapg

#endif
