#ifndef GAPG_GALPATTERNGWIDGET_H
#define GAPG_GALPATTERNGWIDGET_H

#include "BasicObjectEditorWidget.h"
#include "GalPatternEditor.h"

namespace gapg
{

class GalPatternDisplay;

class GalPatternWidget: public BasicObjectEditorWidget
{
  Q_OBJECT

protected:

  gapg::GalPatternEditor* fPatternEditor;

  gapg::GalPatternDisplay* fPatternDisplay;

  /** refresh editor content for pattern id, return unique object id*/
  int RefreshObjectIndex (int ix);

  /** load pattern from file fullname. Returns false if no success*/
  bool LoadObject (const QString& fullname);

  /** save pattern from setup to file fullname. Returns false if no success*/
  bool SaveObject (const QString& fullname, BasicObject* pat);

  virtual bool LoadObjectRequest ();

  /** implement in subclass: dedicated requester for kind of object save*/
  virtual bool SaveObjectRequest ();

  /** implement in subclass: dedicated requester for kind of new object creation. */
  virtual bool NewObjectRequest ();

  /** implement in subclass: dedicated requester for kind of new object deletion. */
  virtual bool DeleteObjectRequest ();

  /** implement in subclass: activate dedicated editor for object kind*/
  virtual void StartEditing ();

  /** implement in subclass: activate dedicated editor for object kind*/
  virtual void CancelEditing ();

  /** implement in subclass: activate dedicated editor for object kind*/
  virtual void ApplyEditing ();

public:
  GalPatternWidget (QWidget* parent = 0);
  virtual ~GalPatternWidget ();

  void SetPatternDisplay(gapg::GalPatternDisplay* disp){fPatternDisplay=disp;}

  virtual void ConnectSlots ();

  virtual void RefreshView ();

  virtual void EvaluateView ();

  /** take values relevant for our widget from Qt settings file*/
  virtual void ReadSettings (QSettings* set);

  /** put values relevant for our widget from Qt settings file*/
  virtual void WriteSettings (QSettings* set);

public slots:

virtual void PatternPreview_clicked();

};

}    // gapg

#endif
