#include "GalapagosGui.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <QProcess>
#include <stdlib.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>
#include <QMdiSubWindow>

#include <QFile>

#include <sstream>
#include <string.h>
#include <errno.h>
#include <math.h>

// *********************************************************





/*
 *  Constructs a GalapagosGui which is a child of 'parent', with the
 *  name 'name'.'
 */
GalapagosGui::GalapagosGui (QWidget* parent) : BasicGui (parent)
{
 

 fImplementationName="GALAPAGUI";
 fVersionString="Welcome to GalapaGUI!\n\t v0.12 of 22-Aug-2019 by JAM (j.adamczewski@gsi.de)";

 fSettings=new QSettings("GSI", fImplementationName);
 fLastFileDir = QDir::currentPath();

 Qt::WindowFlags wflags= Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint;
 fGalChannelWidget= new GalChannelWidget(this);
 QMdiSubWindow* channels=mdiArea->addSubWindow(fGalChannelWidget,wflags);
 channels->setAttribute(Qt::WA_DeleteOnClose, false);

 fGalSequenceWidget= new GalSequenceWidget(this);
  QMdiSubWindow* seqs=mdiArea->addSubWindow(fGalSequenceWidget,wflags);
  seqs->setAttribute(Qt::WA_DeleteOnClose, false);

  setWindowTitle(QString("%1").arg(fImplementationName));


  QObject::connect (fGalChannelWidget->GeneratorActiveButton, SIGNAL(clicked(bool)), this, SLOT(GeneratorActive_clicked(bool)));


  QObject::connect (fGalChannelWidget->Channel_enabled_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_all(bool)));
  GALAGUI_CONNECT_TOGGLED_16(fGalChannelWidget->Channel_enabled_radio_, ChannelEnabled_toggled_);


  QObject::connect (fGalChannelWidget->Channel_simulate_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelSimulated_toggled_all(bool)));
  GALAGUI_CONNECT_TOGGLED_16(fGalChannelWidget->Channel_simulate_radio_, ChannelSimulated_toggled_);


  QObject::connect (fGalChannelWidget->Channel_sequence_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this,  SLOT(ChannelSequence_changed_all(int)));
  GALAGUI_CONNECT_INDEXCHANGED_16(fGalChannelWidget->Channel_sequence_comboBox_,ChannelSequence_changed_);

  QObject::connect (fGalSequenceWidget->Sequence_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SequenceIDChanged(int)));

  QObject::connect (fGalSequenceWidget->SequenceNewButton, SIGNAL(clicked()), this, SLOT(SequenceNew_clicked()));
  QObject::connect (fGalSequenceWidget->SequenceEditButton, SIGNAL(clicked()), this, SLOT(SequenceEdit_clicked()));
  QObject::connect (fGalSequenceWidget->SequenceLoadButton, SIGNAL(clicked()), this, SLOT(SequenceLoad_clicked()));
  QObject::connect (fGalSequenceWidget->SequenceSaveButton, SIGNAL(clicked()), this, SLOT(SequenceSave_clicked()));
  QObject::connect (fGalSequenceWidget->SequenceApplyButton, SIGNAL(clicked()), this, SLOT(SequenceApply_clicked()));
  QObject::connect (fGalSequenceWidget-> SequenceEditCancelButton, SIGNAL(clicked()), this, SLOT(SequenceEditCancel_clicked()));


