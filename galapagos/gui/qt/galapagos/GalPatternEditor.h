#ifndef GAPG_GALPATTERNEDITOR_H
#define GAPG_GALPATTERNEDITOR_H


#include <QWidget>



#include "ui_GalPatternEditor.h"

Q_DECLARE_METATYPE(Okteta::AbstractByteArrayView::ValueCoding);


namespace gapg{


class GalPatternEditor: public QWidget, public Ui::GalPatternEditor
{
  Q_OBJECT

protected:


public:
 GalPatternEditor (QWidget* parent = 0);
  virtual ~GalPatternEditor ();


};

} // gapg

#endif
