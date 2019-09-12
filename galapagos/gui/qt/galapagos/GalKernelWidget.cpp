#include "GalKernelWidget.h"

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
#include "GalKernelEditor.h"

#ifdef USE_GALAPAGOS_LIB
extern "C"
{
#include "galapagos/libgalapagos.h"
}

#endif

namespace gapg
{

GalKernelWidget::GalKernelWidget (QWidget* parent) :
    gapg::BasicObjectEditorWidget (parent)
{
  fKernelEditor = new gapg::GalKernelEditor (this);
  Editor_scrollArea->setWidget (fKernelEditor);
  setWindowTitle("Kernel code");

  Object_comboBox->setToolTip("Select known kernel by name");
  ObjectIDSpinBox-> setToolTip("Unique id of selected kernel");

  ObjectNewButton->setToolTip("Create new kernel");
  ObjectEditButton->setToolTip("Edit contents of selected kernel");
  ObjectDeleteButton->setToolTip("Remove selected kernel");
  ObjectLoadButton->setToolTip("Load kernel from *.gas file to the list");
  ObjectSaveButton->setToolTip("Export selected kernel to a file (*.gas)");
  ObjectEditCancelButton->setToolTip("Cancel edited code for selected kernel (restore last setup)");
  ObjectApplyButton->setToolTip("Apply editor contents for selected kernel");

#ifdef USE_GALAPAGOS_LIB
  // here populate list of available commands:
  fKernelEditor->CommandPrototype_comboBox->clear();
  std::cout<<"GalKernelWidget ctor sees galapagos commands in list: "<<::galapagos_numcommands << std::endl;

   for(int c=0; c<galapagos_numcommands; ++c)
   {
     galapagos_cmd& theCommand= ::galapagos_commandlist[c];
     fKernelEditor->CommandPrototype_comboBox->addItem(theCommand.commandname);

   }
#endif





  fKernelEditor->setEnabled(false);

}

GalKernelWidget::~GalKernelWidget ()
{
}

void GalKernelWidget::ConnectSlots ()
{
  BasicObjectEditorWidget::ConnectSlots ();
  // anything more here?
  QObject::connect (fKernelEditor->CommandPrototypeInsertButton, SIGNAL(clicked()), this, SLOT(CommandPrototypeInsert_clicked()));
  QObject::connect (fKernelEditor->CommandPrototype_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(CommandPrototypeIndexChanged(int)));
  QObject::connect (fKernelEditor->PatternLimitsPickButton, SIGNAL(clicked()), this, SLOT(PatternLimitsPick_clicked()));
  QObject::connect (fKernelEditor->PatternLow_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternLow_spinBox_changed(int)));
  QObject::connect (fKernelEditor->PatternHi_spinBox, SIGNAL(valueChanged(int)), this, SLOT(PatternHi_spinBox_changed(int)));
  QObject::connect (fKernelEditor->Pattern_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(PatternIndexChanged(int)));

}

bool GalKernelWidget::NewObjectRequest ()
{
  //std::cout << "GalKernelWidget:: NewObjectRequest "<< std::endl;
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  bool ok = false;
  // automatic assignment of new id here: begin with id from index
  size_t sid = theSetup->NumKnownKernels () + 1;
  while (theSetup->GetKernel (sid) != 0)
    sid++;

  QString defaultname = QString ("Kernel_%1").arg (sid);
  QString seqname = QInputDialog::getText (this, tr ("Create a new kernel"), tr ("Kernel name:"), QLineEdit::Normal,
      defaultname, &ok);
  if (!ok || seqname.isEmpty ())
    return false;
  GalapagosKernel seq (sid, seqname.toLatin1 ().constData ());
  seq.AddCommand ("NOP");
  seq.Compile ();
  theSetup->AddKernel (seq);
  return true;
}

void GalKernelWidget::StartEditing ()
{
  //std::cout << "GalKernelWidget:: KernelEdit_clicked"<< std::endl;
  //fKernelEditor->KernelTextEdit->setReadOnly (false);

  fKernelEditor->setEnabled (true);
}

void GalKernelWidget::CancelEditing ()
{
  //std::cout << "GalKernelWidget:: CancelEditing"<< std::endl;
  //fKernelEditor->KernelTextEdit->setReadOnly (true);
  fKernelEditor->setEnabled (false);

}