//    GALAGUI_ASSIGN_WIDGETS_16(fChannelEnabledRadio);
    //, fGalChannelWidget->Channel_enabled_radio_);


  fChannelEnabledRadio[0] = fGalChannelWidget->Channel_enabled_radio_00;
  fChannelEnabledRadio[1] = fGalChannelWidget->Channel_enabled_radio_01;
  fChannelEnabledRadio[2] = fGalChannelWidget->Channel_enabled_radio_02;
  fChannelEnabledRadio[3] = fGalChannelWidget->Channel_enabled_radio_03;
  fChannelEnabledRadio[4] = fGalChannelWidget->Channel_enabled_radio_04;
  fChannelEnabledRadio[5] = fGalChannelWidget->Channel_enabled_radio_05;
  fChannelEnabledRadio[6] = fGalChannelWidget->Channel_enabled_radio_06;
  fChannelEnabledRadio[7] = fGalChannelWidget->Channel_enabled_radio_07;
  fChannelEnabledRadio[8] = fGalChannelWidget->Channel_enabled_radio_08;
  fChannelEnabledRadio[9] = fGalChannelWidget->Channel_enabled_radio_09;
  fChannelEnabledRadio[10] = fGalChannelWidget->Channel_enabled_radio_10;
  fChannelEnabledRadio[11] = fGalChannelWidget->Channel_enabled_radio_11;
  fChannelEnabledRadio[12] = fGalChannelWidget->Channel_enabled_radio_12;
  fChannelEnabledRadio[13] = fGalChannelWidget->Channel_enabled_radio_13;
  fChannelEnabledRadio[14] = fGalChannelWidget->Channel_enabled_radio_14;
  fChannelEnabledRadio[15] = fGalChannelWidget->Channel_enabled_radio_15;




  fChannelActiveLED[0] = fGalChannelWidget->Channel_active_LED_00;
   fChannelActiveLED[1] = fGalChannelWidget->Channel_active_LED_01;
   fChannelActiveLED[2] = fGalChannelWidget->Channel_active_LED_02;
   fChannelActiveLED[3] = fGalChannelWidget->Channel_active_LED_03;
   fChannelActiveLED[4] = fGalChannelWidget->Channel_active_LED_04;
   fChannelActiveLED[5] = fGalChannelWidget->Channel_active_LED_05;
   fChannelActiveLED[6] = fGalChannelWidget->Channel_active_LED_06;
   fChannelActiveLED[7] = fGalChannelWidget->Channel_active_LED_07;
   fChannelActiveLED[8] = fGalChannelWidget->Channel_active_LED_08;
   fChannelActiveLED[9] = fGalChannelWidget->Channel_active_LED_09;
   fChannelActiveLED[10] = fGalChannelWidget->Channel_active_LED_10;
   fChannelActiveLED[11] = fGalChannelWidget->Channel_active_LED_11;
   fChannelActiveLED[12] = fGalChannelWidget->Channel_active_LED_12;
   fChannelActiveLED[13] = fGalChannelWidget->Channel_active_LED_13;
   fChannelActiveLED[14] = fGalChannelWidget->Channel_active_LED_14;
   fChannelActiveLED[15] = fGalChannelWidget->Channel_active_LED_15;



   fChannelSimulatedRadio[0] = fGalChannelWidget->Channel_simulate_radio_00;
   fChannelSimulatedRadio[1] = fGalChannelWidget->Channel_simulate_radio_01;
   fChannelSimulatedRadio[2] = fGalChannelWidget->Channel_simulate_radio_02;
   fChannelSimulatedRadio[3] = fGalChannelWidget->Channel_simulate_radio_03;
   fChannelSimulatedRadio[4] = fGalChannelWidget->Channel_simulate_radio_04;
   fChannelSimulatedRadio[5] = fGalChannelWidget->Channel_simulate_radio_05;
   fChannelSimulatedRadio[6] = fGalChannelWidget->Channel_simulate_radio_06;
   fChannelSimulatedRadio[7] = fGalChannelWidget->Channel_simulate_radio_07;
   fChannelSimulatedRadio[8] = fGalChannelWidget->Channel_simulate_radio_08;
   fChannelSimulatedRadio[9] = fGalChannelWidget->Channel_simulate_radio_09;
   fChannelSimulatedRadio[10] = fGalChannelWidget->Channel_simulate_radio_10;
   fChannelSimulatedRadio[11] = fGalChannelWidget->Channel_simulate_radio_11;
   fChannelSimulatedRadio[12] = fGalChannelWidget->Channel_simulate_radio_12;
   fChannelSimulatedRadio[13] = fGalChannelWidget->Channel_simulate_radio_13;
   fChannelSimulatedRadio[14] = fGalChannelWidget->Channel_simulate_radio_14;
   fChannelSimulatedRadio[15] = fGalChannelWidget->Channel_simulate_radio_15;

   //fChannelSequenceCombo

   fChannelSequenceCombo[0] = fGalChannelWidget->Channel_sequence_comboBox_00;
   fChannelSequenceCombo[1] = fGalChannelWidget->Channel_sequence_comboBox_01;
   fChannelSequenceCombo[2] = fGalChannelWidget->Channel_sequence_comboBox_02;
   fChannelSequenceCombo[3] = fGalChannelWidget->Channel_sequence_comboBox_03;
   fChannelSequenceCombo[4] = fGalChannelWidget->Channel_sequence_comboBox_04;
   fChannelSequenceCombo[5] = fGalChannelWidget->Channel_sequence_comboBox_05;
   fChannelSequenceCombo[6] = fGalChannelWidget->Channel_sequence_comboBox_06;
   fChannelSequenceCombo[7] = fGalChannelWidget->Channel_sequence_comboBox_07;
   fChannelSequenceCombo[8] = fGalChannelWidget->Channel_sequence_comboBox_08;
   fChannelSequenceCombo[9] = fGalChannelWidget->Channel_sequence_comboBox_09;
   fChannelSequenceCombo[10] = fGalChannelWidget->Channel_sequence_comboBox_10;
   fChannelSequenceCombo[11] = fGalChannelWidget->Channel_sequence_comboBox_11;
   fChannelSequenceCombo[12] = fGalChannelWidget->Channel_sequence_comboBox_12;
   fChannelSequenceCombo[13] = fGalChannelWidget->Channel_sequence_comboBox_13;
   fChannelSequenceCombo[14] = fGalChannelWidget->Channel_sequence_comboBox_14;
   fChannelSequenceCombo[15] = fGalChannelWidget->Channel_sequence_comboBox_15;


  BuildSetup();
  ReadSettings();
  show ();
}

