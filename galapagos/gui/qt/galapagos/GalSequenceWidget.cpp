#include "GalSequenceWidget.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QSettings>
#include <QDateTime>


GalSequenceWidget::GalSequenceWidget (QWidget* parent) :
  GalSubWidget(parent)
{
  setupUi (this);
}

GalSequenceWidget::~GalSequenceWidget ()
{
}

void GalSequenceWidget::ConnectSlots()
{

  QObject::connect (Sequence_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SequenceIndexChanged(int)));

  QObject::connect (SequenceNewButton, SIGNAL(clicked()), this, SLOT(SequenceNew_clicked()));
  QObject::connect (SequenceEditButton, SIGNAL(clicked()), this, SLOT(SequenceEdit_clicked()));
  QObject::connect (SequenceLoadButton, SIGNAL(clicked()), this, SLOT(SequenceLoad_clicked()));
  QObject::connect (SequenceSaveButton, SIGNAL(clicked()), this, SLOT(SequenceSave_clicked()));
  QObject::connect (SequenceApplyButton, SIGNAL(clicked()), this, SLOT(SequenceApply_clicked()));
  QObject::connect ( SequenceEditCancelButton, SIGNAL(clicked()), this, SLOT(SequenceEditCancel_clicked()));
  QObject::connect ( SequenceDeleteButton, SIGNAL(clicked()), this, SLOT(SequenceDelete_clicked()));
}



void GalSequenceWidget::SequenceIndexChanged (int ix)
{
  GAPG_LOCK_SLOT;
  //std::cout << "GalSequenceWidget::SequenceIndexChanged  ix="<<ix << std::endl;

  RefreshSequenceIndex(ix);

  GAPG_UNLOCK_SLOT;
}

void GalSequenceWidget::SequenceNew_clicked()
{
  //std::cout << "GalSequenceWidget:: SequenceNew_clicked "<< std::endl;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  bool ok=false;
  // automatic assignment of new id here: begin with id from index
  size_t sid= theSetup->NumKnownSequences()+1;
  while (theSetup->GetSequence(sid)!=0) sid++;

  QString defaultname=QString("Sequence_%1").arg(sid);
  QString seqname = QInputDialog::getText(this, tr("Create a new sequence"),
                                         tr("Sequence name:"), QLineEdit::Normal,
                                         defaultname, &ok);
  if (!ok || seqname.isEmpty()) return;
  GalapagosSequence seq(sid,seqname.toLatin1().constData());
  seq.AddCommand("NOP");
  seq.Compile();
  theSetup->AddSequence(seq);



  GAPG_LOCK_SLOT;
  fParent->RefreshView();
  GAPG_UNLOCK_SLOT;
  Sequence_comboBox->setCurrentIndex(sid-1);
  SequenceEdit_clicked();

}


void GalSequenceWidget::SequenceEdit_clicked()
{
  //std::cout << "GalSequenceWidget:: SequenceEdit_clicked"<< std::endl;
  SequenceTextEdit->setReadOnly(false);
  SequenceApplyButton->setEnabled(true);
  SequenceEditCancelButton->setEnabled(true);
  Sequence_comboBox->setEnabled(false);
  SequenceIDSpinBox->setEnabled(true);
}

void GalSequenceWidget::SequenceEditCancel_clicked()
{
  //std::cout << "GalSequenceWidget:: SequenceEditCancel_clicked"<< std::endl;
  int ix=Sequence_comboBox->currentIndex();
  SequenceIndexChanged(ix);
  SequenceTextEdit->setReadOnly(true);
  SequenceApplyButton->setEnabled(false);
  SequenceEditCancelButton->setEnabled(false);
  Sequence_comboBox->setEnabled(true);
  SequenceIDSpinBox->setEnabled(false);
}

void  GalSequenceWidget::SequenceDelete_clicked()
{
  //std::cout << "GalSequenceWidget:: SequenceDelete_clicked"<< std::endl;
  if(QMessageBox::question( this, fImpName, "Really Delete current sequence from list?",
       QMessageBox::Yes | QMessageBox::No ,
       QMessageBox::Yes) != QMessageBox::Yes ) {
     return;
   }
   theSetup_GET_FOR_CLASS(GalapagosSetup);
   int ix=Sequence_comboBox->currentIndex();
   theSetup->RemoveKnownSequence(ix);

   GAPG_LOCK_SLOT
   fParent->RefreshView();
   GAPG_UNLOCK_SLOT
}


