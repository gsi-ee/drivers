#include "GalPackageWidget.h"
#include "GalapagosSetup.h"

#include "BasicGui.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include <QString>
#include <QVBoxLayout>

namespace gapg
{

GalPackageWidget::GalPackageWidget (QWidget* parent) :
     gapg::BasicObjectEditorWidget (parent)
{
  fCoreWidgets.clear();
  fPackageEditor = new gapg::GalPackageEditor (this);

  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->setSpacing(-1);
  vbox->setContentsMargins(1,1,1,1);
  for(int ix=0; ix<GAPG_CORES; ++ix)
  {
    GalCoreWidget* core=new GalCoreWidget(ix, this, fPackageEditor);
    fCoreEnabledRadio[ix]=core->Core_enabled_radio;
    fCoreActiveLED[ix]=core->Core_active_LED;
    fCoreKernelCombo[ix]=  core->CoreKernel_comboBox;
    fCoreWidgets.push_back(core);
    vbox->addWidget(core);
  } // for ix
  //vbox->addStretch(1);
  fPackageEditor->CoresAreaWidget->setLayout(vbox);
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

  CancelEditing();
}

GalPackageWidget::~GalPackageWidget ()
{
}



void GalPackageWidget::ConnectSlots ()
{
  BasicObjectEditorWidget::ConnectSlots();

  QObject::connect (fPackageEditor->GeneratorActiveButton, SIGNAL(toggled(bool)), this, SLOT(GeneratorActive_clicked(bool)));
  QObject::connect (fPackageEditor->CoresSimulateButton, SIGNAL(clicked()), this, SLOT(CoresSimulate_clicked()));
  QObject::connect (fPackageEditor->GeneratorNewStartButton, SIGNAL(clicked()), this, SLOT(GeneratorNewStart_clicked()));

 QObject::connect (fPackageEditor->Core_enabled_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(CoreEnabled_toggled_all(bool)));
 QObject::connect (fPackageEditor->Core_sequence_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this, SLOT(CoreKernel_changed_all(int)));
 
}


bool GalPackageWidget::NewObjectRequest ()
{
  //std::cout << "GalPackageWidget:: NewObjectRequest "<< std::endl;
  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  bool ok = false;
//  // automatic assignment of new id here: begin with id from index
  size_t sid = theSetup->NumKnownPackages () + 1;
  while (theSetup->GetPackage (sid) != 0)
    sid++;
//
  QString defaultname = QString ("Package_%1").arg (sid);
  QString pakname = QInputDialog::getText (this, tr ("Create a new package"), tr ("Package name:"), QLineEdit::Normal,
      defaultname, &ok);
  if (!ok || pakname.isEmpty ())
    return false;
  GalapagosPackage pak (sid, pakname.toLatin1 ().constData ());
  theSetup->AddPackage (pak);
  return true;
}

void GalPackageWidget::StartEditing ()
{
  //std::cout << "GalPackageWidget:: KernelEdit_clicked"<< std::endl;
  fPackageEditor->CoresAreaWidget->setEnabled (true);
  fPackageEditor->Cores_all_frame->setEnabled (true);
  fPackageEditor->CoresControlframe->setEnabled (false);
}

void GalPackageWidget::CancelEditing ()
{
  //std::cout << "GalPackageWidget:: CancelEditing"<< std::endl;
  fPackageEditor->CoresAreaWidget->setEnabled (false);
  fPackageEditor->Cores_all_frame->setEnabled (false);
  fPackageEditor->CoresControlframe->setEnabled (true);
}

bool GalPackageWidget::DeleteObjectRequest ()
{
  //std::cout << "GalPackageWidget:: DeleteObjectRequest"<< std::endl;
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
 // std::cout << "GalPackageWidget::ApplyEditing()"<< std::endl;

  fPackageEditor->CoresAreaWidget->setEnabled (false);
  fPackageEditor->Cores_all_frame->setEnabled (false);
  fPackageEditor->CoresControlframe->setEnabled (true);

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


    for (uint8_t core = 0; core < GAPG_CORES; ++core)
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

        // TODO: assign the core type to the kernel here? or always fixed type depending on index?
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


  GalapagosPackage pak (0, pakname.toLatin1 ().constData ());    // package id will be taken from file
  QByteArray content = sfile.readAll ();
  QList<QByteArray> inlines = content.split (QChar::LineFeed);
  QList<QByteArray>::const_iterator lit;
  bool hasId = false;
  for (lit = inlines.constBegin (); lit != inlines.constEnd (); ++lit)
  {
    QByteArray oneline = *lit;
    //std::cout<< "   scanning line "<<oneline.data() << std::endl;
    QString line (oneline);
    if (!hasId)
    {
      QString line (oneline);
      if (!line.contains ("PackageID"))
        continue;
      //std::cout<< "package id keyword was found!"<< std::endl;
      bool ok = false;
      QString value = line.split ("=").last ();
      int sid = value.toInt (&ok);
      if (!ok)
        continue;
      //std::cout<< "Found package id "<<sid << std::endl;
      pak.SetId (sid);
      hasId = true;
    }
    if (line.contains ("#"))
      continue;

    // here parse the kernels of the package line by line:
    QList<QByteArray> params = oneline.split (QChar::Space);
    if(params.size()<4) break;
    QByteArray score=params[0];
    QString core (score);
    int corenum=core.toInt();
    QByteArray sname=params[1];
    QString name(sname);
    QByteArray skid=params[2];
    QString kid(skid);
    int idnum=kid.toInt();
    QByteArray senab=params[3];
    QString enab(senab);
    bool enabled=enab.toInt();

    //std::cout<< "Found setup for core "<<corenum <<": kernel "<<name.toLatin1 ().constData ()<<" of id "<<idnum<<", enabled:"<<enabled << std::endl;
    // here countercheck if kernel id matches the name in current setup:
    GalapagosKernel* ker = theSetup->GetKernel(idnum);
    if (ker == 0)
    {
      printm ("GalPackageWidget::LoadObject warning - core %d wants to be assigned to kernel of id %d that is not known!!", corenum,idnum);
      continue;
    }
    if(QString(ker->Name()) != name)
    {
        printm ("GalPackageWidget::LoadObject warning - core %d has assigned kernel id %d of name %s that does not match to existing kernel name %s of that id!!!", corenum,idnum, name.toLatin1 ().constData (), ker->Name());
        continue;
    }

    pak.SetKernelID(corenum,idnum);
    pak.SetCoreEnabled(corenum,enabled);
  }


  if (pak.Id () == 0)
  {
    printm ("GalPackageWidget::LoadObject %s error: could not read package ID!", pakname.toLatin1 ().constData ());
    return false;
  }


  GalapagosPackage* oldpak = 0;
  if ((oldpak= theSetup->GetPackage (pak.Id ())) != 0)
  {
    printm ("GalPackageWidget::LoadObject %s warning: overwriting existing package %s with same unique id %d as the new package %s.",
        pakname.toLatin1 ().constData (), oldpak->Name (), pak.Id (), pak.Name());
    theSetup->RemovePackageById(pak.Id());
  }
  theSetup->AddPackage (pak);
  printm ("Package %s of id %d has been loaded from file.", pakname.toLatin1 ().constData (), pak.Id () );
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

  theSetup_GET_FOR_CLASS_RETURN_BOOL(GalapagosSetup);
  QString header = QString ("#Package %1 saved on %2").arg (pak->Name ()).arg (
      QDateTime::currentDateTime ().toString ("dd.MM.yyyy - hh:mm:ss."));
  QString idtag = QString ("#PackageID = %1").arg (pak->Id ());
  sfile.write (header.toLatin1 ().constData ());
  sfile.write ("\n");
  sfile.write (idtag.toLatin1 ().constData ());
  sfile.write ("\n");
  for (uint8_t core = 0; core < GAPG_CORES; ++core)
    {
      bool enabled = pak->IsCoreEnabled (core);
      uint32_t kid=pak->GetKernelID (core);
      if (kid == 0)
      {
        printm ("GalPackageWidget::SaveObject Never come here - core %d has no kernel in setup !", core);
        continue;
      }
      GalapagosKernel* ker = theSetup->GetKernel(kid);
      if (ker == 0)
      {
        printm ("GalPackageWidget::SaveObject warning - core %d has assigned kernel of id %d that is not known!!", core,kid);
        continue;
      }
      QString kline= QString("%1 %2 %3 %4").arg(core).arg (ker->Name ()).arg(kid).arg(enabled);
      sfile.write (kline.toLatin1 ().constData ());
      sfile.write ("\n");
    }
  printm ("Package %s of id %d has been saved to file %s", pak->Name(), pak->Id () ,fileName.toLatin1 ().constData ());
  return true;
}



int GalPackageWidget::RefreshObjectIndex (int ix)
{
  theSetup_GET_FOR_CLASS_RETURN(GalapagosSetup);
  GalapagosPackage* pak = theSetup->GetKnownPackage (ix);
  if (pak == 0)
  {
    printm ("GalPackageWidget::RefreshObjectIndex Warning: unknown package index %d in combobox, NEVER COME HERE!!", ix);
    return -1;
  }
  //std::cout<< "GalPackageWidget::RefreshObjectIndex for "<< ix <<" with package "<< pak->Name() <<std::endl;
  // now refresh the combobox, radiobutton and lamps from configured kernel
  bool isrunning = theSetup->IsGeneratorActive ();

  // TODO: later each galcorewidget will refresh separately like this?


    for (uint8_t core = 0; core <GAPG_CORES; ++core)
    {

      fCoreWidgets[core]->RefreshCoreType(pak->GetCoreType(core));

      bool enabled = pak->IsCoreEnabled (core);
      fCoreEnabledRadio[core]->setChecked (enabled);

      QColor lampcolor;
      if (enabled && isrunning)
        lampcolor = QColor (Qt::green);
      else if (enabled)
        lampcolor = QColor (Qt::yellow);
      else
        lampcolor = QColor (Qt::red);

      fCoreActiveLED[core]->setColor (lampcolor);

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

    int numpaks = settings->value ("/Numpackages", 1).toInt ();
    for (int six = 0; six < numpaks; ++six)    // 2 do not reload the default entries again
    {
      QString settingsname = QString ("/Packages/%1").arg (six);
      QString pakfilename = settings->value (settingsname).toString ();
      //std::cout<< " GalPackageWidget::ReadSettings() will load package file"<<pakfilename.toLatin1().data()<< std::endl;
      if (!LoadObject (pakfilename))
        printm ("Warning: Package %s from setup could not be loaded!", pakfilename.toLatin1 ().data ());
    }
    int oldix = 0;    // later take from settings
    Object_comboBox->setCurrentIndex (oldix);    // toggle refresh the editor?
    ObjectIndexChanged (oldix);
}

void GalPackageWidget::WriteSettings (QSettings* settings)
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);