void GalapagosGui::ReadSettings()
{
  BasicGui::ReadSettings();
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  if(fSettings)
    {
    int numseqs=fSettings->value("/Numsequences", 1).toInt();
    for (int six=3; six<numseqs; ++six) // do not reload the default entries again
          {
            QString settingsname=QString("/Sequences/%1").arg(six);
            QString seqfilename=fSettings->value(settingsname).toString();
            std::cout<< " GalapagosGui::ReasdSettings() will load sequence file"<<seqfilename.toLatin1().data()<< std::endl;
            if(!LoadSequence(seqfilename)) printm("Warning: Sequence %s from setup could not be loaded!",seqfilename.toLatin1().data());
          }

    }
}
void GalapagosGui::WriteSettings()
{
  BasicGui::WriteSettings();

  if(fSettings)
    {


      // here setup of patterns and sequences from file
    theSetup_GET_FOR_CLASS(GalapagosSetup);
    for (int six=0; six<theSetup->NumKnownSequences(); ++six)
      {
        GalapagosSequence* seq=theSetup->GetKnownSequence(six);
        if(seq==0)  continue;
        QString settingsname=QString("/Sequences/%1").arg(six);
        QString seqfilename=QString("%1.gas").arg(seq->Name());
        fSettings->setValue(settingsname, seqfilename);
        std::cout<< " GalapagosGui::WriteSettings() saves sequence file"<<seqfilename.toLatin1().data()<< std::endl;
        SaveSequence(seqfilename, seq);
      }
    fSettings->setValue("Numsequences",(int) theSetup->NumKnownSequences());

    }
}
GalapagosGui::~GalapagosGui ()
{
}






void GalapagosGui::EnableI2C ()
{
//  WriteGAPG ( GOS_I2C_DWR, 0x1000080);
//  WriteGAPG ( GOS_I2C_DWR, 0x2000020);
}

void GalapagosGui::DisableI2C ()
{
//  WriteGAPG ( GOS_I2C_DWR, 0x1000000);
}


void GalapagosGui::ResetSlave()
{

  sleep (1);
  printm("Did FAKE Initialize Galapagos Board");
}






void GalapagosGui::Dump()
{

  //  printm("SFP %d DEV:%d :)",fSFP, fSlave);
    for(int adc=0; adc<1; ++adc){
      for (int chan=0; chan<GAPG_CHANNELS; ++chan){
        //int val=ReadADC_Galapagos(adc,chan);
        int val=adc*chan; // dummy
        if(val<0)
          printm("Read error for adc:%d chan:%d",adc,chan);
        else
          {
            if(fNumberBase==16)
              printm("Val (adc:0x%x chan:0x%x)=0x%x",adc,chan,val);
            else
              printm("Val (adc:%d chan:%d)=%d",adc,chan,val);
        }
      }

    }

}

void GalapagosGui::ApplyFileConfig(int )
{
    BasicGui::ApplyFileConfig(900); // adjust bus wait time to 900 us
}


