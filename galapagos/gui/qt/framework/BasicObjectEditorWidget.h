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
  virtual int RefreshObjectIndex(int ix){return -1;}

  /** load sequence from file fullname. Returns false if no success*/
   virtual bool LoadObject(const QString& fullname){return false;}

   /** save sequence from setup to file fullname. Returns false if no success*/
   virtual bool SaveObject(const QString& fullname, gapg::BasicObject* ob){return false;}

   /** implement in subclass: dedicated requester for kind of object load*/
   virtual bool LoadObjectRequest(){return false;}

   /** implement in subclass: dedicated requester for kind of object save*/
   virtual bool SaveObjectRequest(){return false;}

   /** implement in subclass: dedicated requester for kind of new object creation. */
   virtual bool NewObjectRequest(){return false;}

   /** implement in subclass: dedicated requester for kind of new object deletion. */
    virtual bool DeleteObjectRequest(){return false;}

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void StartEditing(){;}

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void CancelEditing(){;}

   /** implement in subclass: activate dedicated editor for object kind*/
    virtual void ApplyEditing(){;}


public:
 BasicObjectEditorWidget (QWidget* parent = 0);
  virtual ~BasicObjectEditorWidget ();


  virtual void ConnectSlots();


//  virtual void RefreshView ();
//
//  virtual void EvaluateView ();
//
//  /** take values relevant for our widget from Qt settings file*/
//   virtual void ReadSettings(QSettings* set);
//
//   /** put values relevant for our widget from Qt settings file*/
//  virtual void WriteSettings(QSettings* set);



public slots:

virtual void ObjectIndexChanged (int ix);

virtual void ObjectNew_clicked();
virtual void ObjectEdit_clicked();
virtual void ObjectLoad_clicked();
virtual void ObjectSave_clicked();
virtual void ObjectApply_clicked();
virtual void ObjectEditCancel_clicked();
virtual void ObjectDelete_clicked();



};


} // namespace
#endif
