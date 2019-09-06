#include "GalPackageWidget.h"
#include "GalapagosSetup.h"

#include "BasicGui.h"

#include <QMessageBox>
#include <QFileDialog>

namespace gapg
{

GalPackageWidget::GalPackageWidget (QWidget* parent) :
     gapg::BasicObjectEditorWidget (parent)
{

  fPackageEditor = new gapg::GalPackageEditor (this);
   Editor_scrollArea->setWidget (fPackageEditor);
   setWindowTitle("Package of core setup");

   Object_comboBox->setToolTip("Select known package by name");
   ObjectIDSpinBox-> setToolTip("Unique id of selected package");

   ObjectNewButton->setToolTip("Create new package");
   ObjectEditButton->setToolTip("Edit contents of selected package");
   ObjectDeleteButton->setToolTip("Remove selected package");
   ObjectLoadButton->setToolTip("Load Core package from *.gac (galapagos cores) file to the list");
   ObjectSaveButton->setToolTip("Export selected Core package to a galapagos cores file (*.gac)");
   ObjectEditCancelButton->setToolTip("Cancel edited code for selected package (restore last setup)");
   ObjectApplyButton->setToolTip("Apply editor contents for selected package");


  fCoreEnabledRadio[0] = fPackageEditor->Core_enabled_radio_00;
  fCoreEnabledRadio[1] = fPackageEditor->Core_enabled_radio_01;
  fCoreEnabledRadio[2] = fPackageEditor->Core_enabled_radio_02;
  fCoreEnabledRadio[3] = fPackageEditor->Core_enabled_radio_03;
  fCoreEnabledRadio[4] = fPackageEditor->Core_enabled_radio_04;
  fCoreEnabledRadio[5] = fPackageEditor->Core_enabled_radio_05;
  fCoreEnabledRadio[6] = fPackageEditor->Core_enabled_radio_06;
  fCoreEnabledRadio[7] = fPackageEditor->Core_enabled_radio_07;
  fCoreEnabledRadio[8] = fPackageEditor->Core_enabled_radio_08;
  fCoreEnabledRadio[9] = fPackageEditor->Core_enabled_radio_09;
  fCoreEnabledRadio[10] = fPackageEditor->Core_enabled_radio_10;
  fCoreEnabledRadio[11] = fPackageEditor->Core_enabled_radio_11;
  fCoreEnabledRadio[12] = fPackageEditor->Core_enabled_radio_12;
  fCoreEnabledRadio[13] = fPackageEditor->Core_enabled_radio_13;
  fCoreEnabledRadio[14] = fPackageEditor->Core_enabled_radio_14;
  fCoreEnabledRadio[15] = fPackageEditor->Core_enabled_radio_15;

  fCoreActiveLED[0] = fPackageEditor->Core_active_LED_00;
  fCoreActiveLED[1] = fPackageEditor->Core_active_LED_01;
  fCoreActiveLED[2] = fPackageEditor->Core_active_LED_02;
  fCoreActiveLED[3] = fPackageEditor->Core_active_LED_03;
  fCoreActiveLED[4] = fPackageEditor->Core_active_LED_04;
  fCoreActiveLED[5] = fPackageEditor->Core_active_LED_05;
  fCoreActiveLED[6] = fPackageEditor->Core_active_LED_06;
  fCoreActiveLED[7] = fPackageEditor->Core_active_LED_07;
  fCoreActiveLED[8] = fPackageEditor->Core_active_LED_08;
  fCoreActiveLED[9] = fPackageEditor->Core_active_LED_09;
  fCoreActiveLED[10] = fPackageEditor->Core_active_LED_10;
  fCoreActiveLED[11] = fPackageEditor->Core_active_LED_11;
  fCoreActiveLED[12] = fPackageEditor->Core_active_LED_12;
  fCoreActiveLED[13] = fPackageEditor->Core_active_LED_13;
  fCoreActiveLED[14] = fPackageEditor->Core_active_LED_14;
  fCoreActiveLED[15] = fPackageEditor->Core_active_LED_15;

//  fCoreSimulatedRadio[0] = fPackageEditor->Core_simulate_radio_00;
//  fCoreSimulatedRadio[1] = fPackageEditor->Core_simulate_radio_01;
//  fCoreSimulatedRadio[2] = fPackageEditor->Core_simulate_radio_02;
//  fCoreSimulatedRadio[3] = fPackageEditor->Core_simulate_radio_03;
//  fCoreSimulatedRadio[4] = fPackageEditor->Core_simulate_radio_04;
//  fCoreSimulatedRadio[5] = fPackageEditor->Core_simulate_radio_05;
//  fCoreSimulatedRadio[6] = fPackageEditor->Core_simulate_radio_06;
//  fCoreSimulatedRadio[7] = fPackageEditor->Core_simulate_radio_07;
//  fCoreSimulatedRadio[8] = fPackageEditor->Core_simulate_radio_08;
//  fCoreSimulatedRadio[9] = fPackageEditor->Core_simulate_radio_09;
//  fCoreSimulatedRadio[10] = fPackageEditor->Core_simulate_radio_10;
//  fCoreSimulatedRadio[11] = fPackageEditor->Core_simulate_radio_11;
//  fCoreSimulatedRadio[12] = fPackageEditor->Core_simulate_radio_12;
//  fCoreSimulatedRadio[13] = fPackageEditor->Core_simulate_radio_13;
//  fCoreSimulatedRadio[14] = fPackageEditor->Core_simulate_radio_14;
//  fCoreSimulatedRadio[15] = fPackageEditor->Core_simulate_radio_15;

  fCoreKernelCombo[0] = fPackageEditor->Core_sequence_comboBox_00;
  fCoreKernelCombo[1] = fPackageEditor->Core_sequence_comboBox_01;
  fCoreKernelCombo[2] = fPackageEditor->Core_sequence_comboBox_02;
  fCoreKernelCombo[3] = fPackageEditor->Core_sequence_comboBox_03;
  fCoreKernelCombo[4] = fPackageEditor->Core_sequence_comboBox_04;
  fCoreKernelCombo[5] = fPackageEditor->Core_sequence_comboBox_05;
  fCoreKernelCombo[6] = fPackageEditor->Core_sequence_comboBox_06;
  fCoreKernelCombo[7] = fPackageEditor->Core_sequence_comboBox_07;
  fCoreKernelCombo[8] = fPackageEditor->Core_sequence_comboBox_08;
  fCoreKernelCombo[9] = fPackageEditor->Core_sequence_comboBox_09;
  fCoreKernelCombo[10] = fPackageEditor->Core_sequence_comboBox_10;
  fCoreKernelCombo[11] = fPackageEditor->Core_sequence_comboBox_11;
  fCoreKernelCombo[12] = fPackageEditor->Core_sequence_comboBox_12;
  fCoreKernelCombo[13] = fPackageEditor->Core_sequence_comboBox_13;
  fCoreKernelCombo[14] = fPackageEditor->Core_sequence_comboBox_14;
  fCoreKernelCombo[15] = fPackageEditor->Core_sequence_comboBox_15;

//  fCorePatternCombo[0] = fPackageEditor->Core_pattern_comboBox_00;
//  fCorePatternCombo[1] = fPackageEditor->Core_pattern_comboBox_01;
//  fCorePatternCombo[2] = fPackageEditor->Core_pattern_comboBox_02;
//  fCorePatternCombo[3] = fPackageEditor->Core_pattern_comboBox_03;
//  fCorePatternCombo[4] = fPackageEditor->Core_pattern_comboBox_04;
//  fCorePatternCombo[5] = fPackageEditor->Core_pattern_comboBox_05;
//  fCorePatternCombo[6] = fPackageEditor->Core_pattern_comboBox_06;
//  fCorePatternCombo[7] = fPackageEditor->Core_pattern_comboBox_07;
//  fCorePatternCombo[8] = fPackageEditor->Core_pattern_comboBox_08;
//  fCorePatternCombo[9] = fPackageEditor->Core_pattern_comboBox_09;
//  fCorePatternCombo[10] = fPackageEditor->Core_pattern_comboBox_10;
//  fCorePatternCombo[11] = fPackageEditor->Core_pattern_comboBox_11;
//  fCorePatternCombo[12] = fPackageEditor->Core_pattern_comboBox_12;
//  fCorePatternCombo[13] = fPackageEditor->Core_pattern_comboBox_13;
//  fCorePatternCombo[14] = fPackageEditor->Core_pattern_comboBox_14;
//  fCorePatternCombo[15] = fPackageEditor->Core_pattern_comboBox_15;

}

GalPackageWidget::~GalPackageWidget ()
{
}

void GalPackageWidget::ConnectSlots ()
{
  BasicObjectEditorWidget::ConnectSlots();

  QObject::connect (fPackageEditor->GeneratorActiveButton, SIGNAL(clicked(bool)), this, SLOT(GeneratorActive_clicked(bool)));

  QObject::connect (fPackageEditor->Core_enabled_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(CoreEnabled_toggled_all(bool)));
  GALAGUI_CONNECT_TOGGLED_16(fPackageEditor->Core_enabled_radio_, CoreEnabled_toggled_);

//  QObject::connect (fPackageEditor->Core_simulate_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(CoreSimulated_toggled_all(bool)));
//  GALAGUI_CONNECT_TOGGLED_16(fPackageEditor->Core_simulate_radio_, CoreSimulated_toggled_);

  QObject::connect (fPackageEditor->Core_sequence_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this, SLOT(CoreKernel_changed_all(int)));
  GALAGUI_CONNECT_INDEXCHANGED_16(fPackageEditor->Core_sequence_comboBox_,CoreKernel_changed_);

//  QObject::connect (fPackageEditor->Core_pattern_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this, SLOT(CorePattern_changed_all(int)));
//  GALAGUI_CONNECT_INDEXCHANGED_16(fPackageEditor->Core_pattern_comboBox_,CorePattern_changed_);

QObject::connect (fPackageEditor->CoregroupBox_0, SIGNAL(toggled(bool)), this, SLOT(CoreEnabled_toggled_group0(bool)));

QObject::connect (fPackageEditor->CoregroupBox_1, SIGNAL(toggled(bool)), this, SLOT(CoreEnabled_toggled_group1(bool)));
}


bool GalPackageWidget::NewObjectRequest ()
{
  std::cout << "GalPackageWidget:: NewObjectRequest "<< std::endl;
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  bool ok = false;
//  // automatic assignment of new id here: begin with id from index
//  size_t sid = theSetup->NumKnownKernels () + 1;
//  while (theSetup->GetKernel (sid) != 0)
//    sid++;
//
//  QString defaultname = QString ("Kernel_%1").arg (sid);
//  QString seqname = QInputDialog::getText (this, tr ("Create a new kernel"), tr ("Kernel name:"), QLineEdit::Normal,
//      defaultname, &ok);
//  if (!ok || seqname.isEmpty ())
//    return false;
//  GalapagosKernel seq (sid, seqname.toLatin1 ().constData ());
//  seq.AddCommand ("NOP");
//  seq.Compile ();
//  theSetup->AddKernel (seq);
  return true;
}

void GalPackageWidget::StartEditing ()
{
  //std::cout << "GalPackageWidget:: KernelEdit_clicked"<< std::endl;
  fPackageEditor->Cores_tabWidget->setEnabled (true);
  fPackageEditor->Cores_all_frame->setEnabled (true);
}

void GalPackageWidget::CancelEditing ()
{
  std::cout << "GalPackageWidget:: CancelEditing"<< std::endl;
  fPackageEditor->Cores_tabWidget->setEnabled (false);
  fPackageEditor->Cores_all_frame->setEnabled (false);

  //fKernelEditor->KernelTextEdit->setReadOnly (true);
}

bool GalPackageWidget::DeleteObjectRequest ()
{
  std::cout << "GalPackageWidget:: DeleteObjectRequest"<< std::endl;
  if (QMessageBox::question (this, fImpName, "Really Delete current package from list?",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
  {
    return false;
  }

  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  int ix = Object_comboBox->currentIndex ();
  theSetup->RemoveKnownPackage (ix);
  return true;
}

bool GalPackageWidget::LoadObjectRequest ()
{
  std::cout << "GalPackageWidget::LoadObjectRequest()"<< std::endl;
  QFileDialog fd (this, "Select Files with New Galapagos core package", fLastFileDir,
      QString ("Galapagos core package files (*.gac);;All files (*.*)"));

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
      printm ("Package load sees error with %s", fileName.toLatin1 ().data ());
    }
    ++fit;
  }
  return true;
}

bool GalPackageWidget::SaveObjectRequest ()
{
  //std::cout << "GalPackageWidget::KernelSave_clicked"<< std::endl;
  QFileDialog fd (this, "Save Galapagos core package to file", fLastFileDir,
      QString ("Galapagos core package files (*.gac)"));
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);
  QString defname = Object_comboBox->currentText ();
  defname.append (".gac");
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
  // TODO
  GalapagosPackage* pak = theSetup->GetKnownPackage (ix);
  if (pak == 0)
  {
    printm ("NEVER COME HERE:unknown package for index %d!", ix);
    return false;
  }

