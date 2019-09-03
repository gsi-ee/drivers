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
  NewObjectRequest();
//  theSetup_GET_FOR_CLASS(GalapagosSetup);
//  bool ok=false;
//  // automatic assignment of new id here: begin with id from index
//  size_t sid= theSetup->NumKnownObjects()+1;
//  while (theSetup->GetObject(sid)!=0) sid++;
//
//  QString defaultname=QString("Object_%1").arg(sid);
//  QString seqname = QInputDialog::getText(this, tr("Create a new sequence"),
//                                         tr("Object name:"), QLineEdit::Normal,
//                                         defaultname, &ok);
//  if (!ok || seqname.isEmpty()) return;
//  GalapagosObject seq(sid,seqname.toLatin1().constData());
//  seq.AddCommand("NOP");
//  seq.Compile();
//  theSetup->AddObject(seq);



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
  //ObjectTextEdit->setReadOnly(false);

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
  //ObjectTextEdit->setReadOnly(true);
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





//bool BasicObjectEditorWidget::LoadObject(const QString& fileName)
//{
////  //TODO: later redefine sequence script format with some html tags?
////  QFile sfile(fileName);
////  if (!sfile.open( QIODevice::ReadOnly))
////    {
////    printm ("!!! Could not open sequence file %s", fileName.toLatin1().constData());
////    return false;
////    }
////  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
////  QString seqname=fileName.split("/").last();
////  seqname.chop(4);
////  //std::cout << "Loading sequence from file "<< seqname.toLatin1().constData()<< std::endl;
////  GalapagosObject seq(0, seqname.toLatin1().constData()); // sequence id will be taken from file
////  QByteArray content = sfile. readAll();
////  QList<QByteArray> commands=content.split(QChar::LineFeed);
////  QList<QByteArray>::const_iterator cit;
////  bool hasObjectId=false;
////  for (cit = commands.constBegin(); cit != commands.constEnd(); ++cit)
////  {
////    QByteArray cmd=*cit;
////    //std::cout<< "   scanning line "<<cmd.data() << std::endl;
////    QString line(cmd);
////    if(!hasObjectId)
////      {
////      QString line(cmd);
////      if(!line.contains("ObjectID")) continue;
////      //std::cout<< "sequence id keyword was found!"<< std::endl;
////      bool ok=false;
////      QString value=line.split("=").last();
////      int sid=value.toInt(&ok);
////      if(!ok) continue;
////      //std::cout<< "Found sequence id "<<sid << std::endl;
////      seq.SetId(sid);
////      hasObjectId=true;
////    }
////    if(line.contains("#")) continue;
////    //cmd.append(";");
////    seq.AddCommand(cmd.data());
////    //std::cout<< "   Added command "<<cmd.data() << std::endl;
////  }
////  if(seq.Id()==0)
////  {
////    printm("LoadObject %s error: could not read sequence ID!",seqname.toLatin1().constData());
////    return false;
////  }
////
////  seq.Compile();
////  GalapagosObject* oldseq=0;
////  if((oldseq=theSetup->GetObject(seq.Id()))!=0)
////  {
////    printm("LoadObject %s error: sequence %s had already assigned specified unique id %d !",seqname.toLatin1().constData(), oldseq->Name(), seq.Id());
////    return false;
////  }
////  theSetup->AddObject(seq);
////  return true;
//}
//
//bool BasicObjectEditorWidget::SaveObject(const QString& fileName, BasicObject* seq)
//{
////  //TODO: later redefine sequence script format with some html tags?
////  if(seq==0) return false;
////  QFile sfile(fileName);
////    if (!sfile.open( QIODevice::WriteOnly))
////    {
////      printm ("!!! Could not open file %s", fileName.toLatin1().constData());
////      return false;
////    }
////   QString header= QString("#Object %1 saved on %2").arg(seq->Name()).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy - hh:mm:ss."));
////   QString idtag= QString("#ObjectID = %1").arg(seq->Id());
////   sfile.write(header.toLatin1().constData());
////   sfile.write("\n");
////   sfile.write(idtag.toLatin1().constData());
////   sfile.write("\n");
////  const char* line=0;
////     int l=0;
////     while ((line=seq->GetCommandLine(l++)) !=0)
////       {
////         //std::cout<<"ObjectSave_clicked writes line:"<<line  << std::endl;
////         sfile.write(line);
////         sfile.write("\n");
////       }
////  return true;
//}
//



