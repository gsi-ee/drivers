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
            //std::cout<< " GalapagosGui::ReasdSettings() will load sequence file"<<seqfilename.toLatin1().data()<< std::endl;
            if(!LoadSequence(seqfilename)) printm("Warning: Sequence %s from setup could not be loaded!",seqfilename.toLatin1().data());
          }
    int oldix=0; // later take from settings
    fGalSequenceWidget->Sequence_comboBox->setCurrentIndex(oldix); // toggle refresh the editor?
    SequenceIDChanged(oldix);
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
        //std::cout<< " GalapagosGui::WriteSettings() saves sequence file"<<seqfilename.toLatin1().data()<< std::endl;
        SaveSequence(seqfilename, seq);
      }
    fSettings->setValue("Numsequences",(int) theSetup->NumKnownSequences());

    }
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




void GalapagosGui::SequenceIDChanged (int ix)
{
  GAPG_LOCK_SLOT;
  //std::cout << "GalapagosGui::SequenceIDChanged  ix="<<ix << std::endl;

  fGalSequenceWidget->SequenceTextEdit->clear();

  // now take out commands from known sequences:
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  GalapagosSequence* seq=theSetup->GetKnownSequence(ix);
  if(seq==0)  {
      fGalSequenceWidget->SequenceTextEdit->appendPlainText("unknown ID!");
      return;
  }
  //std::cout<<"SequenceIDChanged gets sequence :"<<std::hex<< (ulong) seq<< ", id:"<<std::dec << seq->Id()<<", name:"<<seq->Name()<< std::endl;
  const char* line=0;
  int l=0;
  while ((line=seq->GetCommandLine(l++)) !=0)
    {
      //std::cout<<"SequenceIDChanged reading  line:"<<line  << std::endl;
      QString txt(line);
      fGalSequenceWidget->SequenceTextEdit->appendPlainText(txt);
    }
  GAPG_UNLOCK_SLOT;
}

void GalapagosGui::SequenceNew_clicked()
{
  //std::cout << "GalapagosGui:: SequenceNew_clicked "<< std::endl;
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
  //std::cout << "GalapagosGui:: SequenceEdit_clicked"<< std::endl;
  fGalSequenceWidget->SequenceTextEdit->setReadOnly(false);
  fGalSequenceWidget->SequenceApplyButton->setEnabled(true);
  fGalSequenceWidget->SequenceEditCancelButton->setEnabled(true);
  fGalSequenceWidget->Sequence_comboBox->setEnabled(false);
}

void GalapagosGui::SequenceEditCancel_clicked()
{
  //std::cout << "GalapagosGui:: SequenceEditCancel_clicked"<< std::endl;
  int ix=fGalSequenceWidget->Sequence_comboBox->currentIndex();
  SequenceIDChanged(ix);
  fGalSequenceWidget->SequenceTextEdit->setReadOnly(true);
  fGalSequenceWidget->SequenceApplyButton->setEnabled(false);
  fGalSequenceWidget->SequenceEditCancelButton->setEnabled(false);
  fGalSequenceWidget->Sequence_comboBox->setEnabled(true);
}


void GalapagosGui::SequenceLoad_clicked()
{
  //std::cout << "GalapagosGui::SequenceLoad_clicked"<< std::endl;
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
        ++fit;
     }
 GAPG_LOCK_SLOT;
     RefreshView(); // populate comboboxes with all known sequences
 GAPG_UNLOCK_SLOT;

}

void GalapagosGui::SequenceSave_clicked()
{
  //std::cout << "GalapagosGui::SequenceSave_clicked"<< std::endl;
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
}

void GalapagosGui::SequenceApply_clicked()
{
  //std::cout << "GalapagosGui::SequenceOK_clicked"<< std::endl;
  fGalSequenceWidget->SequenceTextEdit->setReadOnly(true);
  fGalSequenceWidget->SequenceApplyButton->setEnabled(false);


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
   fGalSequenceWidget->Sequence_comboBox->setEnabled(true);
}



    

