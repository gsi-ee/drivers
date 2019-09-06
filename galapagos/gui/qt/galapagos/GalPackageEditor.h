#ifndef GAPG_GALPACKAGEEDITOR_H
#define GAPG_GALPACKAGEEDITOR_H


#include <QWidget>



#include "ui_GalPackageEditor.h"



namespace gapg{


class GalPackageEditor: public QWidget, public Ui::GalPackageEditor
{
  Q_OBJECT

protected:


public:
 GalPackageEditor (QWidget* parent = 0);
  virtual ~GalPackageEditor ();


};

} // gapg

#endif
