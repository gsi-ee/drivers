#include "PolandViewpanelWidget.h"
#include "PolandSetup.h"

#include <kplotobject.h>
#include <kplotwidget.h>
#include <kplotaxis.h>
#include <kplotpoint.h>

// *********************************************************

/*
 *  Constructs a PolandViewpanelWidget to be inserted to the framwework gui
 *  name 'name'.'
 */
PolandViewpanelWidget::PolandViewpanelWidget (QWidget* parent) :
    QWidget (parent), fPlot(0),fPickCounter(0)
{
  setupUi (this);
}

PolandViewpanelWidget::~PolandViewpanelWidget ()
{
}



void PolandViewpanelWidget::ConnectSlots()
{

//  QObject::connect (UnzoomButton, SIGNAL(clicked()), this, SLOT(UnzoomButton_clicked()));
//  QObject::connect (SetZoomButton,SIGNAL(clicked(bool)), this, SLOT(SetZoomButton_toggled(bool)));
//  QObject::connect (PatternLow_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternLow_spinBox_changed(int)));
//  QObject::connect (PatternHi_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternHi_spinBox_changed(int)));


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
        //SetZoomButton->setChecked(false);
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




void PolandViewpanelWidget::RefreshView ()
{
  //GAPG_LOCK_SLOT
//  PatternLow_spinBox->setValue(fLowLimit);
//  PatternHi_spinBox->setValue(fHighLimit);

  PlotwidgetChSlice->setLimits (fLowLimit, fHighLimit, -0.2, 1.2);
  PlotwidgetChSlice->update ();
  //GAPG_UNLOCK_SLOT
}


void PolandViewpanelWidget::ShowSample (PolandSample* theSample)
{
  //std::cout <<"ShowSample for channel:"<<channel<< std::endl;
  //theSetup_GET_FOR_SLAVE(PolandSetup);
  //theSetup->ShowADCSample(channel); // todo: dump sample on different knob

  //KPlotWidget* canvas = fPlotWidget[channel];
//  if (benchmarkdisplay)
//    canvas = fApfelWidget->BenchmarkPlotwidget;
//
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

  // TODO: put this in special functions
  canvas->resetPlot ();
  // labels for plot area:
  canvas->setAntialiasing (true);
  canvas->axis (KPlotWidget::BottomAxis)->setLabel ("Optic data index (#samples)");
  canvas->axis (KPlotWidget::LeftAxis)->setLabel ("Register value ");

  fPlot = new KPlotObject(col, KPlotObject::Points, 1, pstyle);
  //KPlotObject *sampleplot = new KPlotObject (col, KPlotObject::Lines, 2);
  //QString label = QString ("channel:%1").arg (channel);
  //sampleplot->addPoint (0, theSetup->GetADCSample (channel, 0), label);



//  int samplength=theSetup->GetADCSampleLength(channel);
//  for (int i = 1; i < samplength; ++i)
//  {
//    sampleplot->addPoint (i, theSetup->GetADCSample (channel, i));
//  }


  // poor mans solution: just plot values in readout buffer asis
  // later TODO: unpack data and plot only timeslices
  //int numwords = 32 + theSetup->fSteps[0] * 32 + theSetup->fSteps[1] * 32 + theSetup->fSteps[2] * 32;// + 32;
  //snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0 0x%x", fSFP, fSlave, numwords);
//  int numwords = theSetup->fSteps[0] * 32 + theSetup->fSteps[1] * 32 + theSetup->fSteps[2] * 32;// + 32;
// int buf[numwords];
//  int addr=128;//0;
//  int max=0;
//  for (int e = 0; e < numwords; ++e)
//  {
//    buf[e] = ReadGosip (fSFP, fSlave, addr);
//    if( buf[e] == -1)
//        buf[e]=1;
//    //buf[e]=3*e;
//    if(buf[e] >max)
//        max=buf[e];
//    addr+=4;
//  }
//




//  for (int i = 0; i < numwords; ++i)
//    {
//      fPlot->addPoint (i, buf[i]);
//    }

  int max=0;
  int t=0;

  for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
     {
       for (int sl = 0; sl < theSample->GetLoopsize(loop); ++sl)
       {
         for (int ch = 0; ch < POLAND_DAC_NUM; ++ch)
         {
           int val = theSample->GetTraceValue(loop,ch,sl);
           printf("ShowSample l:%d sl:%d c:%d val:0x%x\n",loop,sl,ch,val);
           fPlot->addPoint (t++, val);
           if(val >max)
             max=val;
         }
       }
     }
  fLowLimit=0;
  fHighLimit=t;

  // add it to the plot area
  canvas->addPlotObject (fPlot);
  if(max>0x800) max=0x800;
  canvas->setLimits (fLowLimit, fHighLimit, 0.0, max+1);

  canvas->update ();

  //TODO: Kplotwidget with 2d display of time sliceslike in go4
  // unpacker
  // take mouse click zoom/unzoom functions from galapgagos!


}







