#ifndef FEBEXWIDGET_H
#define FEBEXWIDGET_H

#include "ui_TamexPadiWidget.h"



class TamexPadiWidget: public QWidget, public Ui::TamexPadiWidget
{
  Q_OBJECT

public:
  TamexPadiWidget (QWidget* parent = 0);
  virtual ~TamexPadiWidget ();





public slots:

};

#endif
