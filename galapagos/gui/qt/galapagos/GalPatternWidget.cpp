#include "GalPatternWidget.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QSettings>
#include <QDateTime>

#include <okteta/piecetablebytearraymodel.h>


GalPatternWidget::GalPatternWidget (QWidget* parent) :
    GalSubWidget (parent)
{
  setupUi (this);
}

GalPatternWidget::~GalPatternWidget ()
{
}


void GalPatternWidget::ConnectSlots()
{
  QObject::connect (Pattern_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(PatternIndexChanged(int)));

    QObject::connect (PatternNewButton, SIGNAL(clicked()), this, SLOT(PatternNew_clicked()));
     QObject::connect (PatternEditButton, SIGNAL(clicked()), this, SLOT(PatternEdit_clicked()));
     QObject::connect (PatternLoadButton, SIGNAL(clicked()), this, SLOT(PatternLoad_clicked()));
     QObject::connect (PatternSaveButton, SIGNAL(clicked()), this, SLOT(PatternSave_clicked()));
     QObject::connect (PatternApplyButton, SIGNAL(clicked()), this, SLOT(PatternApply_clicked()));
     QObject::connect ( PatternEditCancelButton, SIGNAL(clicked()), this, SLOT(PatternEditCancel_clicked()));
     QObject::connect ( PatternDeleteButton, SIGNAL(clicked()), this, SLOT(PatternDelete_clicked()));

}

void GalPatternWidget::PatternIndexChanged (int ix)
{
  GAPG_LOCK_SLOT;
  //std::cout << "GalPatternWidget::PatternIDChanged  ix="<<ix << std::endl;
  RefreshPatternIndex(ix);
  //std::cout<<"SequenceIDChanged gets sequence :"<<std::hex<< (ulong) seq<< ", id:"<<std::dec << seq->Id()<<", name:"<<seq->Name()<< std::endl;
  GAPG_UNLOCK_SLOT;
}




void GalPatternWidget::PatternNew_clicked()
{
  std::cout << "GalPatternWidget::PatternNew_clicked"<< std::endl;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
    bool ok=false;
    // automatic assignment of new id here: begin with id from index
    size_t pid= theSetup->NumKnownPatterns()+1;
    while (theSetup->GetPattern(pid)!=0) pid++;

    QString defaultname=QString("Pattern_%1").arg(pid);
    QString patname = QInputDialog::getText(this, tr("Create a new bit pattern"),
                                           tr("Pattern name:"), QLineEdit::Normal,
                                           defaultname, &ok);
    if (!ok || patname.isEmpty()) return;
    GalapagosPattern pat(pid,patname.toLatin1().constData());
    pat.AddByte(0xFF); // some default value to fill editor
    theSetup->AddPattern(pat);



    GAPG_LOCK_SLOT;
    fParent->RefreshView();
    GAPG_UNLOCK_SLOT;
    Pattern_comboBox->setCurrentIndex(pid-1);
    PatternEdit_clicked();
}

void GalPatternWidget::PatternEdit_clicked()
{
  //std::cout << "GalPatternWidget::PatternEdit_clicked"<< std::endl;
  oktetabyteview->setReadOnly(false);
   PatternApplyButton->setEnabled(true);
   PatternEditCancelButton->setEnabled(true);
   Pattern_comboBox->setEnabled(false);
   PatternIDSpinBox->setEnabled(true);

}

void GalPatternWidget::PatternLoad_clicked()
{
  //std::cout << "GalPatternWidget::PatternLoad_clicked"<< std::endl;
  QFileDialog fd( this,
                      "Select Files with New Galapagos bit pattern",
                      fLastFileDir,
                      QString("Galapagos Pattern files (*.gap);;All files (*.*)"));

  fd.setFileMode( QFileDialog::ExistingFiles);

  if ( fd.exec() != QDialog::Accepted ) return;
  QStringList list = fd.selectedFiles();
  QStringList::Iterator fit = list.begin();
  while( fit != list.end() ) {
    QString fileName = *fit;
    fLastFileDir = QFileInfo(fileName).absolutePath();
    if(!LoadPattern(fileName))
    {
      printm("Pattern load sees error with %s",fileName.toLatin1().data());
    }
    ++fit;
  }
  GAPG_LOCK_SLOT;
    fParent->RefreshView(); // populate comboboxes with all known sequences
  GAPG_UNLOCK_SLOT;
}


void GalPatternWidget::PatternSave_clicked()
{
  //std::cout << "GalPatternWidget::PatternSave_clicked"<< std::endl;
  QFileDialog fd( this,
                       "Save Galapagos bit pattern to file",
                       fLastFileDir,
                       QString("Galapagos Pattern files (*.gap)"));
   fd.setFileMode( QFileDialog::AnyFile);
   fd.setAcceptMode(QFileDialog::AcceptSave);
   QString defname=Pattern_comboBox->currentText();
   defname.append(".gap");
   fd.selectFile(defname);
   if (fd.exec() != QDialog::Accepted) return;
   QStringList flst = fd.selectedFiles();
   if (flst.isEmpty()) return;
   theSetup_GET_FOR_CLASS(GalapagosSetup);
   QString fileName = flst[0];
   fLastFileDir = fd.directory().path();
   int ix=Pattern_comboBox->currentIndex();
    GalapagosPattern* pat=theSetup->GetKnownPattern(ix);
     if(pat==0)  {
         printm("NEVER COME HERE:unknown  pattern for index %d!",ix);
         return;
     }

   if(!SavePattern(fileName,pat)){
     printm("Could not save pattern of index %d to file %s!",ix,fileName.toLatin1().constData());
   }



}

