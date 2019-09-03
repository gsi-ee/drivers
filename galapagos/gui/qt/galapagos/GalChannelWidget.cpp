#include "GalChannelWidget.h"
#include "GalapagosSetup.h"


#include "BasicGui.h"

namespace gapg{

GalChannelWidget::GalChannelWidget (QWidget* parent): gapg::BasicSubWidget(parent)
{

  setupUi (this);

  fChannelEnabledRadio[0] = Channel_enabled_radio_00;
    fChannelEnabledRadio[1] = Channel_enabled_radio_01;
    fChannelEnabledRadio[2] = Channel_enabled_radio_02;
    fChannelEnabledRadio[3] = Channel_enabled_radio_03;
    fChannelEnabledRadio[4] = Channel_enabled_radio_04;
    fChannelEnabledRadio[5] = Channel_enabled_radio_05;
    fChannelEnabledRadio[6] = Channel_enabled_radio_06;
    fChannelEnabledRadio[7] = Channel_enabled_radio_07;
    fChannelEnabledRadio[8] = Channel_enabled_radio_08;
    fChannelEnabledRadio[9] = Channel_enabled_radio_09;
    fChannelEnabledRadio[10] = Channel_enabled_radio_10;
    fChannelEnabledRadio[11] = Channel_enabled_radio_11;
    fChannelEnabledRadio[12] = Channel_enabled_radio_12;
    fChannelEnabledRadio[13] = Channel_enabled_radio_13;
    fChannelEnabledRadio[14] = Channel_enabled_radio_14;
    fChannelEnabledRadio[15] = Channel_enabled_radio_15;




    fChannelActiveLED[0] = Channel_active_LED_00;
     fChannelActiveLED[1] = Channel_active_LED_01;
     fChannelActiveLED[2] = Channel_active_LED_02;
     fChannelActiveLED[3] = Channel_active_LED_03;
     fChannelActiveLED[4] = Channel_active_LED_04;
     fChannelActiveLED[5] = Channel_active_LED_05;
     fChannelActiveLED[6] = Channel_active_LED_06;
     fChannelActiveLED[7] = Channel_active_LED_07;
     fChannelActiveLED[8] = Channel_active_LED_08;
     fChannelActiveLED[9] = Channel_active_LED_09;
     fChannelActiveLED[10] = Channel_active_LED_10;
     fChannelActiveLED[11] = Channel_active_LED_11;
     fChannelActiveLED[12] = Channel_active_LED_12;
     fChannelActiveLED[13] = Channel_active_LED_13;
     fChannelActiveLED[14] = Channel_active_LED_14;
     fChannelActiveLED[15] = Channel_active_LED_15;



     fChannelSimulatedRadio[0] = Channel_simulate_radio_00;
     fChannelSimulatedRadio[1] = Channel_simulate_radio_01;
     fChannelSimulatedRadio[2] = Channel_simulate_radio_02;
     fChannelSimulatedRadio[3] = Channel_simulate_radio_03;
     fChannelSimulatedRadio[4] = Channel_simulate_radio_04;
     fChannelSimulatedRadio[5] = Channel_simulate_radio_05;
     fChannelSimulatedRadio[6] = Channel_simulate_radio_06;
     fChannelSimulatedRadio[7] = Channel_simulate_radio_07;
     fChannelSimulatedRadio[8] = Channel_simulate_radio_08;
     fChannelSimulatedRadio[9] = Channel_simulate_radio_09;
     fChannelSimulatedRadio[10] = Channel_simulate_radio_10;
     fChannelSimulatedRadio[11] = Channel_simulate_radio_11;
     fChannelSimulatedRadio[12] = Channel_simulate_radio_12;
     fChannelSimulatedRadio[13] = Channel_simulate_radio_13;
     fChannelSimulatedRadio[14] = Channel_simulate_radio_14;
     fChannelSimulatedRadio[15] = Channel_simulate_radio_15;


     fChannelSequenceCombo[0] = Channel_sequence_comboBox_00;
     fChannelSequenceCombo[1] = Channel_sequence_comboBox_01;
     fChannelSequenceCombo[2] = Channel_sequence_comboBox_02;
     fChannelSequenceCombo[3] = Channel_sequence_comboBox_03;
     fChannelSequenceCombo[4] = Channel_sequence_comboBox_04;
     fChannelSequenceCombo[5] = Channel_sequence_comboBox_05;
     fChannelSequenceCombo[6] = Channel_sequence_comboBox_06;
     fChannelSequenceCombo[7] = Channel_sequence_comboBox_07;
     fChannelSequenceCombo[8] = Channel_sequence_comboBox_08;
     fChannelSequenceCombo[9] = Channel_sequence_comboBox_09;
     fChannelSequenceCombo[10] = Channel_sequence_comboBox_10;
     fChannelSequenceCombo[11] = Channel_sequence_comboBox_11;
     fChannelSequenceCombo[12] = Channel_sequence_comboBox_12;
     fChannelSequenceCombo[13] = Channel_sequence_comboBox_13;
     fChannelSequenceCombo[14] = Channel_sequence_comboBox_14;
     fChannelSequenceCombo[15] = Channel_sequence_comboBox_15;

     fChannelPatternCombo[0] = Channel_pattern_comboBox_00;
     fChannelPatternCombo[1] = Channel_pattern_comboBox_01;
     fChannelPatternCombo[2] = Channel_pattern_comboBox_02;
     fChannelPatternCombo[3] = Channel_pattern_comboBox_03;
     fChannelPatternCombo[4] = Channel_pattern_comboBox_04;
     fChannelPatternCombo[5] = Channel_pattern_comboBox_05;
     fChannelPatternCombo[6] = Channel_pattern_comboBox_06;
     fChannelPatternCombo[7] = Channel_pattern_comboBox_07;
     fChannelPatternCombo[8] = Channel_pattern_comboBox_08;
     fChannelPatternCombo[9] = Channel_pattern_comboBox_09;
     fChannelPatternCombo[10] = Channel_pattern_comboBox_10;
     fChannelPatternCombo[11] = Channel_pattern_comboBox_11;
     fChannelPatternCombo[12] = Channel_pattern_comboBox_12;
     fChannelPatternCombo[13] = Channel_pattern_comboBox_13;
     fChannelPatternCombo[14] = Channel_pattern_comboBox_14;
     fChannelPatternCombo[15] = Channel_pattern_comboBox_15;


}




GalChannelWidget::~GalChannelWidget ()
{
}



void GalChannelWidget::ConnectSlots()
{
  QObject::connect (GeneratorActiveButton, SIGNAL(clicked(bool)), this, SLOT(GeneratorActive_clicked(bool)));


   QObject::connect (Channel_enabled_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_all(bool)));
   GALAGUI_CONNECT_TOGGLED_16(Channel_enabled_radio_, ChannelEnabled_toggled_);


