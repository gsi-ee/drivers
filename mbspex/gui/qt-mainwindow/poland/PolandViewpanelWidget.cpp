#include "PolandViewpanelWidget.h"
#include "PolandSetup.h"

#include "GosipGui.h"


#include <kplotobject.h>
#include <kplotwidget.h>
#include <kplotaxis.h>
#include <kplotpoint.h>

#include <iostream>
// *********************************************************

/*
 *  Constructs a PolandViewpanelWidget to be inserted to the framwework gui
 *  name 'name'.'
 */
PolandViewpanelWidget::PolandViewpanelWidget (QWidget* parent) :
    QWidget (parent), fPlot(0),fPickCounter(0),
	fLowLimit(0), fHighLimit(1), fNminLimit(0), fNmaxLimit(1), fDisplayChannel(POLAND_DAC_NUM), fDisplayLoop(POLAND_QFWLOOPS)

{
  setupUi (this);

  Channel_comboBox->clear();
  for(int c=0; c<POLAND_DAC_NUM; ++c)
  {
	  Channel_comboBox->addItem(QString("Ch %1").arg(c));
  }
  Channel_comboBox->addItem("All Channels");
  Channel_comboBox->setCurrentIndex (POLAND_DAC_NUM);
  Loops_comboBox->clear();
   for(int l=0; l<POLAND_QFWLOOPS; ++l)
   {
	   Loops_comboBox->addItem(QString("Loop %1").arg(l));
   }
   Loops_comboBox->addItem("All Loops");
   Loops_comboBox->setCurrentIndex (POLAND_QFWLOOPS);
  ConnectSlots();
}

PolandViewpanelWidget::~PolandViewpanelWidget ()
{
}



void PolandViewpanelWidget::ConnectSlots()
{

  QObject::connect (UnzoomButton, SIGNAL(clicked()), this, SLOT(UnzoomButton_clicked()));
  QObject::connect (SetZoomButton,SIGNAL(clicked(bool)), this, SLOT(SetZoomButton_toggled(bool)));
  QObject::connect (PatternLow_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternLow_spinBox_changed(int)));
  QObject::connect (PatternHi_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternHi_spinBox_changed(int)));

  QObject::connect (Channel_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Channel_changed(int)));
  QObject::connect (Loops_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Loop_changed(int)));

}


void PolandViewpanelWidget::mousePressEvent (QMouseEvent *event)
{
//std::cout <<" PolandViewpanelWidget::mousePressEvent"  <<std::endl;
if (event->button () == Qt::LeftButton)
{

  if (fPickCounter > 0)
  {

    //std::cout <<" Left button at position x:"<< event->x()<<", y:"<<event->y()  <<std::endl;
    // following correction for the plot border was stolen from a protected method in KPlotWidget:
    QPoint pickpoint = event->pos () - QPoint (PlotwidgetChSlice->leftPadding (), PlotwidgetChSlice->topPadding ())
        - PlotwidgetChSlice->contentsRect ().topLeft ();

    QList<KPlotObject *> objects = PlotwidgetChSlice->plotObjects ();
    if (!objects.isEmpty ())
    {

      KPlotObject * thePlot = objects.first ();
      QList<KPlotPoint *> pointlist = thePlot->points ();
      double olddelta = 500;
      double delta = 0;
      int cursor = 0;
      for (int i = 0; i < pointlist.size (); ++i)
      {
        KPlotPoint* pt = pointlist[i];
        delta = ::fabs ((double) pickpoint.x () - (double) PlotwidgetChSlice->mapToWidget (pt->position ()).x ());
        if (delta <= 100)
        {
          //std::cout<< "Found near plot point "<<i <<", delta="<< delta<< " with coordinates x:"<<pt->x()<<", y:"<<pt->y() << std::endl;

          // here leave pick procedure
          if (delta > olddelta)
            break;
          olddelta = delta;
          cursor = pt->x ();
        }

      }    // for

      //std::cout << "Found final pick point at delta=" << delta << " with cursor x:" << cursor << std::endl;
      if(fPickCounter==2)
      {
        fLowLimit=cursor;
      }
      else if (fPickCounter==1)
      {
        fHighLimit=cursor;
        SetZoomButton->setChecked(false);
        RefreshView();
      }
      else {
        printm(" PolandViewpanelWidget::mousePressEvent NEVER COME HERE: piccounter mismatch : %d",fPickCounter);
      }

      fPickCounter--;

    }

  }    // fPickCounter

}


    QWidget::mousePressEvent(event);

}






void PolandViewpanelWidget::UnzoomButton_clicked()
{
  //std::cout <<" PolandViewpanelWidget::UnzoomButton_clicked"  <<std::endl;
  fLowLimit=0;
  fHighLimit= fPlot ? fPlot->points().size() : 0;
  RefreshView();


}

void PolandViewpanelWidget::SetZoomButton_toggled(bool on)
{
  //std::cout <<" PolandViewpanelWidget::SetZoomButton_toggled "<< on  <<std::endl;
  fPickCounter= on ? 2 : 0;
}

