#ifndef POLANDVIEWPANELWIDGET_H
#define POLANDVIEWPANELWIDGET_H

#include "ui_PolandViewpanelWidget.h"


class PolandSample;

class PolandViewpanelWidget: public QWidget, public Ui::PolandViewpanelWidget
{
  Q_OBJECT






protected:

  KPlotObject *fPlot;

   /** control pick of limits*/
   int fPickCounter;

   /** left pattern boundary*/
   int fLowLimit;

   /** right pattern boundary*/
   int fHighLimit;

   virtual void    mousePressEvent(QMouseEvent *event);

public:
  PolandViewpanelWidget (QWidget* parent = 0);
  virtual ~PolandViewpanelWidget ();


   virtual void ConnectSlots ();

     virtual void RefreshView ();


     /** take values relevant for our widget from Qt settings file*/

     int GetLowLimit(){return fLowLimit;}

     int GetHighLimit(){return fHighLimit;}

     void ShowSample (PolandSample* theSample);


   public slots:

   virtual void UnzoomButton_clicked();
   virtual void SetZoomButton_toggled(bool);
   virtual void PatternLow_spinBox_changed(int);
   virtual void PatternHi_spinBox_changed(int);




};

#endif