  if (!SaveObject (fileName, pak))
  {
    printm ("Could not save package of index %d to file %s!", ix, fileName.toLatin1 ().constData ());
    return false;
  }
  return true;
}

void GalPackageWidget::ApplyEditing ()
{
  std::cout << "GalPackageWidget::ApplyEditing()"<< std::endl;

  fPackageEditor->Cores_tabWidget->setEnabled (false);
  fPackageEditor->Cores_all_frame->setEnabled (false);
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int ix = Object_comboBox->currentIndex ();
  GalapagosPackage* pak = theSetup->GetKnownPackage (ix);
  if (pak == 0)
  {
    fParent->ShowStatusMessage ("NEVER COME HERE: package id not known in setup!");
    return;
  }

    pak->Clear ();
    pak->SetId (ObjectIDSpinBox->value ());


    for (uint8_t core = 0; core < 16; ++core)
      {
        bool enabled = fCoreEnabledRadio[core]->isChecked ();
        pak->SetCoreEnabled(core,enabled);

        size_t ix=fCoreKernelCombo[core]->currentIndex ();
        GalapagosKernel* ker=theSetup->GetKnownKernel(ix);
        if(ker==0)
        {
            printm ("GalPackageWidget::ApplyEditing () Never come here - core %d has no kernel of index %d in setup !", core, ix);
            continue;
        }
        pak->SetKernelID(core, ker->Id());
      }

}