void PolandViewpanelWidget::PatternLow_spinBox_changed(int val)
{
  //std::cout <<" PolandViewpanelWidget::PatternLow_spinBox_changed "<< val  <<std::endl;
  fLowLimit=val;
  RefreshView();
}

void PolandViewpanelWidget::PatternHi_spinBox_changed(int val)
{
  //std::cout <<" PolandViewpanelWidget::PatternHi_spinBox_changed "<< val  <<std::endl;
  fHighLimit=val;
  RefreshView();
}

void PolandViewpanelWidget::Channel_changed(int val)
{
	//std::cout <<" PolandViewpanelWidget::Channel_changed "<< val  <<std::endl;
	fDisplayChannel=val;
	ShowSample(0);
}


void PolandViewpanelWidget::Loop_changed(int val)
{
	//std::cout <<" PolandViewpanelWidget::Loop_changed "<< val  <<std::endl;
	fDisplayLoop=val;
	ShowSample(0);
}


void PolandViewpanelWidget::RefreshView ()
{
  GOSIP_LOCK_SLOT
  PatternLow_spinBox->setValue(fLowLimit);
  PatternHi_spinBox->setValue(fHighLimit);

  PlotwidgetChSlice->setLimits (fLowLimit, fHighLimit, fNminLimit, fNmaxLimit);
  PlotwidgetChSlice->update ();
  GOSIP_UNLOCK_SLOT
}


void PolandViewpanelWidget::ShowSample (PolandSample* sample)
{
  //std::cout <<"ShowSample "<< std::endl;
  if(sample!=0) theSample=sample;
  if(theSample==0) return;

  KPlotWidget* canvas = PlotwidgetChSlice;
  // first fill plotobject with samplepoints
  QColor col;
  KPlotObject::PointStyle pstyle = KPlotObject::Circle;

  col = Qt::red;
       pstyle = KPlotObject::Circle;

//  switch (channel)
//  {
//    case 0:
//    case 8:
//    default:
//      col = Qt::red;
//      pstyle = KPlotObject::Circle;
//      break;
//    case 1:
//    case 9:
//      col = Qt::green;
//      pstyle = KPlotObject::Letter;
//      break;
//    case 2:
//    case 10:
//      col = Qt::blue;
//      pstyle = KPlotObject::Triangle;
//      break;
//    case 3:
//    case 11:
//      col = Qt::cyan;
//      pstyle = KPlotObject::Square;
//      break;
//
//    case 4:
//    case 12:
//      col = Qt::magenta;
//      pstyle = KPlotObject::Pentagon;
//      break;
//    case 5:
//    case 13:
//      col = Qt::yellow;
//      pstyle = KPlotObject::Hexagon;
//      break;
//    case 6:
//    case 14:
//      col = Qt::gray;
//      pstyle = KPlotObject::Asterisk;
//      break;
//    case 7:
//    case 15:
//      col = Qt::darkGreen;
//      pstyle = KPlotObject::Star;
//      break;
//
////        Letter = 2, Triangle = 3,
////         Square = 4, Pentagon = 5, Hexagon = 6, Asterisk = 7,
////         Star = 8
//
//  };

       int numberbase=GosipGui::fInstance->GetNumberBase();
       EventCounter->setMode((numberbase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
       EventCounter->display ((int) theSample->GetEventCounter());


  // TODO: put this in special functions
  canvas->resetPlot ();
  // labels for plot area:
  canvas->setAntialiasing (true);
  canvas->axis (KPlotWidget::BottomAxis)->setLabel ("Trace index (#samples)");
  canvas->axis (KPlotWidget::LeftAxis)->setLabel ("Register value ");

  fPlot = new KPlotObject(col, KPlotObject::Points, 2, pstyle);

  int t=0;
  fNmaxLimit=0;
  for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
     {
	   if((fDisplayLoop<POLAND_QFWLOOPS) && (loop != fDisplayLoop)) continue;
       for (int sl = 0; sl < theSample->GetLoopsize(loop); ++sl)
       {
         for (int ch = 0; ch < POLAND_DAC_NUM; ++ch)
         {
        	 if((fDisplayChannel<POLAND_DAC_NUM) && (ch != fDisplayChannel)) continue;
           int val = theSample->GetTraceValue(loop,ch,sl);
           //printf("ShowSample l:%d sl:%d c:%d val:0x%x\n",loop,sl,ch,val);
           fPlot->addPoint (t++, val);
           if(val >fNmaxLimit)
        	   fNmaxLimit=val;
         }
       }
     }
  fLowLimit=0;
  fHighLimit=t;
  fNmaxLimit+=1;
  fNminLimit=0;
  // add it to the plot area
  canvas->addPlotObject (fPlot);



  if(fNmaxLimit>0x800) fNmaxLimit=0x800;
  RefreshView ();

//  canvas->setLimits (fLowLimit, fHighLimit, 0.0, fNmaxLimit);
//
//  canvas->update ();

  //TODO: Kplotwidget with 2d display of time sliceslike in go4
  // unpacker

}







