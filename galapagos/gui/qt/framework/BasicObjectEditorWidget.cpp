#include "BasicGui.h"
#include "BasicObjectEditorWidget.h"
#include "BasicObject.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QSettings>
#include <QDateTime>

namespace gapg{

BasicObjectEditorWidget::BasicObjectEditorWidget (QWidget* parent) :
  BasicSubWidget(parent)
{
  setupUi (this);
}

BasicObjectEditorWidget::~BasicObjectEditorWidget ()
{
}

void BasicObjectEditorWidget::ConnectSlots()
{

  QObject::connect (Object_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ObjectIndexChanged(int)));

  QObject::connect (ObjectNewButton, SIGNAL(clicked()), this, SLOT(ObjectNew_clicked()));
  QObject::connect (ObjectEditButton, SIGNAL(clicked()), this, SLOT(ObjectEdit_clicked()));
  QObject::connect (ObjectLoadButton, SIGNAL(clicked()), this, SLOT(ObjectLoad_clicked()));
  QObject::connect (ObjectSaveButton, SIGNAL(clicked()), this, SLOT(ObjectSave_clicked()));
  QObject::connect (ObjectApplyButton, SIGNAL(clicked()), this, SLOT(ObjectApply_clicked()));
  QObject::connect ( ObjectEditCancelButton, SIGNAL(clicked()), this, SLOT(ObjectEditCancel_clicked()));
  QObject::connect ( ObjectDeleteButton, SIGNAL(clicked()), this, SLOT(ObjectDelete_clicked()));
}



void BasicObjectEditorWidget::ObjectIndexChanged (int ix)
{
  GAPG_LOCK_SLOT;
  //std::cout << "BasicObjectEditorWidget::ObjectIndexChanged  ix="<<ix << std::endl;
  int uniqueid=RefreshObjectIndex(ix);
  ObjectIDSpinBox->setValue(uniqueid);
  GAPG_UNLOCK_SLOT;
}

void BasicObjectEditorWidget::ObjectNew_clicked()
{
  //std::cout << "BasicObjectEditorWidget:: ObjectNew_clicked "<< std::endl;
  if(!NewObjectRequest()) return;
  GAPG_LOCK_SLOT;
  fParent->RefreshView();
  GAPG_UNLOCK_SLOT;
  int sid=Object_comboBox->count(); // new object is at the end of the list
  Object_comboBox->setCurrentIndex(sid-1);
  ObjectEdit_clicked();

}


void BasicObjectEditorWidget::ObjectEdit_clicked()
{
  //std::cout << "BasicObjectEditorWidget:: ObjectEdit_clicked"<< std::endl;
  ObjectApplyButton->setEnabled(true);
  ObjectEditCancelButton->setEnabled(true);
  Object_comboBox->setEnabled(false);
  ObjectIDSpinBox->setEnabled(true);
  StartEditing();
}

void BasicObjectEditorWidget::ObjectEditCancel_clicked()
{
  //std::cout << "BasicObjectEditorWidget:: ObjectEditCancel_clicked"<< std::endl;
  int ix=Object_comboBox->currentIndex();
  ObjectIndexChanged(ix);
  ObjectApplyButton->setEnabled(false);
  ObjectEditCancelButton->setEnabled(false);
  Object_comboBox->setEnabled(true);
  ObjectIDSpinBox->setEnabled(false);
  CancelEditing();
}

void  BasicObjectEditorWidget::ObjectDelete_clicked()
{
  if(!DeleteObjectRequest()) return;
   GAPG_LOCK_SLOT
   fParent->RefreshView();
   GAPG_UNLOCK_SLOT
}


void BasicObjectEditorWidget::ObjectLoad_clicked()
{
  //std::cout << "BasicObjectEditorWidget::ObjectLoad_clicked"<< std::endl;
  if(!LoadObjectRequest()) return;
 GAPG_LOCK_SLOT;
     fParent->RefreshView(); // populate comboboxes with all known sequences also in other subwindows!
 GAPG_UNLOCK_SLOT;

}

void BasicObjectEditorWidget::ObjectSave_clicked()
{
 SaveObjectRequest();
}

void BasicObjectEditorWidget::ObjectApply_clicked()
{
//  //std::cout << "BasicObjectEditorWidget::ObjectOK_clicked"<< std::endl;
  ApplyEditing();
  ObjectApplyButton->setEnabled(false);
  ObjectIDSpinBox->setEnabled(false);
  Object_comboBox->setEnabled(true);
}


} // namespace




