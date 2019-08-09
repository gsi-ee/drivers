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
 fVersionString="Welcome to GalapaGUI!\n\t v0.11 of 8-Aug-2019 by JAM (j.adamczewski@gsi.de)";

 fSettings=new QSettings("GSI", fImplementationName);

 Qt::WindowFlags wflags= Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint;
 fGalChannelWidget= new GalChannelWidget(this);
 QMdiSubWindow* channels=mdiArea->addSubWindow(fGalChannelWidget,wflags);
 channels->setAttribute(Qt::WA_DeleteOnClose, false);



  setWindowTitle(QString("%1").arg(fImplementationName));


  QObject::connect (fGalChannelWidget->GeneratorActiveButton, SIGNAL(clicked(bool)), this, SLOT(GeneratorActive_clicked(bool)));


  QObject::connect (fGalChannelWidget->Channel_enabled_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_all(bool)));
  GALAGUI_CONNECT_TOGGLED_16(fGalChannelWidget->Channel_enabled_radio_, ChannelEnabled_toggled_);


  QObject::connect (fGalChannelWidget->Channel_simulate_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelSimulated_toggled_all(bool)));
  GALAGUI_CONNECT_TOGGLED_16(fGalChannelWidget->Channel_simulate_radio_, ChannelSimulated_toggled_);


  QObject::connect (fGalChannelWidget->Channel_sequence_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this,  SLOT(ChannelSequence_changed_all(int)));
  GALAGUI_CONNECT_INDEXCHANGED_16(fGalChannelWidget->Channel_sequence_comboBox_,ChannelSequence_changed_);

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

  ReadSettings();
  BuildSetup();
  show ();
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



void GalapagosGui::RefreshView ()
{

  //std::cout << "GalapagosGui::RefreshView"<<std::endl;
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
  }

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
  //std::cout << "GalapagosGui::EvaluateView"<<std::endl;

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

  //std::cout << "GalapagosGui::GetRegisters()"<<std::endl;

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
       theSetup->SetChannelSequence(channel,seqid);

     }



  
  QApplication::restoreOverrideCursor ();
}


BasicSetup* GalapagosGui::CreateSetup()
     {
        GalapagosSetup* setup=new GalapagosSetup();

        // here we mock up some default patterns that might be always available

        GalapagosSequence seq0(1,"SinglePulse");
        seq0.AddCommand("SINGLE PULSE 100");
        seq0.Compile();
        setup->AddSequence(seq0);
        GalapagosSequence seq1(2,"DoublePulse");
        seq1.AddCommand("DOUBLE PULSE 100 500");
        seq1.Compile();
        setup->AddSequence(seq1);
        GalapagosSequence seq2(3,"PulseSequenceNew");
        seq2.AddCommand("SEQUENCE PULSE 100 20 20000");
        seq2.Compile();
        setup->AddSequence(seq2);


      // std::cout <<"GalapagosGui:: CreateSetup" <<std::endl;
       return setup;
     }




    