   QObject::connect (Channel_simulate_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelSimulated_toggled_all(bool)));
   GALAGUI_CONNECT_TOGGLED_16(Channel_simulate_radio_, ChannelSimulated_toggled_);


   QObject::connect (Channel_sequence_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this,  SLOT(ChannelSequence_changed_all(int)));
   GALAGUI_CONNECT_INDEXCHANGED_16(Channel_sequence_comboBox_,ChannelSequence_changed_);

   QObject::connect (Channel_pattern_comboBox_ALL, SIGNAL(currentIndexChanged(int)), this,  SLOT(ChannelPattern_changed_all(int)));
   GALAGUI_CONNECT_INDEXCHANGED_16(Channel_pattern_comboBox_,ChannelPattern_changed_);



   QObject::connect (ChannelgroupBox_0, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_group0(bool)));

   QObject::connect (ChannelgroupBox_1, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_group1(bool)));
}



void GalChannelWidget::GeneratorActive_clicked(bool checked)
{
  //std::cout<< "GeneratorActive_clicked with checked="<< checked << std::endl;
  GAPG_AUTOAPPLY(ApplyGeneratorActive(checked));

}



void GalChannelWidget::ChannelEnabled_toggled_all(bool on)
{
  for(int chan=0;chan<16;++chan)
    fChannelEnabledRadio[chan]->setChecked (on);
}



void GalChannelWidget::ChannelEnabled_toggled (int channel, bool on)
{
  GAPG_AUTOAPPLY(ApplyChannelEnabled(channel, on));
}

void  GalChannelWidget::ChannelEnabled_toggled_group0(bool on)
{
  for(int chan=0;chan<8;++chan)
      fChannelEnabledRadio[chan]->setChecked (on);
}

