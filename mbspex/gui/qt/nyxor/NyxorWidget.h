#ifndef NYXORWIDGET_H
#define NYXORWIDGET_H

#include "ui_NyxorWidget.h"

class NyxorWidget : public QWidget, public Ui::NyxorWidget
{
  Q_OBJECT

public:
  NyxorWidget (QWidget* parent = 0);
  virtual ~NyxorWidget ();


};

#endif