void GalapagosGui::GeneratorActive_clicked(bool checked)
{
  //std::cout<< "GeneratorActive_clicked with checked="<< checked << std::endl;
  GAPG_AUTOAPPLY(ApplyGeneratorActive(checked));

}



void GalapagosGui::ChannelEnabled_toggled_all(bool on)
{
  for(int chan=0;chan<16;++chan)
    fChannelEnabledRadio[chan]->setChecked (on);
}



void GalapagosGui::ChannelEnabled_toggled (int channel, bool on)
{
  GAPG_AUTOAPPLY(ApplyChannelEnabled(channel, on));
}

GALAGUI_IMPLEMENT_MULTICHANNEL_TOGGLED_16(ChannelEnabled);



void GalapagosGui::ChannelSimulated_toggled_all(bool on)
{
  for(int chan=0;chan<16;++chan)
    fChannelSimulatedRadio[chan]->setChecked (on);
}



void GalapagosGui::ChannelSimulated_toggled (int channel, bool on)
{
  GAPG_AUTOAPPLY(ApplyChannelSimulated(channel, on));

}


GALAGUI_IMPLEMENT_MULTICHANNEL_TOGGLED_16(ChannelSimulated);





void GalapagosGui::ChannelSequence_changed_all(int ix)
{
  for(int chan=0;chan<16;++chan)
    fChannelSequenceCombo[chan]->setCurrentIndex (ix);
}

void GalapagosGui::ChannelSequence_changed (int channel, int ix)
{
  GAPG_LOCK_SLOT
  //std::cout << "GalapagosGui::ChannelSequence_changed ch="<<channel<<",  ix="<<ix << std::endl;
  GAPG_AUTOAPPLY(ApplyChannelSequence(channel, ix));
  GAPG_UNLOCK_SLOT
}

GALAGUI_IMPLEMENT_MULTICHANNEL_CHANGED_16(ChannelSequence);


 void GalapagosGui::ApplyGeneratorActive(bool on)
 {
//   std::cout << "GalapagosGui::ApplyGeneratorActive on="<<on << std::endl;
    theSetup_GET_FOR_CLASS(GalapagosSetup);
    theSetup->SetGeneratorActive(on);
    /* TODO: this should be common function, or will be handled by setting whole controlregister from structure*/
    uint32_t controlword=0;
     if(theSetup->IsGeneratorActive())
       controlword |= GAPG_BIT_MAIN_ENABLE;
     else
       controlword &= ~GAPG_BIT_MAIN_ENABLE;

     WriteGAPG ( GAPG_MAIN_CONTROL,  controlword);

     GetRegisters();
     RefreshView();

 }


void GalapagosGui::ApplyChannelEnabled(int channel, bool on)
{
  //std::cout << "GalapagosGui::ApplyEnabled chan="<<channel<<",  on="<<on << std::endl;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
    theSetup->SetChannelEnabled(channel, on);
  WriteGAPG ( GAPG_CHANNEL_ENABLE_LOW, theSetup->GetChannelControl_0());
  WriteGAPG ( GAPG_CHANNEL_ENABLE_HI,  theSetup->GetChannelControl_1());
  RefreshView();

}

void GalapagosGui::ApplyChannelSimulated(int channel, bool on)
{
  //std::cout << "GalapagosGui::ApplySimuated chan="<<channel<<",  on="<<on << std::endl;
  //theSetup_GET_FOR_CLASS(GalapagosSetup);
 //   theSetup->SetChannelSimulated(channel, on);
 // RefreshView();

}

void GalapagosGui::ApplyChannelSequence(int channel, int ix)
{
 // std::cout << "GalapagosGui::ApplyChannelSequence chan="<<channel<<",  ix="<<ix << std::endl;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  const char* seqname= fChannelSequenceCombo[channel]->itemText(ix).toLatin1().data();
  bool rev=theSetup->SetChannelSequence(channel, seqname);
  if(!rev) printm ("ApplyChannelSequence Warning: sequence %s of current channel %d not known",
              seqname,channel);
  theSetup->SetChannelSequence(channel, seqname);
  WriteGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelSequenceID(channel));

 // RefreshView();

}


