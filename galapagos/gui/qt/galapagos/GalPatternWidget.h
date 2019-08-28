#ifndef GALPATTERNGWIDGET_H
#define GALPATTERNGWIDGET_H



#include "ui_GalPatternWidget.h"

Q_DECLARE_METATYPE(Okteta::AbstractByteArrayView::ValueCoding);


class GalPatternWidget: public QWidget, public Ui::GalPatternWidget
{
  Q_OBJECT

public:
 GalPatternWidget (QWidget* parent = 0);
  virtual ~GalPatternWidget ();





public slots:

};

#endif
