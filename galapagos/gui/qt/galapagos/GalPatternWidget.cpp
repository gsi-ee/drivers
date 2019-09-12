#include "GalPatternWidget.h"
#include "GalPatternEditor.h"
#include "GalapagosSetup.h"

// TODO: need below include because of theSetup macros. include partitioning!
#include "BasicGui.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QSettings>
#include <QDateTime>

#include <okteta/piecetablebytearraymodel.h>

namespace gapg {

GalPatternWidget::GalPatternWidget (QWidget* parent) :
gapg::BasicObjectEditorWidget(parent)
{
  fPatternEditor=new gapg::GalPatternEditor(this);
  Editor_scrollArea->setWidget(fPatternEditor);
  setWindowTitle("Pattern Editor");

  Object_comboBox->setToolTip("Select known pattern by name");
  ObjectIDSpinBox-> setToolTip("Unique id of selected pattern");

  ObjectNewButton->setToolTip("Create new pattern");
  ObjectEditButton->setToolTip("Edit contents of selected pattern");
  ObjectDeleteButton->setToolTip("Remove selected pattern");
  ObjectLoadButton->setToolTip("Load pattern from *.gap file to the list");
  ObjectSaveButton->setToolTip("Export selected pattern to a file (*.gap)");
  ObjectEditCancelButton->setToolTip("Cancel edited code for selected pattern (restore last setup)");
  ObjectApplyButton->setToolTip("Apply editor contents for selected pattern");

}

GalPatternWidget::~GalPatternWidget ()
{
}


void GalPatternWidget::ConnectSlots()
{
  BasicObjectEditorWidget::ConnectSlots();
  // anything more here?

}




bool GalPatternWidget::NewObjectRequest()
{
  std::cout << "GalPatternWidget::NewObjectRequest"<< std::endl;
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
    bool ok=false;
    // automatic assignment of new id here: begin with id from index
    size_t pid= theSetup->NumKnownPatterns()+1;
    while (theSetup->GetPattern(pid)!=0) pid++;

    QString defaultname=QString("Pattern_%1").arg(pid);
    QString patname = QInputDialog::getText(this, tr("Create a new bit pattern"),
                                           tr("Pattern name:"), QLineEdit::Normal,
                                           defaultname, &ok);
    if (!ok || patname.isEmpty()) return false;
    GalapagosPattern pat(pid,patname.toLatin1().constData());
    pat.AddByte(0xFF); // some default value to fill editor
    theSetup->AddPattern(pat);
    return true;
}

void GalPatternWidget::StartEditing()
{
  //std::cout << "GalPatternWidget::StartEditing"<< std::endl;
  fPatternEditor->oktetabyteview->setReadOnly(false);

}

bool GalPatternWidget::LoadObjectRequest()
{
  //std::cout << "GalPatternWidget::LoadObjectRequest()"<< std::endl;
  QFileDialog fd( this,
                      "Select Files with New Galapagos bit pattern",
                      fLastFileDir,
                      QString("Galapagos Pattern files (*.gap);;All files (*.*)"));

  fd.setFileMode( QFileDialog::ExistingFiles);

  if ( fd.exec() != QDialog::Accepted ) return false;
  QStringList list = fd.selectedFiles();
  QStringList::Iterator fit = list.begin();
  while( fit != list.end() ) {
    QString fileName = *fit;
    fLastFileDir = QFileInfo(fileName).absolutePath();
    if(!LoadObject(fileName))
    {
      printm("Pattern load sees error with %s",fileName.toLatin1().data());
    }
    ++fit;
  }
  return true;
}


bool GalPatternWidget::SaveObjectRequest()
{
  //std::cout << "GalPatternWidget::SaveObjectRequest()"<< std::endl;
  QFileDialog fd( this,
                       "Save Galapagos bit pattern to file",
                       fLastFileDir,
                       QString("Galapagos Pattern files (*.gap)"));
   fd.setFileMode( QFileDialog::AnyFile);
   fd.setAcceptMode(QFileDialog::AcceptSave);
   QString defname=Object_comboBox->currentText();
   defname.append(".gap");
   fd.selectFile(defname);
   if (fd.exec() != QDialog::Accepted) return false;
   QStringList flst = fd.selectedFiles();
   if (flst.isEmpty()) return false;
   theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
   QString fileName = flst[0];
   fLastFileDir = fd.directory().path();
   int ix=Object_comboBox->currentIndex();
    GalapagosPattern* pat=theSetup->GetKnownPattern(ix);
     if(pat==0)  {
         printm("NEVER COME HERE:unknown  pattern for index %d!",ix);
         return false;
     }

   if(!SaveObject(fileName,pat)){
     printm("Could not save pattern of index %d to file %s!",ix,fileName.toLatin1().constData());
     return false;
   }
   return true;
}

void GalPatternWidget::ApplyEditing()
{
  //std::cout << "GalPatternWidget::ApplyEditing"<< std::endl;
  fPatternEditor->oktetabyteview->setReadOnly(true);

   theSetup_GET_FOR_CLASS(GalapagosSetup);
   int ix=Object_comboBox->currentIndex();
   GalapagosPattern* pat=theSetup->GetKnownPattern(ix);
   if(pat==0)  {
     fParent->ShowStatusMessage("NEVER COME HERE: Pattern id not known in setup!");
     return;
    }
   // evaluate bytarray from model and put it to our pattern object:
   fPatternEditor->oktetabyteview->selectAll(true);
   QByteArray theData=fPatternEditor->oktetabyteview->selectedData();
   pat->Clear();
   pat->SetId(ObjectIDSpinBox->value());
   size_t numbytes=theData.size();
   for(int i=0; i<numbytes; ++i)
   {
     pat->AddByte(theData[i]);
   }
}



void GalPatternWidget::CancelEditing()
{
  //std::cout << "GalPatternWidget::PatternEditCancel_clicked"<< std::endl;

  fPatternEditor->oktetabyteview->setReadOnly(true);

}


bool  GalPatternWidget::DeleteObjectRequest()
{
  //std::cout << "GalPatternWidget::PatternDelete_clicked"<< std::endl;
  if(QMessageBox::question( this, fImpName, "Really Delete current pattern from list?",
      QMessageBox::Yes | QMessageBox::No ,
      QMessageBox::Yes) != QMessageBox::Yes ) {
    return false;
  }
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  int ix=Object_comboBox->currentIndex();
  theSetup->RemoveKnownPattern(ix);
  return true;

}


void GalPatternWidget::EvaluateView ()
{


}

int GalPatternWidget::RefreshObjectIndex(int ix)
{
  theSetup_GET_FOR_CLASS_RETURN(GalapagosSetup);
   GalapagosPattern* pat=theSetup->GetKnownPattern(ix);
   if(pat==0)  {
       printm("Warning: unknown pattern index %d in combobox, NEVER COME HERE!!",ix);
       return -1;
   }

   // here provide okteta model from our pattern data:

   // provide tempory bytearray:
   QByteArray theByteArray;
   size_t numbytes=pat->NumBytes();
   for(int c=0; c<numbytes; ++c)
     theByteArray.append(pat->GetByte(c));

   Okteta::PieceTableByteArrayModel* theByteArrayModel =
       new Okteta::PieceTableByteArrayModel(theByteArray, fPatternEditor->oktetabyteview);



   fPatternEditor->oktetabyteview->setByteArrayModel(theByteArrayModel);
   fPatternEditor->oktetabyteview->setReadOnly(true);
   fPatternEditor->oktetabyteview->setOverwriteMode(false);
   return (pat->Id());
}




void GalPatternWidget::RefreshView ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int oldpat=Object_comboBox->currentIndex(); // remember our active item
  Object_comboBox->clear();
  for (int pix=0; pix<theSetup->NumKnownPatterns(); ++pix)
   {
    GalapagosPattern* pat=theSetup->GetKnownPattern(pix);
    if(pat==0)  continue;
    Object_comboBox->addItem(pat->Name());
   }
  if(oldpat>=theSetup->NumKnownPatterns()) oldpat=theSetup->NumKnownPatterns()-1;
  Object_comboBox->setCurrentIndex(oldpat); // restore active item
  RefreshObjectIndex(oldpat);

