#ifndef GALPATTERNGWIDGET_H
#define GALPATTERNGWIDGET_H



#include "ui_GalPatternWidget.h"
#include "GalSubWidget.h"

Q_DECLARE_METATYPE(Okteta::AbstractByteArrayView::ValueCoding);


class GalPatternWidget: public GalSubWidget, public Ui::GalPatternWidget
{
  Q_OBJECT

protected:

  /** refresh editor content for pattern id*/
    void RefreshPatternIndex(int ix);

    /** load pattern from file fullname. Returns false if no success*/
    bool LoadPattern(const QString& fullname);

        /** save pattern from setup to file fullname. Returns false if no success*/
    bool SavePattern(const QString& fullname, GalapagosPattern* pat);

public:
 GalPatternWidget (QWidget* parent = 0);
  virtual ~GalPatternWidget ();


  virtual void ConnectSlots();


   virtual void RefreshView ();

   virtual void EvaluateView ();

   /** take values relevant for our widget from Qt settings file*/
    virtual void ReadSettings(QSettings* set);

    /** put values relevant for our widget from Qt settings file*/
   virtual void WriteSettings(QSettings* set);


public slots:

virtual void PatternIndexChanged (int ix);

virtual void PatternNew_clicked();
virtual void PatternEdit_clicked();
virtual void PatternLoad_clicked();
virtual void PatternSave_clicked();
virtual void PatternApply_clicked();
virtual void PatternEditCancel_clicked();
virtual void PatternDelete_clicked();


};

#endif
