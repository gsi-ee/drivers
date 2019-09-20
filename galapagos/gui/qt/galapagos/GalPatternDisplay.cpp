#include "GalPatternDisplay.h"
#include "GalapagosSetup.h"

#include <kplotobject.h>
#include <kplotwidget.h>
#include <kplotaxis.h>
#include <kplotpoint.h>

#include <math.h>

// TODO: need below include because of theSetup macros. include partitioning!
#include "BasicGui.h"



namespace gapg {

GalPatternDisplay::GalPatternDisplay (QWidget* parent) :
gapg::BasicSubWidget(parent), fPlot(0),fPickCounter(0)
{
  Ui_GalPatternDisplay::setupUi (this);

  Plotwidget->axis (KPlotWidget::BottomAxis)->setLabel ("Time (#tics)");
  Plotwidget->axis (KPlotWidget::LeftAxis)->setLabel ("Output value");
  Plotwidget->setLimits (0, 1024, 0, 1.5);
  Plotwidget->update ();
}

GalPatternDisplay::~GalPatternDisplay ()
{
}


void GalPatternDisplay::ConnectSlots()
{

  QObject::connect (UnzoomButton, SIGNAL(clicked()), this, SLOT(UnzoomButton_clicked()));
  QObject::connect (SetZoomButton,SIGNAL(clicked(bool)), this, SLOT(SetZoomButton_toggled(bool)));
  QObject::connect (PatternLow_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternLow_spinBox_changed(int)));
  QObject::connect (PatternHi_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternHi_spinBox_changed(int)));


}


void GalPatternDisplay::mousePressEvent (QMouseEvent *event)
{
//std::cout <<" GalPatternDisplay::mousePressEvent"  <<std::endl;
if (event->button () == Qt::LeftButton)
{

  if (fPickCounter > 0)
  {

    //std::cout <<" Left button at position x:"<< event->x()<<", y:"<<event->y()  <<std::endl;
    // following correction for the plot border was stolen from a protected method in KPlotWidget:
    QPoint pickpoint = event->pos () - QPoint (Plotwidget->leftPadding (), Plotwidget->topPadding ())
        - Plotwidget->contentsRect ().topLeft ();

    QList<KPlotObject *> objects = Plotwidget->plotObjects ();
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
        delta = ::fabs ((double) pickpoint.x () - (double) Plotwidget->mapToWidget (pt->position ()).x ());
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
        printm(" GalPatternDisplay::mousePressEvent NEVER COME HERE: piccounter mismatch : %d",fPickCounter);
      }

      fPickCounter--;

    }

  }    // fPickCounter

}


    QWidget::mousePressEvent(event);

}



void GalPatternDisplay::EvaluateView ()
{


}





void GalPatternDisplay::RefreshView ()
{
  GAPG_LOCK_SLOT
  PatternLow_spinBox->setValue(fLowLimit);
  PatternHi_spinBox->setValue(fHighLimit);

  Plotwidget->setLimits (fLowLimit, fHighLimit, -0.2, 1.2);
  Plotwidget->update ();
  GAPG_UNLOCK_SLOT
}









void GalPatternDisplay::ReadSettings (QSettings* settings)
{
//  int numpats = settings->value ("/Numpatterns", 1).toInt ();
//  for (int pix = 4; pix < numpats; ++pix)    // do not reload the default entries again
//  {
//    QString settingsname = QString ("/Patterns/%1").arg (pix);
//    QString patfilename = settings->value (settingsname).toString ();
//    //std::cout<< " GalapagosGui::ReasdSettings() will load sequence file"<<seqfilename.toLatin1().data()<< std::endl;
//    if (!LoadObject (patfilename))
//      printm ("Warning: Pattern %s from setup could not be loaded!", patfilename.toLatin1 ().data ());
//  }
//  int oldpix = 0;    // later take from settings
//  Object_comboBox->setCurrentIndex (oldpix);    // toggle refresh the editor?

}

void GalPatternDisplay::WriteSettings (QSettings* settings)
{
//  theSetup_GET_FOR_CLASS(GalapagosSetup);
//  for (int pix = 0; pix < theSetup->NumKnownPatterns (); ++pix)
//  {
//    GalapagosPattern* pat = theSetup->GetKnownPattern (pix);
//    if (pat == 0)
//      continue;
//    QString settingsname = QString ("/Patterns/%1").arg (pix);
//    QString patfilename = QString ("%1.gap").arg (pat->Name ());
//    settings->setValue (settingsname, patfilename);
//    SaveObject (patfilename, pat);
//  }
//  settings->setValue ("Numpatterns", (int) theSetup->NumKnownPatterns ());

}




void GalPatternDisplay::UnzoomButton_clicked()
{
  //std::cout <<" GalPatternDisplay::UnzoomButton_clicked"  <<std::endl;
  fLowLimit=0;
  fHighLimit= fPlot ? fPlot->points().size() : 0;
  RefreshView();


}

void GalPatternDisplay::SetZoomButton_toggled(bool on)
{
  //std::cout <<" GalPatternDisplay::SetZoomButton_toggled "<< on  <<std::endl;
  fPickCounter= on ? 2 : 0;
}

void GalPatternDisplay::PatternLow_spinBox_changed(int val)
{
  //std::cout <<" GalPatternDisplay::PatternLow_spinBox_changed "<< val  <<std::endl;
  fLowLimit=val;
  RefreshView();
}

void GalPatternDisplay::PatternHi_spinBox_changed(int val)
{
  //std::cout <<" GalPatternDisplay::PatternHi_spinBox_changed "<< val  <<std::endl;
  fHighLimit=val;
  RefreshView();
}









void GalPatternDisplay::PlotPattern(QByteArray& pat, const QString& name)
{
  GAPG_LOCK_SLOT
  //std::cout <<" GalPatternDisplay::PlotPattern"  <<std::endl;

  QColor col=Qt::green;;
  KPlotObject::PointStyle pstyle = KPlotObject::Circle;
  Plotwidget->resetPlot ();
  Plotwidget->axis (KPlotWidget::BottomAxis)->setLabel ("Time (#tics)");
  Plotwidget->axis (KPlotWidget::LeftAxis)->setLabel ("Output value");

  // not necessary, handled by resetPlot
  //if(fPlot) delete fPlot;
  fPlot = new KPlotObject (col, KPlotObject::Lines, 2);
  int samplength=pat.size()*8;
  for (int i = 0; i < pat.size(); ++i)
     {
       char val=pat[i];
       for(int b=0; b<8;++b)
       {
         int pos = i*8 + b;
         char mask=1;
         char bit= (val >> b) & mask ;
         fPlot->addPoint (pos, bit);
       }
     }
  fLowLimit=0;
  fHighLimit=samplength;


  Plotwidget->addPlotObject (fPlot);
  Plotwidget->setLimits (0, samplength, -0.2, 1.2);
  Plotwidget->update ();

  PatternLow_spinBox->setValue(fLowLimit);
  PatternHi_spinBox->setValue(fHighLimit);

  fPatternName=name;
  GAPG_UNLOCK_SLOT
 }


} // gapg