//void BasicObjectEditorWidget::EvaluateView ()
//{
//
//
//}


//void BasicObjectEditorWidget::RefreshObjectIndex(int ix)
//{
//
////  ObjectTextEdit->clear();
////
////   // now take out commands from known sequences:
////   theSetup_GET_FOR_CLASS(GalapagosSetup);
////   GalapagosObject* seq=theSetup->GetKnownObject(ix);
////   if(seq==0)  {
////       //ObjectTextEdit->appendPlainText("unknown ID!");
////       printm("Warning: unknown sequence index %d in combobox, NEVER COME HERE!!",ix);
////       return;
////   }
////   //std::cout<<"RefreshObjectIndex gets sequence :"<<std::hex<< (ulong) seq<< ", id:"<<std::dec << seq->Id()<<", name:"<<seq->Name()<< std::endl;
////   const char* line=0;
////   int l=0;
////   while ((line=seq->GetCommandLine(l++)) !=0)
////     {
////       //std::cout<<"RefreshObjectIndex reading  line:"<<line  << std::endl;
////       QString txt(line);
////       ObjectTextEdit->appendPlainText(txt);
////     }
////   ObjectIDSpinBox->setValue(seq->Id());
//}

//void BasicObjectEditorWidget::RefreshView ()
//{
////  theSetup_GET_FOR_CLASS(GalapagosSetup);
////  int oldsid=Object_comboBox->currentIndex(); // remember our active item
////   Object_comboBox->clear();
////   for (int six=0; six<theSetup->NumKnownObjects(); ++six)
////   {
////     GalapagosObject* seq=theSetup->GetKnownObject(six);
////     if(seq==0)  continue;
////     // populate names in sequence editor window:
////     Object_comboBox->addItem(seq->Name());
////   }
////   Object_comboBox->setCurrentIndex(oldsid); // restore active item
////   RefreshObjectIndex(oldsid);
//
//}



//void BasicObjectEditorWidget::ReadSettings (QSettings* settings)
//{
////  int numseqs = settings->value ("/Numsequences", 1).toInt ();
////  for (int six = 3; six < numseqs; ++six)    // do not reload the default entries again
////  {
////    QString settingsname = QString ("/Objects/%1").arg (six);
////    QString seqfilename = settings->value (settingsname).toString ();
////    //std::cout<< " GalapagosGui::ReasdSettings() will load sequence file"<<seqfilename.toLatin1().data()<< std::endl;
////    if (!LoadObject (seqfilename))
////      printm ("Warning: Object %s from setup could not be loaded!", seqfilename.toLatin1 ().data ());
////  }
////  int oldix = 0;    // later take from settings
////  Object_comboBox->setCurrentIndex (oldix);    // toggle refresh the editor?
////  ObjectIndexChanged (oldix);
//}
//
//void BasicObjectEditorWidget::WriteSettings (QSettings* settings)
//{
////  theSetup_GET_FOR_CLASS(GalapagosSetup);
////  for (int six = 0; six < theSetup->NumKnownObjects (); ++six)
////  {
////    GalapagosObject* seq = theSetup->GetKnownObject (six);
////    if (seq == 0)
////      continue;
////    QString settingsname = QString ("/Objects/%1").arg (six);
////    QString seqfilename = QString ("%1.gas").arg (seq->Name ());
////    settings->setValue (settingsname, seqfilename);
////    //std::cout<< " BasicObjectEditorWidget::WriteSettings() saves sequence file"<<seqfilename.toLatin1().data()<< std::endl;
////    SaveObject (seqfilename, seq);
////  }
////  settings->setValue ("Numsequences", (int) theSetup->NumKnownObjects ());
//
//}


} // namespace