void GalapagosGui::SequenceIDChanged (int ix)
{
  GAPG_LOCK_SLOT;
  std::cout << "GalapagosGui::SequenceIDChanged  ix="<<ix << std::endl;

  fGalSequenceWidget->SequenceTextEdit->clear();

  // now take out commands from known sequences:
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
  if(seq==0)  {
      fGalSequenceWidget->SequenceTextEdit->appendPlainText("unknown ID!");
      return;
  }
  std::cout<<"SequenceIDChanged gets sequence :"<<std::hex<< (ulong) seq<< ", id:"<<std::dec << seq->Id()<<", name:"<<seq->Name()<< std::endl;
  const char* line=0;
  int l=0;
  while ((line=seq->GetCommandLine(l++)) !=0)
    {
      std::cout<<"SequenceIDChanged reading  line:"<<line  << std::endl;
      QString txt(line);
      fGalSequenceWidget->SequenceTextEdit->appendPlainText(txt);
    }
  GAPG_UNLOCK_SLOT;
}

void GalapagosGui::SequenceNew_clicked()
{
  std::cout << "GalapagosGui:: SequenceNew_clicked "<< std::endl;
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
  RefreshView();
  GAPG_UNLOCK_SLOT;
  fGalSequenceWidget->Sequence_comboBox->setCurrentIndex(sid-1);
  SequenceEdit_clicked();

}


void GalapagosGui::SequenceEdit_clicked()
{
  std::cout << "GalapagosGui:: SequenceEdit_clicked"<< std::endl;
  fGalSequenceWidget->SequenceTextEdit->setReadOnly(false);
  fGalSequenceWidget->SequenceApplyButton->setEnabled(true);
  fGalSequenceWidget->SequenceEditCancelButton->setEnabled(true);
  fGalSequenceWidget->Sequence_comboBox->setEnabled(false);
}

void GalapagosGui::SequenceEditCancel_clicked()
{
  std::cout << "GalapagosGui:: SequenceEditCancel_clicked"<< std::endl;
  int ix=fGalSequenceWidget->Sequence_comboBox->currentIndex();
  SequenceIDChanged(ix);
  fGalSequenceWidget->SequenceTextEdit->setReadOnly(true);
  fGalSequenceWidget->SequenceApplyButton->setEnabled(false);
  fGalSequenceWidget->SequenceEditCancelButton->setEnabled(false);
  fGalSequenceWidget->Sequence_comboBox->setEnabled(true);
}


void GalapagosGui::SequenceLoad_clicked()
{
  std::cout << "GalapagosGui::SequenceLoad_clicked"<< std::endl;
  QFileDialog fd( this,
                     "Select Files with New Galapagos command sequence",
                     fLastFileDir,
                     QString("Galapagos Sequence files (*.gas);;All files (*.*)"));

     fd.setFileMode( QFileDialog::ExistingFiles);

     if ( fd.exec() != QDialog::Accepted ) return;
     //theSetup_GET_FOR_CLASS(GalapagosSetup);
     QStringList list = fd.selectedFiles();
     QStringList::Iterator fit = list.begin();
     //size_t sid= theSetup->NumKnownSequences()+1;
     while( fit != list.end() ) {
        QString fileName = *fit;
        fLastFileDir = QFileInfo(fileName).absolutePath();
        if(!LoadSequence(fileName))
        {
           printm("Sequence load sees error with %s",fileName.toLatin1().data());
        }


//        QFile sfile(fileName);
//        if (!sfile.open( QIODevice::ReadOnly))
//          {
//            printm ("!!! Could not open file %s", fileName.toLatin1().constData());
//            continue;
//          }
//        QString seqname=fileName.split("/").last();
//        seqname.chop(4);
//        std::cout << "Loading sequence from file "<< seqname.toLatin1().constData()<< std::endl;
//        GalapagosSequence seq(sid++, seqname.toLatin1().constData()); // TODO: sequence id also taken from file?
//        QByteArray content = sfile. readAll();
//        QList<QByteArray> commands=content.split(QChar::LineFeed);
//        QList<QByteArray>::const_iterator cit;
//          for (cit = commands.constBegin(); cit != commands.constEnd(); ++cit)
//             {
//               QByteArray cmd=*cit;
//               //cmd.append(";");
//               seq.AddCommand(cmd.data());
//               std::cout<< "   Added command "<<cmd.data() << std::endl;
//             }
//          seq.Compile();
//          theSetup->AddSequence(seq);
//        ++sid;

        ++fit;
     }
 GAPG_LOCK_SLOT;
     RefreshView(); // populate comboboxes with all known sequences
 GAPG_UNLOCK_SLOT;

}