  fPatternEditor->oktetabyteview->setValueCoding(fParent->GetNumberBase()==16 ? Okteta::AbstractByteArrayView::HexadecimalCoding : Okteta::AbstractByteArrayView::BinaryCoding);


}







bool GalPatternWidget::LoadObject(const QString& fileName)
{
  QFile pfile(fileName);
   if (!pfile.open( QIODevice::ReadOnly))
     {
     printm ("!!! Could not open pattern file %s", fileName.toLatin1().constData());
     return false;
     }
   theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
   QString patname=fileName.split("/").last();
   patname.chop(4);
   GalapagosPattern pat(0, patname.toLatin1().constData()); // pattern id will be taken from file

// read back our own binary format:

   QDataStream inStream(&pfile);
   // Read and check the header
   quint32 magic, version, id, trailer;
   inStream >> magic;
   if (magic != PATTERN_FILE_TAG)
   {
     printm ("!!! Wrong format header 0x%x in file %s", magic, fileName.toLatin1().constData());
     return false;
   }
   inStream >> version;
   if (version != PATTERN_FILE_VERSION)
   {
     printm ("!!! Wrong format version %d in file %s", version, fileName.toLatin1().constData());
     return false;
   }
   inStream >> id;
   if(id==0)
     {
       printm("LoadPattern %s error: could not read pattern ID!",patname.toLatin1().constData());
       return false;
     }
   pat.SetId(id);

   char* pdata=0;
   uint psize=0;
   inStream.readBytes(pdata, psize);
   if(pdata==0 || psize==0)
   {
     printm ("!!! Error reading pattern bytes from file %s", fileName.toLatin1().constData());
     return false;
   }
   inStream >> trailer;

   if (trailer != PATTERN_FILE_TAG)
   {
       printm ("!!! Wrong format trailer 0x%x in file %s", trailer, fileName.toLatin1().constData());
       return false;
   }
   GalapagosPattern* oldpat=0;
   if((oldpat=theSetup->GetPattern(pat.Id()))!=0)
   {
     printm("LoadPattern %s error: pattern %s had already assigned specified unique id %d !",patname.toLatin1().constData(), oldpat->Name(), pat.Id());
     return false;
   }

   // everything from file was ok, now put it into new pattern:
   for(int t=0; t<psize; ++t)
   {
     pat.AddByte(pdata[t]);
   }
   delete pdata; // was allocated by Qt inside the readBytes function

   theSetup->AddPattern(pat);
   return true;
}