bool GalPackageWidget::LoadObject (const QString& fileName)
{
  //TODO: later redefine kernel script format with some html tags?
  QFile sfile (fileName);
  if (!sfile.open (QIODevice::ReadOnly))
  {
    printm ("!!! Could not open package file %s", fileName.toLatin1 ().constData ());
    return false;
  }
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  QString pakname = fileName.split ("/").last ();
  pakname.chop (4);
  //std::cout << "Loading kernel from file "<< seqname.toLatin1().constData()<< std::endl;

// TODO:specify format to describe the package setup! maybe ASCII using the names?


//  GalapagosKernel seq (0, seqname.toLatin1 ().constData ());    // kernel id will be taken from file
//  QByteArray content = sfile.readAll ();
//  QList<QByteArray> commands = content.split (QChar::LineFeed);
//  QList<QByteArray>::const_iterator cit;
//  bool hasKernelId = false;
//  for (cit = commands.constBegin (); cit != commands.constEnd (); ++cit)
//  {
//    QByteArray cmd = *cit;
//    //std::cout<< "   scanning line "<<cmd.data() << std::endl;
//    QString line (cmd);
//    if (!hasKernelId)
//    {
//      QString line (cmd);
//      if (!line.contains ("KernelID"))
//        continue;
//      //std::cout<< "kernel id keyword was found!"<< std::endl;
//      bool ok = false;
//      QString value = line.split ("=").last ();
//      int sid = value.toInt (&ok);
//      if (!ok)
//        continue;
//      //std::cout<< "Found kernel id "<<sid << std::endl;
//      seq.SetId (sid);
//      hasKernelId = true;
//    }
//    if (line.contains ("#"))
//      continue;
//    //cmd.append(";");
//    seq.AddCommand (cmd.data ());
//    //std::cout<< "   Added command "<<cmd.data() << std::endl;
//  }
//  if (seq.Id () == 0)
//  {
//    printm ("LoadKernel %s error: could not read kernel ID!", seqname.toLatin1 ().constData ());
//    return false;
//  }
//
//  seq.Compile ();
//  GalapagosKernel* oldseq = 0;
//  if ((oldseq = theSetup->GetKernel (seq.Id ())) != 0)
//  {
//    printm ("LoadKernel %s error: kernel %s had already assigned specified unique id %d !",
//        seqname.toLatin1 ().constData (), oldseq->Name (), seq.Id ());
//    return false;
//  }
//  theSetup->AddKernel (seq);
  return true;
}