void GalapagosGui::SequenceSave_clicked()
{
  std::cout << "GalapagosGui::SequenceSave_clicked"<< std::endl;
  QFileDialog fd( this,
                      "Save Galapagos command sequence to file",
                      fLastFileDir,
                      QString("Galapagos Sequence files (*.gas)"));
  fd.setFileMode( QFileDialog::AnyFile);
  fd.setAcceptMode(QFileDialog::AcceptSave);
  QString defname=fGalSequenceWidget->Sequence_comboBox->currentText();
  defname.append(".gas");
  fd.selectFile(defname);
  if (fd.exec() != QDialog::Accepted) return;
  QStringList flst = fd.selectedFiles();
  if (flst.isEmpty()) return;
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  QString fileName = flst[0];
  fLastFileDir = fd.directory().path();
  int ix=fGalSequenceWidget->Sequence_comboBox->currentIndex();
   GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
    if(seq==0)  {
        printm("NEVER COME HERE:unknown  sequence for index %d!",ix);
        return;
    }

  if(!SaveSequence(fileName,seq)){
    printm("Could not save sequence of index %d to file %s!",ix,fileName.toLatin1().constData());
  }

//  if (fileName.indexOf(".gas", 0, Qt::CaseInsensitive)<0) fileName+=".gas";
//  theSetup_GET_FOR_CLASS(GalapagosSetup);
//  QFile sfile(fileName);
//  if (!sfile.open( QIODevice::WriteOnly))
//  {
//    printm ("!!! Could not open file %s", fileName.toLatin1().constData());
//    return;
//  }
//
//  int ix=fGalSequenceWidget->Sequence_comboBox->currentIndex();
//  GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
//   if(seq==0)  {
//       printm("unknown  sequence for ID %d!",ix);
//       return;
//   }
//   const char* line=0;
//   int l=0;
//   while ((line=seq->GetCommandLine(l++)) !=0)
//     {
//       std::cout<<"SequenceSave_clicked writes line:"<<line  << std::endl;
//       sfile.write(line);
//       sfile.write("\n");
//     }
}

void GalapagosGui::SequenceApply_clicked()
{
  std::cout << "GalapagosGui::SequenceOK_clicked"<< std::endl;
  fGalSequenceWidget->SequenceTextEdit->setReadOnly(true);
  fGalSequenceWidget->SequenceApplyButton->setEnabled(false);

  // TODO: copy content of editor into list of known sequences.

  theSetup_GET_FOR_CLASS(GalapagosSetup);
  int ix=fGalSequenceWidget->Sequence_comboBox->currentIndex();
  GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
  if(seq==0)  {
    statusBar()->showMessage("NEVER COME HERE: sequence id not known in setup!");
    return;
   }
  seq->Clear();
  const char* line=0;
   int l=0;
   QString theCode=fGalSequenceWidget->SequenceTextEdit->toPlainText();
   std::cout<< "got editor code: "<<theCode.toLatin1().constData() << std::endl;
   QStringList commands = theCode.split(QChar::LineFeed);
   QStringList::const_iterator it;
   for (it = commands.constBegin(); it != commands.constEnd(); ++it)
      {
        QString cmd=*it;
        //cmd.append(";");
        seq->AddCommand(cmd.toLatin1().constData());
        std::cout<< "   Added command "<<cmd.toLatin1().constData() << std::endl;
      }
   seq->Compile(); // TODO: here we may check if sequence is valid and give feedback output
   fGalSequenceWidget->Sequence_comboBox->setEnabled(true);
}


