#ifndef POLANDVIEWPANELWIDGET_H
#define POLANDVIEWPANELWIDGET_H

#include "ui_PolandViewpanelWidget.h"

#include <kplotobject.h>

class PolandSample;

class PolandViewpanelWidget: public QWidget, public Ui::PolandViewpanelWidget
{
  Q_OBJECT






protected:

  KPlotObject *fPlot;

  /** remember current sample for redisplay of channels etc.*/
  PolandSample* theSample;

   /** control pick of limits*/
   int fPickCounter;

   /** left pattern boundary*/
   int fLowLimit;

   /** right pattern boundary*/
   int fHighLimit;

   /** lower y display boundary*/
   int fNminLimit;

   /** upper y display boundary*/
   int fNmaxLimit;

   /** current channel to display*/
   int fDisplayChannel;

   /** current loop to display*/
   int fDisplayLoop;

   /** if true, plot channel sum over traces vs channel number. otherwise different loop trace modes*/
   bool fChannelSumMode;

   /** color code of plot point*/
   int fPlotColorcode;

   /** size factor for plot*/
   int fPlotSize;

   /** style index for plot*/
   int fPlotStyle;


   virtual void    mousePressEvent(QMouseEvent *event);


   /** translate color index into Qt code*/
   QColor mapColorIndex(int i);

   /** translate style index into Kplot code*/
   KPlotObject::PointStyle mapPointstyleIndex(int i);


public:
  PolandViewpanelWidget (QWidget* parent = 0);
  virtual ~PolandViewpanelWidget ();


   virtual void ConnectSlots ();

   virtual void RefreshView ();

   virtual void RefreshDrawStyle();

   virtual void RefreshEventCounter();

     /** take values relevant for our widget from Qt settings file*/

     int GetLowLimit(){return fLowLimit;}

     int GetHighLimit(){return fHighLimit;}

     void ShowSample (PolandSample* sample=0);


   public slots:

   virtual void UnzoomButton_clicked();
   virtual void SetZoomButton_toggled(bool);
   virtual void PatternLow_spinBox_changed(int);
   virtual void PatternHi_spinBox_changed(int);

   virtual void Channel_changed(int);
   virtual void Loop_changed(int);

   virtual void PointColor_changed(int);
   virtual void PointStyle_changed(int);
   virtual void PointSize_changed(int);

   virtual void TracesEnabled_toggled(bool);


};

#endif