void GalSequenceWidget::SequenceLoad_clicked()
{
  //std::cout << "GalSequenceWidget::SequenceLoad_clicked"<< std::endl;
  QFileDialog fd( this,
                     "Select Files with New Galapagos command sequence",
                     fLastFileDir,
                     QString("Galapagos Sequence files (*.gas);;All files (*.*)"));

     fd.setFileMode( QFileDialog::ExistingFiles);

     if ( fd.exec() != QDialog::Accepted ) return;
     QStringList list = fd.selectedFiles();
     QStringList::Iterator fit = list.begin();
     while( fit != list.end() ) {
        QString fileName = *fit;
        fLastFileDir = QFileInfo(fileName).absolutePath();
        if(!LoadSequence(fileName))
        {
           printm("Sequence load sees error with %s",fileName.toLatin1().data());
        }
        ++fit;
     }
 GAPG_LOCK_SLOT;
     fParent->RefreshView(); // populate comboboxes with all known sequences also in other subwindows!
 GAPG_UNLOCK_SLOT;

}

void GalSequenceWidget::SequenceSave_clicked()
{
  //std::cout << "GalSequenceWidget::SequenceSave_clicked"<< std::endl;
  QFileDialog fd( this,
                      "Save Galapagos command sequence to file",
                      fLastFileDir,
                      QString("Galapagos Sequence files (*.gas)"));
  fd.setFileMode( QFileDialog::AnyFile);
  fd.setAcceptMode(QFileDialog::AcceptSave);
  QString defname=Sequence_comboBox->currentText();
  defname.append(".gas");
  fd.selectFile(defname);
  if (fd.exec() != QDialog::Accepted) return;
  QStringList flst = fd.selectedFiles();
  if (flst.isEmpty()) return;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  QString fileName = flst[0];
  fLastFileDir = fd.directory().path();
  int ix=Sequence_comboBox->currentIndex();
   GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
    if(seq==0)  {
        printm("NEVER COME HERE:unknown  sequence for index %d!",ix);
        return;
    }

  if(!SaveSequence(fileName,seq)){
    printm("Could not save sequence of index %d to file %s!",ix,fileName.toLatin1().constData());
  }
}

void GalSequenceWidget::SequenceApply_clicked()
{
  //std::cout << "GalSequenceWidget::SequenceOK_clicked"<< std::endl;
  SequenceTextEdit->setReadOnly(true);
  SequenceApplyButton->setEnabled(false);
  SequenceIDSpinBox->setEnabled(false);


  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int ix=Sequence_comboBox->currentIndex();
  GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
  if(seq==0)  {
    fParent->ShowStatusMessage("NEVER COME HERE: sequence id not known in setup!");
    return;
   }
  seq->Clear();
  seq->SetId(SequenceIDSpinBox->value());
  const char* line=0;
   int l=0;
   QString theCode=SequenceTextEdit->toPlainText();
   //std::cout<< "got editor code: "<<theCode.toLatin1().constData() << std::endl;
   QStringList commands = theCode.split(QChar::LineFeed);
   QStringList::const_iterator it;
   for (it = commands.constBegin(); it != commands.constEnd(); ++it)
      {
        QString cmd=*it;
        //cmd.append(";");
        seq->AddCommand(cmd.toLatin1().constData());
        //std::cout<< "   Added command "<<cmd.toLatin1().constData() << std::endl;
      }
   seq->Compile(); // TODO: here we may check if sequence is valid and give feedback output
   Sequence_comboBox->setEnabled(true);
}





bool GalSequenceWidget::LoadSequence(const QString& fileName)
{
  //TODO: later redefine sequence script format with some html tags?
  QFile sfile(fileName);
  if (!sfile.open( QIODevice::ReadOnly))
    {
    printm ("!!! Could not open sequence file %s", fileName.toLatin1().constData());
    return false;
    }
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  QString seqname=fileName.split("/").last();
  seqname.chop(4);
  //std::cout << "Loading sequence from file "<< seqname.toLatin1().constData()<< std::endl;
  GalapagosSequence seq(0, seqname.toLatin1().constData()); // sequence id will be taken from file
  QByteArray content = sfile. readAll();
  QList<QByteArray> commands=content.split(QChar::LineFeed);
  QList<QByteArray>::const_iterator cit;
  bool hasSequenceId=false;
  for (cit = commands.constBegin(); cit != commands.constEnd(); ++cit)
  {
    QByteArray cmd=*cit;
    //std::cout<< "   scanning line "<<cmd.data() << std::endl;
    QString line(cmd);
    if(!hasSequenceId)
      {
      QString line(cmd);
      if(!line.contains("SequenceID")) continue;
      //std::cout<< "sequence id keyword was found!"<< std::endl;
      bool ok=false;
      QString value=line.split("=").last();
      int sid=value.toInt(&ok);
      if(!ok) continue;
      //std::cout<< "Found sequence id "<<sid << std::endl;
      seq.SetId(sid);
      hasSequenceId=true;
    }
    if(line.contains("#")) continue;
    //cmd.append(";");
    seq.AddCommand(cmd.data());
    //std::cout<< "   Added command "<<cmd.data() << std::endl;
  }
  if(seq.Id()==0)
  {
    printm("LoadSequence %s error: could not read sequence ID!",seqname.toLatin1().constData());
    return false;
  }

  seq.Compile();
  GalapagosSequence* oldseq=0;
  if((oldseq=theSetup->GetSequence(seq.Id()))!=0)
  {
    printm("LoadSequence %s error: sequence %s had already assigned specified unique id %d !",seqname.toLatin1().constData(), oldseq->Name(), seq.Id());
    return false;
  }
  theSetup->AddSequence(seq);
  return true;
}