bool GalapagosGui::LoadSequence(const QString& fileName)
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
  std::cout << "Loading sequence from file "<< seqname.toLatin1().constData()<< std::endl;
  GalapagosSequence seq(0, seqname.toLatin1().constData()); // sequence id will be taken from file
  QByteArray content = sfile. readAll();
  QList<QByteArray> commands=content.split(QChar::LineFeed);
  QList<QByteArray>::const_iterator cit;
  bool hasSequenceId=false;
  for (cit = commands.constBegin(); cit != commands.constEnd(); ++cit)
  {
    QByteArray cmd=*cit;
    std::cout<< "   scanning line "<<cmd.data() << std::endl;
    QString line(cmd);
    if(!hasSequenceId)
      {
      QString line(cmd);
      if(!line.contains("SequenceID")) continue;
      std::cout<< "sequence id keyword was found!"<< std::endl;
      bool ok=false;
      QString value=line.split("=").last();
      int sid=value.toInt(&ok);
      if(!ok) continue;
      std::cout<< "Found sequence id "<<sid << std::endl;
      seq.SetId(sid);
      hasSequenceId=true;
    }
    if(line.contains("#")) continue;
    //cmd.append(";");
    seq.AddCommand(cmd.data());
    std::cout<< "   Added command "<<cmd.data() << std::endl;
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

bool GalapagosGui::SaveSequence(const QString& fileName, GalapagosSequence* seq)
{
  //TODO: later redefine sequence script format with some html tags?
  if(seq==0) return false;
  QFile sfile(fileName);
    if (!sfile.open( QIODevice::WriteOnly))
    {
      printm ("!!! Could not open file %s", fileName.toLatin1().constData());
      return false;
    }
   QString header= QString("#Sequence %1 saved on %2").arg(seq->Id()).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy - hh:mm:ss."));
   QString idtag= QString("#SequenceID = %1").arg(seq->Id());
   sfile.write(header.toLatin1().constData());
   sfile.write("\n");
   sfile.write(idtag.toLatin1().constData());
   sfile.write("\n");
  const char* line=0;
     int l=0;
     while ((line=seq->GetCommandLine(l++)) !=0)
       {
         std::cout<<"SequenceSave_clicked writes line:"<<line  << std::endl;
         sfile.write(line);
         sfile.write("\n");
       }
  return true;
}


void GalapagosGui::RefreshView ()
{

  std::cout << "GalapagosGui::RefreshView"<<std::endl;
// display setup structure to gui:

  //GAPG_LOCK_SLOT
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  bool isrunning = theSetup->IsGeneratorActive();

  fGalChannelWidget->Channel_active_LED_ALL->setColor(isrunning ? QColor(Qt::green) : QColor(Qt::red));

  for(uint8_t channel=0; channel<16;++channel)
        {
             // change leds depending on enabled and running state
             bool enabled = theSetup->IsChannelEnabled(channel);
             fChannelEnabledRadio[channel]->setChecked(enabled);
             QColor lampcolor;
             if(enabled && isrunning)
                 lampcolor=QColor(Qt::green);
             else if (enabled)
               lampcolor=QColor(Qt::yellow);
             else
               lampcolor=QColor(Qt::red);

             fChannelActiveLED[channel] ->setColor(lampcolor);
        }

  fGalChannelWidget->GeneratorActiveButton->setChecked(theSetup->IsGeneratorActive());
;


  // setup combobox entries from known sequences:
  fGalChannelWidget->Channel_sequence_comboBox_ALL->clear();

  int oldsid=fGalSequenceWidget->Sequence_comboBox->currentIndex(); // remember our active item
  fGalSequenceWidget->Sequence_comboBox->clear();
  for (int six=0; six<theSetup->NumKnownSequences(); ++six)
  {
    GalapagosSequence* seq=theSetup->GetKnownSequence(six);
    if(seq==0)  continue;
    for(uint8_t channel=0; channel<16;++channel)
    {
      if(six==0)fChannelSequenceCombo[channel]->clear();
      fChannelSequenceCombo[channel]->addItem(seq->Name());
    }



    fGalChannelWidget->Channel_sequence_comboBox_ALL->addItem(seq->Name());

    // also populate names in sequence editor window:
    fGalSequenceWidget->Sequence_comboBox->addItem(seq->Name());

  }
  fGalSequenceWidget->Sequence_comboBox->setCurrentIndex(oldsid); // restore active item


  // now refresh the combobox from configured sequences:
  for(uint8_t channel=0; channel<16;++channel)
      {
        GalapagosSequence* seq=theSetup->GetChannelSequence(channel);
        if(seq==0){
          printm ("Never come here - channel %d has no sequence in setup !",channel);
          continue;
        }
        int cix=fChannelSequenceCombo[channel]->findText(QString(seq->Name()));
        if(cix<0)
           printm ("Never come here - channel %d has no comboboxsequence entry %s",channel,seq->Name());
        else
          fChannelSequenceCombo[channel]->setCurrentIndex(cix);

      }


  //GAPG_UNLOCK_SLOT
  BasicGui::RefreshView ();
   // ^this handles the refresh of chains and status. better use base class function here! JAM2018
}





void GalapagosGui::EvaluateView ()
{
  std::cout << "GalapagosGui::EvaluateView"<<std::endl;

  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_CLASS(GalapagosSetup);


  theSetup->SetGeneratorActive(fGalChannelWidget->GeneratorActiveButton->isChecked());

  for (uint8_t channel = 0; channel < GAPG_CHANNELS; ++channel)
   {
     theSetup->SetChannelEnabled(channel, fChannelEnabledRadio[channel]->isChecked ());
     const char* seqname= fChannelSequenceCombo[channel]->currentText().toLatin1().data();
     bool rev=theSetup->SetChannelSequence(channel, seqname);
     if(!rev) printm ("Evaluate View Warning: sequence %s of current channel %d not known",
         seqname,channel);

   }




}



void GalapagosGui::SetRegisters ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);

  QApplication::setOverrideCursor (Qt::WaitCursor);

  for(uint8_t channel=0; channel<GAPG_CHANNELS;++channel)
  {
    WriteGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelSequenceID(channel));
  }
  /** channel enabled registers:*/
  WriteGAPG ( GAPG_CHANNEL_ENABLE_LOW, theSetup->GetChannelControl_0());
  WriteGAPG ( GAPG_CHANNEL_ENABLE_HI,  theSetup->GetChannelControl_1());

  /** possible master control register to start/stop processing*/
  uint32_t controlword=0;
  if(theSetup->IsGeneratorActive())
    controlword |= GAPG_BIT_MAIN_ENABLE;
  else
    controlword &= ~GAPG_BIT_MAIN_ENABLE;

  WriteGAPG ( GAPG_MAIN_CONTROL,  controlword);





  QApplication::restoreOverrideCursor ();

}