    for (int six = 0; six < theSetup->NumKnownPackages() ; ++six)
    {
      GalapagosPackage* pak = theSetup->GetKnownPackage (six);
      if (pak == 0)
        continue;
      QString settingsname = QString ("/Packages/%1").arg (six);
      QString pakfilename = QString ("%1.gac").arg (pak->Name ());
      settings->setValue (settingsname, pakfilename);
      //std::cout<< " GalPackageWidget::WriteSettings() saves package file "<<pakfilename.toLatin1().data()<< std::endl;
      SaveObject (pakfilename, pak);
    }
    settings->setValue ("Numpackages", (int) theSetup->NumKnownPackages ());

}





void GalPackageWidget::GeneratorActive_clicked (bool checked)
{
//std::cout<< "GeneratorActive_clicked with checked="<< checked << std::endl;
// when generator is active, we disable to change the current package:
ObjectProtect_enabled(checked);

GAPG_AUTOAPPLY(ApplyGeneratorActive (checked));

}


void GalPackageWidget::CoresSimulate_clicked ()
{
  std::cout<< "CoresSimulate_clicked"<< std::endl;
}

void GalPackageWidget::GeneratorNewStart_clicked()
{
  //std::cout<< "GeneratorNewStart_clicked" << std::endl;

  // TODO: here compile current package first:
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int cix=theSetup->GetCurrentPackageIndex();
  printm ("Compiling current package of index %d", cix);
  bool rev=theSetup->CompilePackage(cix);
  if(!rev)
  {
    printm ("ERROR on compilation of package! Do not start generator!");
    return;
  }
  fPackageEditor->GeneratorActiveButton->setChecked(true);
}