bool GalKernelWidget::DeleteObjectRequest ()
{
  //std::cout << "GalKernelWidget:: KernelDelete_clicked"<< std::endl;
  if (QMessageBox::question (this, fImpName, "Really Delete current kernel from list?",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
  {
    return false;
  }
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  int ix = Object_comboBox->currentIndex ();
  theSetup->RemoveKnownKernel (ix);
  return true;
}

bool GalKernelWidget::LoadObjectRequest ()
{
  //std::cout << "GalKernelWidget::LoadObjectRequest()"<< std::endl;
  QFileDialog fd (this, "Select Files with New Galapagos command kernel", fLastFileDir,
      QString ("Galapagos Kernel files (*.gak);;All files (*.*)"));

  fd.setFileMode (QFileDialog::ExistingFiles);

  if (fd.exec () != QDialog::Accepted)
    return false;
  QStringList list = fd.selectedFiles ();
  QStringList::Iterator fit = list.begin ();
  while (fit != list.end ())
  {
    QString fileName = *fit;
    fLastFileDir = QFileInfo (fileName).absolutePath ();
    if (!LoadObject (fileName))
    {
      printm ("Kernel load sees error with %s", fileName.toLatin1 ().data ());
    }
    ++fit;
  }
  return true;
}

bool GalKernelWidget::SaveObjectRequest ()
{
  //std::cout << "GalKernelWidget::KernelSave_clicked"<< std::endl;
  QFileDialog fd (this, "Save Galapagos command kernel to file", fLastFileDir,
      QString ("Galapagos Kernel files (*.gak)"));
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);
  QString defname = Object_comboBox->currentText ();
  defname.append (".gak");
  fd.selectFile (defname);
  if (fd.exec () != QDialog::Accepted)
    return false;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return false;
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  QString fileName = flst[0];
  fLastFileDir = fd.directory ().path ();
  int ix = Object_comboBox->currentIndex ();
  GalapagosKernel* seq = theSetup->GetKnownKernel (ix);
  if (seq == 0)
  {
    printm ("NEVER COME HERE:unknown  kernel for index %d!", ix);
    return false;
  }

  if (!SaveObject (fileName, seq))
  {
    printm ("Could not save kernel of index %d to file %s!", ix, fileName.toLatin1 ().constData ());
    return false;
  }
  return true;
}

void GalKernelWidget::ApplyEditing ()
{
  //std::cout << "GalKernelWidget::ApplyEditing()"<< std::endl;
  //fKernelEditor->KernelTextEdit->setReadOnly (true);
  fKernelEditor->setEnabled(false);

  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int ix = Object_comboBox->currentIndex ();
  GalapagosKernel* seq = theSetup->GetKnownKernel (ix);
  if (seq == 0)
  {
    fParent->ShowStatusMessage ("NEVER COME HERE: kernel id not known in setup!");
    return;
  }
  seq->Clear ();
  seq->SetId (ObjectIDSpinBox->value ());
  const char* line = 0;
  int l = 0;
  QString theCode = fKernelEditor->KernelTextEdit->toPlainText ();
  //std::cout<< "got editor code: "<<theCode.toLatin1().constData() << std::endl;
  QStringList commands = theCode.split (QChar::LineFeed);
  QStringList::const_iterator it;
  for (it = commands.constBegin (); it != commands.constEnd (); ++it)
  {
    QString cmd = *it;
    //cmd.append(";");
    seq->AddCommand (cmd.toLatin1 ().constData ());
    //std::cout<< "   Added command "<<cmd.toLatin1().constData() << std::endl;
  }
  seq->Compile ();    // TODO: here we may check if kernel is valid and give feedback output
}

bool GalKernelWidget::LoadObject (const QString& fileName)
{
  //TODO: later redefine kernel script format with some html tags?
  QFile sfile (fileName);
  if (!sfile.open (QIODevice::ReadOnly))
  {
    printm ("!!! Could not open kernel file %s", fileName.toLatin1 ().constData ());
    return false;
  }
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  QString seqname = fileName.split ("/").last ();
  seqname.chop (4);
  //std::cout << "Loading kernel from file "<< seqname.toLatin1().constData()<< std::endl;
  GalapagosKernel seq (0, seqname.toLatin1 ().constData ());    // kernel id will be taken from file
  QByteArray content = sfile.readAll ();
  QList<QByteArray> commands = content.split (QChar::LineFeed);
  QList<QByteArray>::const_iterator cit;
  bool hasKernelId = false;
  for (cit = commands.constBegin (); cit != commands.constEnd (); ++cit)
  {
    QByteArray cmd = *cit;
    //std::cout<< "   scanning line "<<cmd.data() << std::endl;
    QString line (cmd);
    if (!hasKernelId)
    {
      QString line (cmd);
      if (!line.contains ("KernelID"))
        continue;
      //std::cout<< "kernel id keyword was found!"<< std::endl;
      bool ok = false;
      QString value = line.split ("=").last ();
      int sid = value.toInt (&ok);
      if (!ok)
        continue;
      //std::cout<< "Found kernel id "<<sid << std::endl;
      seq.SetId (sid);
      hasKernelId = true;
    }
    if (line.contains ("#"))
      continue;
    //cmd.append(";");
    seq.AddCommand (cmd.data ());
    //std::cout<< "   Added command "<<cmd.data() << std::endl;
  }
  if (seq.Id () == 0)
  {
    printm ("LoadKernel %s error: could not read kernel ID!", seqname.toLatin1 ().constData ());
    return false;
  }

  seq.Compile ();
  GalapagosKernel* oldseq = 0;
  if ((oldseq = theSetup->GetKernel (seq.Id ())) != 0)
  {
    printm ("LoadKernel %s error: kernel %s had already assigned specified unique id %d !",
        seqname.toLatin1 ().constData (), oldseq->Name (), seq.Id ());
    return false;
  }
  theSetup->AddKernel (seq);
  return true;
}

bool GalKernelWidget::SaveObject (const QString& fileName, BasicObject* ob)
{
  //TODO: later redefine kernel script format with some html tags?
  GalapagosKernel* seq = dynamic_cast<GalapagosKernel*> (ob);
  if (seq == 0)
    return false;
  QFile sfile (fileName);
  if (!sfile.open (QIODevice::WriteOnly))
  {
    printm ("!!! Could not open file %s", fileName.toLatin1 ().constData ());
    return false;
  }
  QString header = QString ("#Kernel %1 saved on %2").arg (seq->Name ()).arg (
      QDateTime::currentDateTime ().toString ("dd.MM.yyyy - hh:mm:ss."));
  QString idtag = QString ("#KernelID = %1").arg (seq->Id ());
  sfile.write (header.toLatin1 ().constData ());
  sfile.write ("\n");
  sfile.write (idtag.toLatin1 ().constData ());
  sfile.write ("\n");
  const char* line = 0;
  int l = 0;
  while ((line = seq->GetCommandLine (l++)) != 0)
  {
    //std::cout<<"KernelSave_clicked writes line:"<<line  << std::endl;
    sfile.write (line);
    sfile.write ("\n");
  }
  return true;
}

void GalKernelWidget::EvaluateView ()
{

}

int GalKernelWidget::RefreshObjectIndex (int ix)
{
  std::cout << "GalKernelWidget::RefreshObjectIndex "<< ix<< std::endl;
  fKernelEditor->KernelTextEdit->clear ();

  // now take out commands from known kernels:
  theSetup_GET_FOR_CLASS_RETURN(GalapagosSetup);
  GalapagosKernel* ker = theSetup->GetKnownKernel (ix);
  if (ker == 0)
  {
    //KernelTextEdit->appendPlainText("unknown ID!");
    printm ("Warning: unknown kernel index %d in combobox, NEVER COME HERE!!", ix);
    return -1;
  }

  uint32_t pid=ker->GetPatternID();
  GalapagosPattern* pat= theSetup->GetPattern(pid);
  if(pat)
  {
    int pix = fKernelEditor->Pattern_comboBox->findText (QString (pat->Name ()));
    if (pix < 0)
      printm ("GalKernelWidget::RefreshObjectIndex  Never come here - kernel %d has no combobox pattern entry %s", ix, ker->Name ());
    else
      fKernelEditor->Pattern_comboBox->setCurrentIndex (pix);
  }
  else
  {
    printm ("GalKernelWidget::RefreshObjectIndex  ERROR - kernel %d has assigned unknown pattern of unique id %d", ix, pid);
  }

  const char* line = 0;
  int l = 0;
  while ((line = ker->GetCommandLine (l++)) != 0)
  {
    QString txt (line);
    fKernelEditor->KernelTextEdit->appendPlainText (txt);
  }
  return (ker->Id ());
}

void GalKernelWidget::RefreshView ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);

  int oldsid = Object_comboBox->currentIndex ();    // remember our active item
  Object_comboBox->clear ();
  for (int six = 0; six < theSetup->NumKnownKernels (); ++six)
  {
    GalapagosKernel* seq = theSetup->GetKnownKernel (six);
    if (seq == 0)
      continue;
    // populate names in kernel editor window:
    Object_comboBox->addItem (seq->Name ());
  }
  // now populate the combobox of patterns with all known:
  fKernelEditor->Pattern_comboBox->clear();
   for (int pix=0; pix<theSetup->NumKnownPatterns(); ++pix)
    {
     GalapagosPattern* pat=theSetup->GetKnownPattern(pix);
     if(pat==0)  continue;
     fKernelEditor->Pattern_comboBox->addItem(pat->Name());
    }
   // note that actual pattern is selected when refreshing the kernel display

  if (oldsid >= theSetup->NumKnownKernels ())
    oldsid = theSetup->NumKnownKernels () - 1;
  Object_comboBox->setCurrentIndex (oldsid);    // restore active item
  RefreshObjectIndex (oldsid);

}