bool GalPackageWidget::SaveObject (const QString& fileName, BasicObject* ob)
{
  //TODO: later redefine kernel script format with some html tags?
  GalapagosPackage* pak = dynamic_cast<GalapagosPackage*> (ob);
  if (pak == 0)
    return false;
  QFile sfile (fileName);
  if (!sfile.open (QIODevice::WriteOnly))
  {
    printm ("!!! Could not open file %s", fileName.toLatin1 ().constData ());
    return false;
  }

  // TODO:specify format to describe the package setup! maybe ASCII using the names?


//  QString header = QString ("#Kernel %1 saved on %2").arg (seq->Name ()).arg (
//      QDateTime::currentDateTime ().toString ("dd.MM.yyyy - hh:mm:ss."));
//  QString idtag = QString ("#KernelID = %1").arg (seq->Id ());
//  sfile.write (header.toLatin1 ().constData ());
//  sfile.write ("\n");
//  sfile.write (idtag.toLatin1 ().constData ());
//  sfile.write ("\n");
//  const char* line = 0;
//  int l = 0;
//  while ((line = seq->GetCommandLine (l++)) != 0)
//  {
//    //std::cout<<"KernelSave_clicked writes line:"<<line  << std::endl;
//    sfile.write (line);
//    sfile.write ("\n");
//  }
  return true;
}