void  GalChannelWidget::ChannelEnabled_toggled_group1(bool on)
{
  for(int chan=8;chan<16;++chan)
        fChannelEnabledRadio[chan]->setChecked (on);
}


GALAGUI_IMPLEMENT_MULTICHANNEL_TOGGLED_16(GalChannelWidget, ChannelEnabled);



void GalChannelWidget::ChannelSimulated_toggled_all(bool on)
{
  for(int chan=0;chan<16;++chan)
    fChannelSimulatedRadio[chan]->setChecked (on);
}



void GalChannelWidget::ChannelSimulated_toggled (int channel, bool on)
{
  GAPG_AUTOAPPLY(ApplyChannelSimulated(channel, on));

}


GALAGUI_IMPLEMENT_MULTICHANNEL_TOGGLED_16(GalChannelWidget, ChannelSimulated);





void GalChannelWidget::ChannelSequence_changed_all(int ix)
{
  for(int chan=0;chan<16;++chan)
    fChannelSequenceCombo[chan]->setCurrentIndex (ix);
}

void GalChannelWidget::ChannelSequence_changed (int channel, int ix)
{
  GAPG_LOCK_SLOT
  //std::cout << "GalChannelWidget::ChannelSequence_changed ch="<<channel<<",  ix="<<ix << std::endl;
  GAPG_AUTOAPPLY(ApplyChannelSequence(channel, ix));
  GAPG_UNLOCK_SLOT
}

GALAGUI_IMPLEMENT_MULTICHANNEL_CHANGED_16(GalChannelWidget, ChannelSequence);




void GalChannelWidget::ChannelPattern_changed_all(int ix)
{
  for(int chan=0;chan<16;++chan)
    fChannelPatternCombo[chan]->setCurrentIndex (ix);
}


void GalChannelWidget::ChannelPattern_changed (int channel, int ix)
{
  GAPG_LOCK_SLOT
  //std::cout << "GalChannelWidget::ChannelSequence_changed ch="<<channel<<",  ix="<<ix << std::endl;
  GAPG_AUTOAPPLY(ApplyChannelPattern(channel, ix));
  GAPG_UNLOCK_SLOT
}

GALAGUI_IMPLEMENT_MULTICHANNEL_CHANGED_16(GalChannelWidget, ChannelPattern);



void GalChannelWidget::EvaluateView ()
{
  //std::cout << "GalChannelWidget::EvaluateView"<<std::endl;

  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_CLASS(GalapagosSetup);


  theSetup->SetGeneratorActive(GeneratorActiveButton->isChecked());

  for (uint8_t channel = 0; channel < GAPG_CHANNELS; ++channel)
   {
     theSetup->SetChannelEnabled(channel, fChannelEnabledRadio[channel]->isChecked ());
     const char* seqname= fChannelSequenceCombo[channel]->currentText().toLatin1().data();
     bool rev=theSetup->SetChannelSequence(channel, seqname);
     if(!rev) printm ("Evaluate View Warning: sequence %s of current channel %d not known",
         seqname,channel);

     const char* patname= fChannelPatternCombo[channel]->currentText().toLatin1().data();
     rev=theSetup->SetChannelPattern(channel, patname);
     if(!rev) printm ("Evaluate View Warning: pattern %s of current channel %d not known",
         patname,channel);
   }

}







void GalChannelWidget::RefreshView ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  bool isrunning = theSetup->IsGeneratorActive();

  Channel_active_LED_ALL->setColor(isrunning ? QColor(Qt::green) : QColor(Qt::red));

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

 GeneratorActiveButton->setChecked(theSetup->IsGeneratorActive());
;