void GalKernelWidget::ReadSettings (QSettings* settings)
{
  int numseqs = settings->value ("/Numkernels", 1).toInt ();
  for (int six = 3; six < numseqs; ++six)    // do not reload the default entries again
  {
    QString settingsname = QString ("/Kernels/%1").arg (six);
    QString seqfilename = settings->value (settingsname).toString ();
    //std::cout<< " GalapagosGui::ReasdSettings() will load kernel file"<<seqfilename.toLatin1().data()<< std::endl;
    if (!LoadObject (seqfilename))
      printm ("Warning: Kernel %s from setup could not be loaded!", seqfilename.toLatin1 ().data ());
  }
  int oldix = 0;    // later take from settings
  Object_comboBox->setCurrentIndex (oldix);    // toggle refresh the editor?
  ObjectIndexChanged (oldix);
}

void GalKernelWidget::WriteSettings (QSettings* settings)
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  for (int six = 0; six < theSetup->NumKnownKernels (); ++six)
  {
    GalapagosKernel* seq = theSetup->GetKnownKernel (six);
    if (seq == 0)
      continue;
    QString settingsname = QString ("/Kernels/%1").arg (six);
    QString seqfilename = QString ("%1.gak").arg (seq->Name ());
    settings->setValue (settingsname, seqfilename);
    //std::cout<< " GalKernelWidget::WriteSettings() saves kernel file"<<seqfilename.toLatin1().data()<< std::endl;
    SaveObject (seqfilename, seq);
  }
  settings->setValue ("Numkernels", (int) theSetup->NumKnownKernels ());

}

