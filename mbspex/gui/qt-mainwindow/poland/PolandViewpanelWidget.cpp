#include "PolandViewpanelWidget.h"
#include "PolandSetup.h"

#include "GosipGui.h"


//#include <kplotobject.h>
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
    QWidget (parent), fPlot(0), theSample(0), fPickCounter(0),
	fLowLimit(0), fHighLimit(1), fNminLimit(0), fNmaxLimit(1),
	fDisplayChannel(POLAND_DAC_NUM), fDisplayLoop(POLAND_QFWLOOPS),
	fChannelSumMode(false), fPlotColorcode(0), fPlotSize(1), fPlotStyle(0)

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

  QObject::connect (PointColorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(PointColor_changed(int)));
  QObject::connect (PointStyleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(PointStyle_changed(int)));
  QObject::connect (PlotsizeSlider, SIGNAL(valueChanged(int)), this, SLOT(PointSize_changed(int)));

  QObject::connect (Traces_radioButton, SIGNAL(toggled(bool)), this, SLOT(TracesEnabled_toggled(bool)));

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

void PolandViewpanelWidget::PointColor_changed(int val)
{
	//std::cout <<" PolandViewpanelWidget::PointColor_changed "<< val  <<std::endl;
	fPlotColorcode=val;
	RefreshDrawStyle();

}
void PolandViewpanelWidget::PointStyle_changed(int val)
 {
	//std::cout <<" PolandViewpanelWidget::PointStyle_changed "<< val  <<std::endl;
	fPlotStyle=val;
	RefreshDrawStyle();
 }


void PolandViewpanelWidget::PointSize_changed(int val)
{
	//std::cout <<" PolandViewpanelWidget::PointSize_changed "<< val  <<std::endl;
	fPlotSize=val;
	RefreshDrawStyle();
}

void PolandViewpanelWidget::TracesEnabled_toggled(bool on)
{
	//std::cout <<" PolandViewpanelWidget::TracesEnabled_toggled "<< on  <<std::endl;
  fChannelSumMode=!on;
	TraceModeFrame->setEnabled(on);
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


void PolandViewpanelWidget::RefreshDrawStyle()
{
	GOSIP_LOCK_SLOT

	 QList<KPlotObject *> objects = PlotwidgetChSlice->plotObjects ();
	if (!objects.isEmpty ())
	{
	      KPlotObject * thePlot = objects.first ();
	      thePlot->setPointStyle (mapPointstyleIndex(fPlotStyle));
	      thePlot->setSize(fPlotSize);
	      QColor col=mapColorIndex(fPlotColorcode);
	      // set here all possible line and point colors to the chosen:
	      thePlot->setBrush(col);
          thePlot->setBarBrush(col);          
	       thePlot->setPen( QPen( thePlot->brush(), 1 ) );
	       thePlot->setLinePen( thePlot->pen() );
	       thePlot->setBarPen( thePlot->pen() );
	       thePlot->setLabelPen( thePlot->pen() );
	      // end stolen from KPlotObject ctor JAM2020

	}
	PlotwidgetChSlice->update ();
	GOSIP_UNLOCK_SLOT
}

void PolandViewpanelWidget::RefreshEventCounter()
{
	 if(theSample==0) return;
	 int numberbase=GosipGui::fInstance->GetNumberBase();
	 EventCounter->setMode((numberbase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
	 EventCounter->display ((int) theSample->GetEventCounter());

}


void PolandViewpanelWidget::ShowSample (PolandSample* sample)
{
  //std::cout <<"ShowSample "<< std::endl;
  if(sample!=0) theSample=sample;
  if(theSample==0) return;
  RefreshEventCounter();

  PlotwidgetChSlice->resetPlot ();
  // labels for plot area:
  PlotwidgetChSlice->setAntialiasing (true);

  if(fChannelSumMode)
  {
	  // display sum over each channel here:
	  PlotwidgetChSlice->axis (KPlotWidget::BottomAxis)->setLabel ("Channel number");
	  PlotwidgetChSlice->axis (KPlotWidget::LeftAxis)->setLabel ("Sum counts");
	  fPlot = new KPlotObject(mapColorIndex(fPlotColorcode), KPlotObject::Points, fPlotSize, mapPointstyleIndex(fPlotStyle));
	   int channelsums[POLAND_DAC_NUM]={0};
		for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
		{
			for (int ch = 0; ch < POLAND_DAC_NUM; ++ch)
			{
				for (int sl = 0; sl < theSample->GetLoopsize(loop); ++sl)
				{
					channelsums[ch] += theSample->GetTraceValue(loop, ch, sl);
				}
			}
		}

		fNmaxLimit=0;
		for (int ch = 0; ch < POLAND_DAC_NUM; ++ch)
		{
			 int val=channelsums[ch];
			 fPlot->addPoint (ch, val);
			 if(val >fNmaxLimit)
				 fNmaxLimit=val;
		}
		 fLowLimit=0;
		 fHighLimit=POLAND_DAC_NUM;
		 fNmaxLimit+=1;
		 fNminLimit=0;
  }
  else
  {
  // regular mode: display traces, optionally with filters for loops and channels


  PlotwidgetChSlice->axis (KPlotWidget::BottomAxis)->setLabel ("Trace index (#samples)");
  PlotwidgetChSlice->axis (KPlotWidget::LeftAxis)->setLabel ("Register value ");


  fPlot = new KPlotObject(mapColorIndex(fPlotColorcode), KPlotObject::Points, fPlotSize, mapPointstyleIndex(fPlotStyle));

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
}
  // add it to the plot area
  PlotwidgetChSlice->addPlotObject (fPlot);



  //if(fNmaxLimit>0x800) fNmaxLimit=0x800;
  
  RefreshView ();


}


  QColor PolandViewpanelWidget::mapColorIndex(int i)
  {
	  QColor col;
	  switch (i)
	 {
	     case 0:
	     default:
	    	 col=Qt::red;
	    	 break;
	     case 1:
	    	 col=Qt::green;
	    	 break;
	     case 2:
	    	 col=Qt::blue;
	    	 break;
	     case 3:
	    	 col=Qt::cyan;
	    	 break;
	     case 4:
	    	 col=Qt::yellow;
	    	 break;
	     case 5:
	    	 col=Qt::magenta;
	    	 break;
	 };
	  	  return col;
  }

  KPlotObject::PointStyle PolandViewpanelWidget::mapPointstyleIndex(int i)
  {
	  KPlotObject::PointStyle stil;

	  switch (i)
	 	 {
	 	     case 0:
	 	     default:
	 	    	 stil=KPlotObject::Circle;
	 	    	 break;
	 	     case 1:
	 	    	 stil=KPlotObject::Letter;
	 	    	 break;
	 	     case 2:
	 	    	 stil=KPlotObject::Asterisk;
	 	    	 break;
	 	     case 3:
	 	    	 stil=KPlotObject::Square;
	 	    	 break;
	 	     case 4:
	 	    	 stil=KPlotObject::Triangle;
	 	    	 break;
	 	     case 5:
	 	    	 stil=KPlotObject::Pentagon;
	 	    	 break;
	 	    case 6:
	 	    	stil=KPlotObject::Hexagon;
	 	    	break;

	 	 };



	  return stil;
  }




