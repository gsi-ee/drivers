#ifndef GALSEQUENCEGWIDGET_H
#define GALSEQUENCEGWIDGET_H

#include "ui_GalSequenceWidget.h"
#include "GalSubWidget.h"


class GalSequenceWidget: public GalSubWidget, public Ui::GalSequenceWidget
{
  Q_OBJECT

protected:

  /** Refresh gui display for sequence index ix in list*/
  void RefreshSequenceIndex(int ix);

  /** load sequence from file fullname. Returns false if no success*/
   bool LoadSequence(const QString& fullname);

   /** save sequence from setup to file fullname. Returns false if no success*/
   bool SaveSequence(const QString& fullname, GalapagosSequence* seq);

public:
 GalSequenceWidget (QWidget* parent = 0);
  virtual ~GalSequenceWidget ();


  virtual void ConnectSlots();


  virtual void RefreshView ();

  virtual void EvaluateView ();

  /** take values relevant for our widget from Qt settings file*/
   virtual void ReadSettings(QSettings* set);

   /** put values relevant for our widget from Qt settings file*/
  virtual void WriteSettings(QSettings* set);



public slots:

virtual void SequenceIndexChanged (int ix);

virtual void SequenceNew_clicked();
virtual void SequenceEdit_clicked();
virtual void SequenceLoad_clicked();
virtual void SequenceSave_clicked();
virtual void SequenceApply_clicked();
virtual void SequenceEditCancel_clicked();
virtual void SequenceDelete_clicked();



};

#endif
