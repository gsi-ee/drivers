#ifndef GAPG_GALSEQUENCEEDITOR_H
#define GAPG_GALSEQUENCEEDITOR_H

#include <QWidget>
#include "ui_GalSequenceEditor.h"



namespace gapg {

class GalSequenceEditor: public QWidget, public Ui::GalSequenceEditor
{
  Q_OBJECT

public:
 GalSequenceEditor (QWidget* parent = 0);
  virtual ~GalSequenceEditor ();





};

} // gapg

#endif
