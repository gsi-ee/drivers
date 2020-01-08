#ifndef GAPG_BASICOBJECTEDITORWIDGET_H
#define GAPG_BASICOBJECTEDITORWIDGET_H

#include "ui_BasicObjectEditorWidget.h"
#include "BasicSubWidget.h"

namespace gapg{

class BasicObject;

class BasicObjectEditorWidget: public gapg::BasicSubWidget, public Ui::BasicObjectEditorWidget
{
  Q_OBJECT

protected:

  /** Refresh gui display for sequence index ix in list.
   * Returns unique object id in setup*/
  virtual int RefreshObjectIndex(int ix)=0;

  /** load sequence from file fullname. Returns false if no success*/
   virtual bool LoadObject(const QString& fullname)=0;

   /** save sequence from setup to file fullname. Returns false if no success*/
   virtual bool SaveObject(const QString& fullname, gapg::BasicObject* ob)=0;

   /** implement in subclass: dedicated requester for kind of object load*/
   virtual bool LoadObjectRequest()=0;

   /** implement in subclass: dedicated requester for kind of object save*/
   virtual bool SaveObjectRequest()=0;

   /** implement in subclass: dedicated requester for kind of new object creation. */
   virtual bool NewObjectRequest()=0;

   /** implement in subclass: dedicated requester for kind of new object deletion. */
    virtual bool DeleteObjectRequest()=0;

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void StartEditing()=0;

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void CancelEditing()=0;

   /** implement in subclass: activate dedicated editor for object kind*/
    virtual void ApplyEditing()=0;

public:
    BasicObjectEditorWidget (QWidget* parent = 0);

    virtual ~BasicObjectEditorWidget ();


  virtual void ConnectSlots();


public slots:

virtual void ObjectIndexChanged (int ix);

virtual void ObjectNew_clicked();
virtual void ObjectEdit_clicked();
virtual void ObjectLoad_clicked();
virtual void ObjectSave_clicked();
virtual void ObjectApply_clicked();
virtual void ObjectEditCancel_clicked();
virtual void ObjectDelete_clicked();
virtual void ObjectProtect_enabled(bool on);

};


} // namespace
#endif