void GalPatternWidget::PatternApply_clicked()
{
  //std::cout << "GalPatternWidget::PatternApply_clicked"<< std::endl;
  oktetabyteview->setReadOnly(true);
  PatternApplyButton->setEnabled(false);
  PatternIDSpinBox->setEnabled(false);

   theSetup_GET_FOR_CLASS(GalapagosSetup);
   int ix=Pattern_comboBox->currentIndex();
   GalapagosPattern* pat=theSetup->GetKnownPattern(ix);
   if(pat==0)  {
     fParent->ShowStatusMessage("NEVER COME HERE: Pattern id not known in setup!");
     return;
    }
   // evaluate bytarray from model and put it to our pattern object:
   oktetabyteview->selectAll(true);
   QByteArray theData=oktetabyteview->selectedData();
   pat->Clear();
   pat->SetId(PatternIDSpinBox->value());
   size_t numbytes=theData.size();
   for(int i=0; i<numbytes; ++i)
   {
     pat->AddByte(theData[i]);
   }

   Pattern_comboBox->setEnabled(true);

}

void GalPatternWidget::PatternEditCancel_clicked()
{
  //std::cout << "GalPatternWidget::PatternEditCancel_clicked"<< std::endl;
  int ix=Pattern_comboBox->currentIndex();
  PatternIndexChanged(ix);
  oktetabyteview->setReadOnly(true);
  PatternApplyButton->setEnabled(false);
  PatternEditCancelButton->setEnabled(false);
  Pattern_comboBox->setEnabled(true);
  PatternIDSpinBox->setEnabled(false);



}


void  GalPatternWidget::PatternDelete_clicked()
{
  //std::cout << "GalPatternWidget::PatternDelete_clicked"<< std::endl;
  if(QMessageBox::question( this, fImpName, "Really Delete current pattern from list?",
      QMessageBox::Yes | QMessageBox::No ,
      QMessageBox::Yes) != QMessageBox::Yes ) {
    return;
  }
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int ix=Pattern_comboBox->currentIndex();
  theSetup->RemoveKnownPattern(ix);

  GAPG_LOCK_SLOT
  fParent->RefreshView();
  GAPG_UNLOCK_SLOT

}


void GalPatternWidget::EvaluateView ()
{


}

void GalPatternWidget::RefreshPatternIndex(int ix)
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
   GalapagosPattern* pat=theSetup->GetKnownPattern(ix);
   if(pat==0)  {
       printm("Warning: unknown pattern index %d in combobox, NEVER COME HERE!!",ix);
       return;
   }

   // here provide okteta model from our pattern data:

   // provide tempory bytearray:
   QByteArray theByteArray;
   size_t numbytes=pat->NumBytes();
   for(int c=0; c<numbytes; ++c)
     theByteArray.append(pat->GetByte(c));

   Okteta::PieceTableByteArrayModel* theByteArrayModel =
       new Okteta::PieceTableByteArrayModel(theByteArray, oktetabyteview);



   oktetabyteview->setByteArrayModel(theByteArrayModel);
   oktetabyteview->setReadOnly(true);
   oktetabyteview->setOverwriteMode(false);

   PatternIDSpinBox->setValue(pat->Id());
}




void GalPatternWidget::RefreshView ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int oldpat=Pattern_comboBox->currentIndex(); // remember our active item
  Pattern_comboBox->clear();
  for (int pix=0; pix<theSetup->NumKnownPatterns(); ++pix)
   {
    GalapagosPattern* pat=theSetup->GetKnownPattern(pix);
    if(pat==0)  continue;
    Pattern_comboBox->addItem(pat->Name());
   }
  Pattern_comboBox->setCurrentIndex(oldpat); // restore active item
  RefreshPatternIndex(oldpat);

  oktetabyteview->setValueCoding(fParent->GetNumberBase()==16 ? Okteta::AbstractByteArrayView::HexadecimalCoding : Okteta::AbstractByteArrayView::BinaryCoding);


}







bool GalPatternWidget::LoadPattern(const QString& fileName)
{
  QFile pfile(fileName);
   if (!pfile.open( QIODevice::ReadOnly))
     {
     printm ("!!! Could not open sequence file %s", fileName.toLatin1().constData());
     return false;
     }
   theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
   QString patname=fileName.split("/").last();
   patname.chop(4);
   //std::cout << "Loading sequence from file "<< seqname.toLatin1().constData()<< std::endl;
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


bool GalPatternWidget::SavePattern(const QString& fileName, GalapagosPattern* pat)
{
  // TODO: optionally change id when saving a pattern?
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
    QByteArray byteArray=pat->GetByteArray();

//    size_t bytesize=byteArray.size()
//    outStream << (qint32) bytesize; // pattern length information
//    outStream.writeRawData(byteArray.data(), byteArray.size()); // the actual pattern data
    outStream.writeBytes(byteArray.data(), byteArray.size()); // the actual pattern data


    outStream << (qint32) PATTERN_FILE_TAG; // GALAPAGOS gap format identifier trailer

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
    if (!LoadPattern (patfilename))
      printm ("Warning: Pattern %s from setup could not be loaded!", patfilename.toLatin1 ().data ());
  }
  int oldpix = 0;    // later take from settings
  Pattern_comboBox->setCurrentIndex (oldpix);    // toggle refresh the editor?

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
    SavePattern (patfilename, pat);
  }
  settings->setValue ("Numpatterns", (int) theSetup->NumKnownPatterns ());

}