void GalKernelWidget::CommandPrototypeInsert_clicked()
{
  std::cout << "GalKernelWidget::CommandPrototypeInsert_clicked()" << std::endl;
#ifdef USE_GALAPAGOS_LIB
  int ix=fKernelEditor->CommandPrototype_comboBox->currentIndex();
  if(ix>=::galapagos_numcommands)
   {
     printm("NEVER COME HERE: combobox index exceeds galapagos library list of commands!!!");
     ix=::galapagos_numcommands-1;
   }
  galapagos_cmd& theCommand= ::galapagos_commandlist[ix];
  QString suggestion;
  if(theCommand.id==GAPG_RUN_SEQUENCE)
  {
    // special for sequence: use limits from patter pick fields
    suggestion=QString("%1 %2 %3 repetitions").arg(theCommand.commandname).arg(fKernelEditor->PatternLow_spinBox->value()).arg(fKernelEditor->PatternHi_spinBox->value());
  }
  else
  {
    suggestion=QString("%1 %2").arg(theCommand.commandname).arg(theCommand.argnames);
  }
  fKernelEditor->KernelTextEdit->insertPlainText(suggestion);
#endif
}




void GalKernelWidget::CommandPrototypeIndexChanged(int ix)
{
  std::cout << "GalKernelWidget::CommandPrototypeIndexChanged  ix="<<ix << std::endl;

#ifdef USE_GALAPAGOS_LIB
  if(ix>=::galapagos_numcommands)
  {
    printm("NEVER COME HERE: combobox index exceeds galapagos library list of commands!!!");
    ix=galapagos_numcommands-1;
  }

  galapagos_cmd& theCommand= ::galapagos_commandlist[ix];
  fKernelEditor->CommandHelpLabel->setText(theCommand.argnames);
#endif



}



void GalKernelWidget::PatternLimitsPick_clicked()
{
  std::cout << "GalKernelWidget::PatternLimitsPick_clicked()" << std::endl;
}

void GalKernelWidget::PatternLow_spinBox_changed(int val)
{
  std::cout << "GalKernelWidget::PatternLow_spinBox_changed  val="<<val << std::endl;
}

void GalKernelWidget::PatternHi_spinBox_changed(int val)
{
  std::cout << "GalKernelWidget::PatternHi_spinBox_changed  val="<<val << std::endl;
}

void GalKernelWidget::PatternIndexChanged(int ix)
{
  std::cout << "GalKernelWidget::PatternIndexChanged  ix="<<ix << std::endl;
}



}    // gapg

