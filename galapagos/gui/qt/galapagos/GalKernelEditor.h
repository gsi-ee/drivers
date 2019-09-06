#ifndef GAPG_GALKERNELEDITOR_H
#define GAPG_GALKERNELEDITOR_H

#include <QWidget>
#include "ui_GalKernelEditor.h"



namespace gapg {

class GalKernelEditor: public QWidget, public Ui::GalKernelEditor
{
  Q_OBJECT

public:
 GalKernelEditor (QWidget* parent = 0);
  virtual ~GalKernelEditor ();





};

} // gapg

#endif