//void GalPackageWidget::EvaluateView ()
//{
//
//}

int GalPackageWidget::RefreshObjectIndex (int ix)
{
//  fKernelEditor->KernelTextEdit->clear ();
//
//  // now take out commands from known kernels:
  theSetup_GET_FOR_CLASS_RETURN(GalapagosSetup);
  GalapagosPackage* pak = theSetup->GetKnownPackage (ix);
  if (pak == 0)
  {
    printm ("GalPackageWidget::RefreshObjectIndex Warning: unknown package index %d in combobox, NEVER COME HERE!!", ix);
    return -1;
  }

  // now refresh the combobox from configured kernel
    for (uint8_t core = 0; core < 16; ++core)
    {
      bool enabled = pak->IsCoreEnabled (core);
      fCoreEnabledRadio[core]->setChecked (enabled);

//      GalapagosKernel* ker = pak->GetKernel (core);
//      if (ker == 0)
//      {
//        printm ("Never come here - core %d has no kernel in setup !", core);
//        continue;
//      }

      uint32_t kid=pak->GetKernelID (core);
      if (kid == 0)
            {
              printm ("GalPackageWidget::RefreshObjectIndex  Never come here - core %d has no kernel in setup !", core);
              continue;
            }
      GalapagosKernel* ker = theSetup->GetKernel(kid);
      if (ker == 0)
        {
          printm ("GalPackageWidget::RefreshObjectIndex warning - core %d has assigned kernel of id %d that is not known!!", core,kid);
          continue;
        }

      // maybe paranoid, but this is countercheck if gui has been refreshed corretly:
      int cix = fCoreKernelCombo[core]->findText (QString (ker->Name ()));
      if (cix < 0)
        printm ("RefreshObjectIndex  Never come here - core %d has no combobox sequence entry %s", core, ker->Name ());
      else
        fCoreKernelCombo[core]->setCurrentIndex (cix);
    }

   theSetup->SetCurrentPackageIndex(ix);


  return pak->Id();
}