void GalapagosGui::GetRegisters ()
{
// read register values into structure with gosipcmd

 // return; //no readback from driver for the moment!

  theSetup_GET_FOR_CLASS(GalapagosSetup);
  QApplication::setOverrideCursor (Qt::WaitCursor);

  std::cout << "GalapagosGui::GetRegisters()"<<std::endl;

  uint32_t status=ReadGAPG ( GAPG_MAIN_CONTROL);

  theSetup->SetGeneratorActive((status & GAPG_BIT_MAIN_ENABLE) == GAPG_BIT_MAIN_ENABLE);


  /** channel enabled registers:*/
    uint32_t chanlo= ReadGAPG ( GAPG_CHANNEL_ENABLE_LOW);
    uint32_t chanhi= ReadGAPG ( GAPG_CHANNEL_ENABLE_HI);

    theSetup->SetChannelControl_0(chanlo);
    theSetup->SetChannelControl_1(chanhi);


    for(uint8_t channel=0; channel<GAPG_CHANNELS;++channel)
     {
       uint32_t seqid=ReadGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t));
       if(!theSetup->SetChannelSequence(channel,seqid))
         {
           printm ("GetRegisters Warning- channel %d has unknown sequence id %d on hardware, fallback to id 1",channel,seqid);
           theSetup->SetChannelSequence(channel,1);
         }
     }

  
  QApplication::restoreOverrideCursor ();
}


BasicSetup* GalapagosGui::CreateSetup()
     {
        GalapagosSetup* setup=new GalapagosSetup();

        // here we mock up some default patterns that might be always available

        GalapagosSequence seq0(1,"SinglePulse");
        seq0.AddCommand("SINGLE PULSE 100;");
        seq0.Compile();
        setup->AddSequence(seq0);
        GalapagosSequence seq1(2,"DoublePulse");
        seq1.AddCommand("DOUBLE PULSE 100 500;");
        seq1.Compile();
        setup->AddSequence(seq1);
        GalapagosSequence seq2(3,"PulseSequenceNew");
        seq2.AddCommand("SEQUENCE PULSE 100 20 20000;");
        seq2.AddCommand("KEEP 0 200;");
        seq2.AddCommand("SEQUENCE PULSE 100 20 30000;");
        seq2.AddCommand("KEEP 0 100;");
        seq2.Compile();
        setup->AddSequence(seq2);


      // std::cout <<"GalapagosGui:: CreateSetup" <<std::endl;
       return setup;
     }




    