void GalPackageWidget::CoreEnabled_toggled_all (bool on)
{
for (int chan = 0; chan < GAPG_CORES; ++chan)
  fCoreEnabledRadio[chan]->setChecked (on);
}

void GalPackageWidget::CoreEnabled_toggled (int core, bool on)
{
GAPG_LOCK_SLOT;
GAPG_AUTOAPPLY(ApplyCoreEnabled (core, on));
GAPG_UNLOCK_SLOT;
}





void GalPackageWidget::CoreKernel_changed_all (int ix)
{
for (int chan = 0; chan < GAPG_CORES; ++chan)
  fCoreKernelCombo[chan]->setCurrentIndex (ix);
}

void GalPackageWidget::CoreKernel_changed (int core, int ix)
{
GAPG_LOCK_SLOT;
//std::cout << "GalPackageWidget::CoreKernel_changed ch="<<core<<",  ix="<<ix << std::endl;
GAPG_AUTOAPPLY(ApplyCoreKernel(core, ix));
GAPG_UNLOCK_SLOT;
}



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
   // note: colors of package lamps are done in RefreshObjectIndex()

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

// setup combobox entries from known packages:
  fPackageEditor->Core_sequence_comboBox_ALL->clear ();
  for (int six = 0; six < theSetup->NumKnownKernels (); ++six)
  {
    GalapagosKernel* ker = theSetup->GetKnownKernel (six);
    if (ker == 0)
      continue;
    for (uint8_t core = 0; core < GAPG_CORES; ++core)
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
//  GAPG_LOCK_SLOT
  RefreshView();
//  GAPG_UNLOCK_SLOT

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

