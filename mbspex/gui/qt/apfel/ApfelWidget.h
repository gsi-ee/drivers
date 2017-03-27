#ifndef APFELWIDGET_H
#define APFELWIDGET_H

#include "ui_ApfelWidget.h"



class ApfelWidget: public QWidget, public Ui::ApfelWidget
{
  Q_OBJECT

public:
  ApfelWidget (QWidget* parent = 0);
  virtual ~ApfelWidget ();





public slots:

};

#endif