void GalPackageWidget::ReadSettings (QSettings* settings)
{
//  int numseqs = settings->value ("/Numpackages", 1).toInt ();
//  for (int six = 3; six < numseqs; ++six)    // do not reload the default entries again
//  {
//    QString settingsname = QString ("/Packages/%1").arg (six);
//    QString seqfilename = settings->value (settingsname).toString ();
//    //std::cout<< " GalapagosGui::ReasdSettings() will load kernel file"<<seqfilename.toLatin1().data()<< std::endl;
//    if (!LoadObject (seqfilename))
//      printm ("Warning: Package %s from setup could not be loaded!", seqfilename.toLatin1 ().data ());
//  }
//  int oldix = 0;    // later take from settings
//  Object_comboBox->setCurrentIndex (oldix);    // toggle refresh the editor?
//  ObjectIndexChanged (oldix);
}

void GalPackageWidget::WriteSettings (QSettings* settings)
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
//  for (int six = 0; six < theSetup->NumKnownKernels (); ++six)
//  {
//    GalapagosKernel* seq = theSetup->GetKnownKernel (six);
//    if (seq == 0)
//      continue;
//    QString settingsname = QString ("/Kernels/%1").arg (six);
//    QString seqfilename = QString ("%1.gak").arg (seq->Name ());
//    settings->setValue (settingsname, seqfilename);
//    //std::cout<< " GalPackageWidget::WriteSettings() saves kernel file"<<seqfilename.toLatin1().data()<< std::endl;
//    SaveObject (seqfilename, seq);
//  }
//  settings->setValue ("Numkernels", (int) theSetup->NumKnownKernels ());

}





void GalPackageWidget::GeneratorActive_clicked (bool checked)
{
//std::cout<< "GeneratorActive_clicked with checked="<< checked << std::endl;
GAPG_AUTOAPPLY(ApplyGeneratorActive (checked));

}

void GalPackageWidget::CoreEnabled_toggled_all (bool on)
{
for (int chan = 0; chan < 16; ++chan)
  fCoreEnabledRadio[chan]->setChecked (on);
}

void GalPackageWidget::CoreEnabled_toggled (int core, bool on)
{
GAPG_AUTOAPPLY(ApplyCoreEnabled (core, on));
}

void GalPackageWidget::CoreEnabled_toggled_group0 (bool on)
{
for (int chan = 0; chan < 8; ++chan)
  fCoreEnabledRadio[chan]->setChecked (on);
}

void GalPackageWidget::CoreEnabled_toggled_group1 (bool on)
{
for (int chan = 8; chan < 16; ++chan)
  fCoreEnabledRadio[chan]->setChecked (on);
}

GALAGUI_IMPLEMENT_MULTICHANNEL_TOGGLED_16(GalPackageWidget, CoreEnabled);




void GalPackageWidget::CoreKernel_changed_all (int ix)
{
for (int chan = 0; chan < 16; ++chan)
  fCoreKernelCombo[chan]->setCurrentIndex (ix);
}

void GalPackageWidget::CoreKernel_changed (int core, int ix)
{
GAPG_LOCK_SLOT;
//std::cout << "GalPackageWidget::CoreKernel_changed ch="<<core<<",  ix="<<ix << std::endl;
GAPG_AUTOAPPLY(ApplyCoreKernel(core, ix));
GAPG_UNLOCK_SLOT;
}

GALAGUI_IMPLEMENT_MULTICHANNEL_CHANGED_16(GalPackageWidget, CoreKernel);


void GalPackageWidget::EvaluateView ()
{
  //std::cout << "GalPackageWidget::EvaluateView"<<std::endl;

  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_CLASS(GalapagosSetup);

  theSetup->SetGeneratorActive (fPackageEditor->GeneratorActiveButton->isChecked ());

  // we do not change the editor content in setup here unless we have been in editor mode!


}