bool GalPatternWidget::SaveObject(const QString& fileName, gapg::BasicObject* ob)
{
  // TODO: optionally change id when saving a pattern?
  GalapagosPattern* pat = dynamic_cast<GalapagosPattern*>(ob);
  if(pat==0) return false;
  QFile pfile(fileName);
    if (!pfile.open( QIODevice::WriteOnly))
    {
      printm ("!!! Could not open file %s", fileName.toLatin1().constData());
      return false;
    }

// first approach: implement own binary format here
// later do support common formats (intel hex, uuencode?) like in okteta for exchange with external editor
    QDataStream outStream(&pfile);
    outStream << (qint32) PATTERN_FILE_TAG; // GALAPAGOS gap format identifier
    outStream << (qint32) PATTERN_FILE_VERSION; // format version number
    outStream << (quint32) pat->Id(); // unique setup id of pattern
    QByteArray* byteArray=pat->CreateByteArray();

//    size_t bytesize=byteArray.size()
//    outStream << (qint32) bytesize; // pattern length information
//    outStream.writeRawData(byteArray.data(), byteArray.size()); // the actual pattern data
    outStream.writeBytes(byteArray->data(), byteArray->size()); // the actual pattern data


    outStream << (qint32) PATTERN_FILE_TAG; // GALAPAGOS gap format identifier trailer

    delete byteArray;
    printm("Saved Pattern with id %d to file %s ",pat->Id(), fileName.toLatin1().constData());
    return true;
}

void GalPatternWidget::ReadSettings (QSettings* settings)
{
  int numpats = settings->value ("/Numpatterns", 1).toInt ();
  for (int pix = 4; pix < numpats; ++pix)    // do not reload the default entries again
  {
    QString settingsname = QString ("/Patterns/%1").arg (pix);
    QString patfilename = settings->value (settingsname).toString ();
    //std::cout<< " GalapagosGui::ReasdSettings() will load sequence file"<<seqfilename.toLatin1().data()<< std::endl;
    if (!LoadObject (patfilename))
      printm ("Warning: Pattern %s from setup could not be loaded!", patfilename.toLatin1 ().data ());
  }
  int oldpix = 0;    // later take from settings
  Object_comboBox->setCurrentIndex (oldpix);    // toggle refresh the editor?

}

void GalPatternWidget::WriteSettings (QSettings* settings)
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  for (int pix = 0; pix < theSetup->NumKnownPatterns (); ++pix)
  {
    GalapagosPattern* pat = theSetup->GetKnownPattern (pix);
    if (pat == 0)
      continue;
    QString settingsname = QString ("/Patterns/%1").arg (pix);
    QString patfilename = QString ("%1.gap").arg (pat->Name ());
    settings->setValue (settingsname, patfilename);
    SaveObject (patfilename, pat);
  }
  settings->setValue ("Numpatterns", (int) theSetup->NumKnownPatterns ());

}


} // gapg