bool GalSequenceWidget::SaveSequence(const QString& fileName, GalapagosSequence* seq)
{
  //TODO: later redefine sequence script format with some html tags?
  if(seq==0) return false;
  QFile sfile(fileName);
    if (!sfile.open( QIODevice::WriteOnly))
    {
      printm ("!!! Could not open file %s", fileName.toLatin1().constData());
      return false;
    }
   QString header= QString("#Sequence %1 saved on %2").arg(seq->Name()).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy - hh:mm:ss."));
   QString idtag= QString("#SequenceID = %1").arg(seq->Id());
   sfile.write(header.toLatin1().constData());
   sfile.write("\n");
   sfile.write(idtag.toLatin1().constData());
   sfile.write("\n");
  const char* line=0;
     int l=0;
     while ((line=seq->GetCommandLine(l++)) !=0)
       {
         //std::cout<<"SequenceSave_clicked writes line:"<<line  << std::endl;
         sfile.write(line);
         sfile.write("\n");
       }
  return true;
}




void GalSequenceWidget::EvaluateView ()
{


}


void GalSequenceWidget::RefreshSequenceIndex(int ix)
{
  SequenceTextEdit->clear();

   // now take out commands from known sequences:
   theSetup_GET_FOR_CLASS(GalapagosSetup);
   GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
   if(seq==0)  {
       //SequenceTextEdit->appendPlainText("unknown ID!");
       printm("Warning: unknown sequence index %d in combobox, NEVER COME HERE!!",ix);
       return;
   }
   //std::cout<<"RefreshSequenceIndex gets sequence :"<<std::hex<< (ulong) seq<< ", id:"<<std::dec << seq->Id()<<", name:"<<seq->Name()<< std::endl;
   const char* line=0;
   int l=0;
   while ((line=seq->GetCommandLine(l++)) !=0)
     {
       //std::cout<<"RefreshSequenceIndex reading  line:"<<line  << std::endl;
       QString txt(line);
       SequenceTextEdit->appendPlainText(txt);
     }
   SequenceIDSpinBox->setValue(seq->Id());
}

void GalSequenceWidget::RefreshView ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int oldsid=Sequence_comboBox->currentIndex(); // remember our active item
   Sequence_comboBox->clear();
   for (int six=0; six<theSetup->NumKnownSequences(); ++six)
   {
     GalapagosSequence* seq=theSetup->GetKnownSequence(six);
     if(seq==0)  continue;
     // populate names in sequence editor window:
     Sequence_comboBox->addItem(seq->Name());
   }
   Sequence_comboBox->setCurrentIndex(oldsid); // restore active item
   RefreshSequenceIndex(oldsid);

}



void GalSequenceWidget::ReadSettings (QSettings* settings)
{
  int numseqs = settings->value ("/Numsequences", 1).toInt ();
  for (int six = 3; six < numseqs; ++six)    // do not reload the default entries again
  {
    QString settingsname = QString ("/Sequences/%1").arg (six);
    QString seqfilename = settings->value (settingsname).toString ();
    //std::cout<< " GalapagosGui::ReasdSettings() will load sequence file"<<seqfilename.toLatin1().data()<< std::endl;
    if (!LoadSequence (seqfilename))
      printm ("Warning: Sequence %s from setup could not be loaded!", seqfilename.toLatin1 ().data ());
  }
  int oldix = 0;    // later take from settings
  Sequence_comboBox->setCurrentIndex (oldix);    // toggle refresh the editor?
  SequenceIndexChanged (oldix);
}

void GalSequenceWidget::WriteSettings (QSettings* settings)
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  for (int six = 0; six < theSetup->NumKnownSequences (); ++six)
  {
    GalapagosSequence* seq = theSetup->GetKnownSequence (six);
    if (seq == 0)
      continue;
    QString settingsname = QString ("/Sequences/%1").arg (six);
    QString seqfilename = QString ("%1.gas").arg (seq->Name ());
    settings->setValue (settingsname, seqfilename);
    //std::cout<< " GalSequenceWidget::WriteSettings() saves sequence file"<<seqfilename.toLatin1().data()<< std::endl;
    SaveSequence (seqfilename, seq);
  }
  settings->setValue ("Numsequences", (int) theSetup->NumKnownSequences ());

}