// setup combobox entries from known sequences:
 Channel_sequence_comboBox_ALL->clear();
 for (int six=0; six<theSetup->NumKnownSequences(); ++six)
 {
   GalapagosSequence* seq=theSetup->GetKnownSequence(six);
   if(seq==0)  continue;
   for(uint8_t channel=0; channel<16;++channel)
   {
     if(six==0)fChannelSequenceCombo[channel]->clear();
     fChannelSequenceCombo[channel]->addItem(seq->Name());
   }
   Channel_sequence_comboBox_ALL->addItem(seq->Name());
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

 ////////////// dito for the pattern combo boxes:

  // setup combobox entries from known sequences:
  Channel_pattern_comboBox_ALL->clear ();
  for (int pix = 0; pix < theSetup->NumKnownPatterns (); ++pix)
  {
    GalapagosPattern* pat = theSetup->GetKnownPattern (pix);
    if (pat == 0)
      continue;
    for (uint8_t channel = 0; channel < 16; ++channel)
    {
      if (pix == 0)
        fChannelPatternCombo[channel]->clear ();
      fChannelPatternCombo[channel]->addItem (pat->Name ());
    }
    Channel_pattern_comboBox_ALL->addItem (pat->Name ());
  }

  for (uint8_t channel = 0; channel < 16; ++channel)
  {
    GalapagosPattern* pat = theSetup->GetChannelPattern (channel);
    if (pat == 0)
    {
      printm ("Never come here - channel %d has no pattern in setup !", channel);
      continue;
    }
    int cix = fChannelPatternCombo[channel]->findText (QString (pat->Name ()));
    if (cix < 0)
      printm ("Never come here - channel %d has no combobox pattern entry %s", channel, pat->Name ());
    else
      fChannelPatternCombo[channel]->setCurrentIndex (cix);
  }


}



void GalChannelWidget::ApplyGeneratorActive(bool on)
{
//   std::cout << "GalChannelWidget::ApplyGeneratorActive on="<<on << std::endl;
   theSetup_GET_FOR_CLASS(GalapagosSetup);
   theSetup->SetGeneratorActive(on);
   /* TODO: this should be common function, or will be handled by setting whole controlregister from structure*/
   uint32_t controlword=0;
    if(theSetup->IsGeneratorActive())
      controlword |= GAPG_BIT_MAIN_ENABLE;
    else
      controlword &= ~GAPG_BIT_MAIN_ENABLE;

    fParent->WriteGAPG ( GAPG_MAIN_CONTROL,  controlword);
    GAPG_LOCK_SLOT
    fParent->GetRegisters();
    RefreshView();
    GAPG_UNLOCK_SLOT

}


void GalChannelWidget::ApplyChannelEnabled(int channel, bool on)
{
 //std::cout << "GalChannelWidget::ApplyEnabled chan="<<channel<<",  on="<<on << std::endl;
 theSetup_GET_FOR_CLASS(GalapagosSetup);
 theSetup->SetChannelEnabled(channel, on);
 fParent->WriteGAPG ( GAPG_CHANNEL_ENABLE_LOW, theSetup->GetChannelControl_0());
 fParent->WriteGAPG ( GAPG_CHANNEL_ENABLE_HI,  theSetup->GetChannelControl_1());
 GAPG_LOCK_SLOT
 RefreshView();
 GAPG_UNLOCK_SLOT

}

void GalChannelWidget::ApplyChannelSimulated(int channel, bool on)
{
 //std::cout << "GalChannelWidget::ApplySimuated chan="<<channel<<",  on="<<on << std::endl;
 //theSetup_GET_FOR_CLASS(GalapagosSetup);
//   theSetup->SetChannelSimulated(channel, on);
// RefreshView();

}

void GalChannelWidget::ApplyChannelSequence(int channel, int ix)
{
// std::cout << "GalChannelWidget::ApplyChannelSequence chan="<<channel<<",  ix="<<ix << std::endl;
 theSetup_GET_FOR_CLASS(GalapagosSetup);
 const char* seqname= fChannelSequenceCombo[channel]->itemText(ix).toLatin1().data();
 bool rev=theSetup->SetChannelSequence(channel, seqname);
 if(!rev) printm ("ApplyChannelSequence Warning: sequence %s of current channel %d not known",
             seqname,channel);
 theSetup->SetChannelSequence(channel, seqname);
 fParent->WriteGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelSequenceID(channel));

// RefreshView();

}


void GalChannelWidget::ApplyChannelPattern(int channel, int ix)
{
std::cout << "GalChannelWidget::ApplyChannelPattern chan="<<channel<<",  ix="<<ix << std::endl;
 theSetup_GET_FOR_CLASS(GalapagosSetup);
 const char* patname= fChannelPatternCombo[channel]->itemText(ix).toLatin1().data();
 bool rev=theSetup->SetChannelPattern(channel, patname);
 if(!rev) printm ("ApplyChannelPattern Warning: pattern %s of current channel %d not known",
             patname,channel);
 theSetup->SetChannelPattern(channel, patname);
 fParent->WriteGAPG ( GAPG_CHANNEL_PATTERN_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelPatternID(channel));

// RefreshView();

}

} // namespace

