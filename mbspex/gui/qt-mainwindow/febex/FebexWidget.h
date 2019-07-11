#ifndef FEBEXWIDGET_H
#define FEBEXWIDGET_H

#include "ui_FebexWidget.h"



class FebexWidget: public QWidget, public Ui::FebexWidget
{
  Q_OBJECT

public:
  FebexWidget (QWidget* parent = 0);
  virtual ~FebexWidget ();





public slots:

};

#endif