void GalPackageWidget::RefreshView ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  // first update some status display, if desired:
   bool isrunning = theSetup->IsGeneratorActive ();

   fPackageEditor->Core_active_LED_ALL->setColor (isrunning ? QColor (Qt::green) : QColor (Qt::red));

   for (uint8_t core = 0; core < 16; ++core)
   {
     // change leds depending on enabled and running state
     bool enabled = theSetup->IsCoreRunning (core);
     //fCoreEnabledRadio[core]->setChecked (enabled);
     QColor lampcolor;
     if (enabled && isrunning)
       lampcolor = QColor (Qt::green);
     else if (enabled)
       lampcolor = QColor (Qt::yellow);
     else
       lampcolor = QColor (Qt::red);

     fCoreActiveLED[core]->setColor (lampcolor);
   }

  // then populate the editor selection with registered packages:
  int oldsid = Object_comboBox->currentIndex ();    // remember our active item
   Object_comboBox->clear ();
   for (int pax = 0; pax < theSetup->NumKnownPackages(); ++pax)
   {
     GalapagosPackage* pak = theSetup->GetKnownPackage (pax);
     if (pak == 0)
       continue;
     // populate names in kernel editor window:
     Object_comboBox->addItem (pak->Name ());
   }
   if (oldsid >= theSetup->NumKnownPackages ())
     oldsid = theSetup->NumKnownPackages () - 1;
   Object_comboBox->setCurrentIndex (oldsid);    // restore active item


   fPackageEditor->GeneratorActiveButton->setChecked (theSetup->IsGeneratorActive ());
  ;

// setup combobox entries from known packages:
  fPackageEditor->Core_sequence_comboBox_ALL->clear ();
  for (int six = 0; six < theSetup->NumKnownKernels (); ++six)
  {
    GalapagosKernel* ker = theSetup->GetKnownKernel (six);
    if (ker == 0)
      continue;
    for (uint8_t core = 0; core < 16; ++core)
    {
      if (six == 0)
        fCoreKernelCombo[core]->clear ();
      fCoreKernelCombo[core]->addItem (ker->Name ());
    }
    fPackageEditor->Core_sequence_comboBox_ALL->addItem (ker->Name ());
  }


  // this will refresh the display of the currently edited package:
  RefreshObjectIndex (oldsid);



}

void GalPackageWidget::ApplyGeneratorActive (bool on)
{
//   std::cout << "GalPackageWidget::ApplyGeneratorActive on="<<on << std::endl;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  theSetup->SetGeneratorActive (on);
  /* TODO: this should be common function, or will be handled by setting whole controlregister from structure*/
  uint32_t controlword = 0;
  if (theSetup->IsGeneratorActive ())
    controlword |= GAPG_BIT_MAIN_ENABLE;
  else
    controlword &= ~GAPG_BIT_MAIN_ENABLE;

  fParent->WriteGAPG ( GAPG_MAIN_CONTROL, controlword);
  GAPG_LOCK_SLOT;
  fParent->GetRegisters ();
  RefreshView ();
  GAPG_UNLOCK_SLOT;

}

void GalPackageWidget::ApplyCoreEnabled (int core, bool on)
{
  //std::cout << "GalPackageWidget::ApplyEnabled chan="<<core<<",  on="<<on << std::endl;
    theSetup_GET_FOR_CLASS(GalapagosSetup);
    theSetup->SetCoreEnabled (core, on);

    // TODO: possible to enable current core on the fly here?
//  fParent->WriteGAPG ( GAPG_CHANNEL_ENABLE_LOW, theSetup->GetCoreStatus_0 ());
//  fParent->WriteGAPG ( GAPG_CHANNEL_ENABLE_HI, theSetup->GetCoreControl_1 ());
  GAPG_LOCK_SLOT
  RefreshView();
  GAPG_UNLOCK_SLOT

}



void GalPackageWidget::ApplyCoreKernel (int core, int ix)
{

// std::cout << "GalPackageWidget::ApplyCoreKernel chan="<<core<<",  ix="<<ix << std::endl;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  const char* seqname = fCoreKernelCombo[core]->itemText (ix).toLatin1 ().data ();
  bool rev = theSetup->SetCoreKernel (core, seqname);
  if (!rev)
    printm ("ApplyCoreKernel Warning: kernel %s of current core %d not known", seqname, core);

  // TODO: for interactive mode, need to compile complete package,
  // do upload to hardware and optionally run it?

  //theSetup->SetCoreKernel (core, seqname);

  // NO Autoapply of kernels on hardware!
  //fParent->WriteGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + core * sizeof(uint32_t),
  //    theSetup->GetCoreKernelID (core));
}


}    // namespace

