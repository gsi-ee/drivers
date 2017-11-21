// implementation file for ApfelGui class with all slot functions

#include "ApfelGui.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>








void ApfelGui::AutoAdjustBtn_clicked ()
{
  //std::cout <<"AutoAdjustBtn_clicked "<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(AutoAdjust());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::CalibrateADCBtn_clicked ()
{
  //std::cout <<"CalibrateADCBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(CalibrateSelectedADCs());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::CalibrateResetBtn_clicked ()
{
  //std::cout <<"CalibrateResetBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(CalibrateResetSelectedADCs());
  QApplication::restoreOverrideCursor ();
}



void ApfelGui::PeakFinderBtn_clicked()
{
  //std::cout <<"PeakFinderBtn_clicked"<< std::endl;
  int channel = fApfelWidget->PlotTabWidget->currentIndex ();
  FindPeaks(channel);
  RefreshSampleMaxima(channel);
}

void ApfelGui::AcquireSamplesBtn_clicked ()
{
  //std::cout <<"AcquireSamplesBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(AcquireSelectedSamples());
  QApplication::restoreOverrideCursor ();
}

void ApfelGui::MaximaCellDoubleClicked(int row, int column)
{
  //std::cout <<"MaximaCellDoubleClicked("<<row<<","<<column<<")"<< std::endl;
  int channel = fApfelWidget->PlotTabWidget->currentIndex ();
  ZoomSampleToPeak(channel,row);
}


void ApfelGui::DumpSamplesBtn_clicked ()
{
  //std::cout <<"DumpSamplesBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(ShowSelectedSamples());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::ZoomSampleBtn_clicked ()
{
  //std::cout <<"ZoomSampleBtn_clicked"<< std::endl;
  int channel = fApfelWidget->PlotTabWidget->currentIndex ();
  ZoomSample (channel);
}

void ApfelGui::UnzoomSampleBtn_clicked ()
{
  //std::cout <<"UnzoomSampleBtn_clicked"<< std::endl;
  int channel = fApfelWidget->PlotTabWidget->currentIndex ();
  UnzoomSample (channel);
}

void ApfelGui::RefreshSampleBtn_clicked ()
{
  //std::cout <<"RefreshSampleBtn_clicked"<< std::endl;
  int channel = fApfelWidget->PlotTabWidget->currentIndex ();
  //std::cout <<"Got current index"<<channel<< std::endl;
  QApplication::setOverrideCursor (Qt::WaitCursor);
  AcquireSample (channel);
  ShowSample (channel);
  RefreshStatus ();
  QApplication::restoreOverrideCursor ();
}








void ApfelGui::SwitchChanged ()
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplySwitch());
  }
}

void ApfelGui::PulserTimeout ()
{
  //std::cout << "ApfelGui::PulserTimeout" << std::endl;

  if (fApfelWidget->PulseBroadcastCheckBox->isChecked () && !fBroadcasting)
  {
    SetBroadcastPulser ();
  }
  else
  {
    for (uint8_t apf = 0; apf < APFEL_NUMCHIPS; ++apf)
    {
      SetPulser (apf);
    }

  }
  fPulserProgressCounter++;

}

void ApfelGui::PulserDisplayTimeout ()
{
  //std::cout << "ApfelGui::PulserdisplayTimeout" << std::endl;

  double progress = (fPulserProgressCounter % 100);

  fApfelWidget->PulserProgressBar->setValue (progress);    // let the progress bar flicker from 0 to 100%
  //std::cout << "Set Progress to"<<progress << std::endl;
}



void ApfelGui::PulserChanged (int apfel)
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplyPulser(apfel));
  }

}

void ApfelGui::PulserChanged_0 ()
{
  PulserChanged (0);
}

void ApfelGui::PulserChanged_1 ()
{
  PulserChanged (1);
}

void ApfelGui::PulserChanged_2 ()
{
  PulserChanged (2);
}

void ApfelGui::PulserChanged_3 ()
{
  PulserChanged (3);
}

void ApfelGui::PulserChanged_4 ()
{
  PulserChanged (4);
}

void ApfelGui::PulserChanged_5 ()
{
  PulserChanged (5);
}

void ApfelGui::PulserChanged_6 ()
{
  PulserChanged (6);
}

void ApfelGui::PulserChanged_7 ()
{
  PulserChanged (7);
}

void ApfelGui::GainChanged (int apfel, int channel)
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplyGain(apfel,channel));
  }
}

void ApfelGui::GainChanged_0 ()
{
  GainChanged (0, 0);
}

void ApfelGui::GainChanged_1 ()
{
  GainChanged (0, 1);
}

void ApfelGui::GainChanged_2 ()
{
  GainChanged (1, 0);
}

void ApfelGui::GainChanged_3 ()
{
  GainChanged (1, 1);
}

void ApfelGui::GainChanged_4 ()
{
  GainChanged (2, 0);
}

void ApfelGui::GainChanged_5 ()
{
  GainChanged (2, 1);
}

void ApfelGui::GainChanged_6 ()
{
  GainChanged (3, 0);
}

void ApfelGui::GainChanged_7 ()
{
  GainChanged (3, 1);
}

void ApfelGui::GainChanged_8 ()
{
  GainChanged (4, 0);
}

void ApfelGui::GainChanged_9 ()
{
  GainChanged (4, 1);
}

void ApfelGui::GainChanged_10 ()
{
  GainChanged (5, 0);
}

void ApfelGui::GainChanged_11 ()
{
  GainChanged (5, 1);
}

void ApfelGui::GainChanged_12 ()
{
  GainChanged (6, 0);
}

void ApfelGui::GainChanged_13 ()
{
  GainChanged (6, 1);
}

void ApfelGui::GainChanged_14 ()
{
  GainChanged (7, 0);
}

void ApfelGui::GainChanged_15 ()
{
  GainChanged (7, 1);
}





void ApfelGui::PowerChanged(int apfel, int checkstate)
{
  //std::cout <<"PowerChanged slot, apfel="<<apfel<<", state="<<checkstate  <<std::endl;

  if (checkBox_AA->isChecked () && !fBroadcasting)
   {
     EvaluateSlave ();
     GOSIP_BROADCAST_ACTION(AutoApplyPower(apfel,checkstate));
   }
}



void ApfelGui::PowerChanged_0(int checkstate)
{
  PowerChanged(0,checkstate);
}
void ApfelGui::PowerChanged_1(int checkstate)
{
  PowerChanged(1,checkstate);
}
void ApfelGui::PowerChanged_2(int checkstate)
{
  PowerChanged(2,checkstate);
}
void ApfelGui::PowerChanged_3(int checkstate)
{
  PowerChanged(3,checkstate);
}
void ApfelGui::PowerChanged_4(int checkstate)
{
  PowerChanged(4,checkstate);
}
void ApfelGui::PowerChanged_5(int checkstate)
{
  PowerChanged(5,checkstate);
}
void ApfelGui::PowerChanged_6(int checkstate)
{
  PowerChanged(6,checkstate);
}
void ApfelGui::PowerChanged_7(int checkstate)
{
  PowerChanged(7,checkstate);
}













void ApfelGui::DAC_enterText (int apfel, int dac)
{
  GOSIP_LOCK_SLOT
  // catch signal editingFinished() from Apfel1_DAClineEdit_1 etc.
  // need to synchronize with the sliders anyway:
  int val = fDACLineEdit[apfel][dac]->text ().toUInt (0, fNumberBase);
  fDACSlider[apfel][dac]->setValue (val & 0x3FF);

  //std::cout << "ApfelGui::DAC_enterText=" << apfel << ", dac=" << dac << ", val=" << val << std::endl;
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplyDAC(apfel,dac, val));
  }
  GOSIP_UNLOCK_SLOT
}

void ApfelGui::DAC_enterText_0_0 ()
{
  DAC_enterText (0, 0);
}

void ApfelGui::DAC_enterText_0_1 ()
{
  DAC_enterText (0, 1);
}

void ApfelGui::DAC_enterText_0_2 ()
{
  DAC_enterText (0, 2);
}

void ApfelGui::DAC_enterText_0_3 ()
{
  DAC_enterText (0, 3);
}

void ApfelGui::DAC_enterText_1_0 ()
{
  DAC_enterText (1, 0);
}

void ApfelGui::DAC_enterText_1_1 ()
{
  DAC_enterText (1, 1);
}

void ApfelGui::DAC_enterText_1_2 ()
{
  DAC_enterText (1, 2);
}

void ApfelGui::DAC_enterText_1_3 ()
{
  DAC_enterText (1, 3);
}

void ApfelGui::DAC_enterText_2_0 ()
{
  DAC_enterText (2, 0);
}
void ApfelGui::DAC_enterText_2_1 ()
{
  DAC_enterText (2, 1);
}

void ApfelGui::DAC_enterText_2_2 ()
{
  DAC_enterText (2, 2);
}

void ApfelGui::DAC_enterText_2_3 ()
{
  DAC_enterText (2, 3);
}

void ApfelGui::DAC_enterText_3_0 ()
{
  DAC_enterText (3, 0);
}

void ApfelGui::DAC_enterText_3_1 ()
{
  DAC_enterText (3, 1);
}

void ApfelGui::DAC_enterText_3_2 ()
{
  DAC_enterText (3, 2);
}

void ApfelGui::DAC_enterText_3_3 ()
{
  DAC_enterText (3, 3);
}

void ApfelGui::DAC_enterText_4_0 ()
{
  DAC_enterText (4, 0);
}

void ApfelGui::DAC_enterText_4_1 ()
{
  DAC_enterText (4, 1);
}
void ApfelGui::DAC_enterText_4_2 ()
{
  DAC_enterText (4, 2);
}

void ApfelGui::DAC_enterText_4_3 ()
{
  DAC_enterText (4, 3);
}

void ApfelGui::DAC_enterText_5_0 ()
{
  DAC_enterText (5, 0);
}

void ApfelGui::DAC_enterText_5_1 ()
{
  DAC_enterText (5, 1);
}

void ApfelGui::DAC_enterText_5_2 ()
{
  DAC_enterText (5, 2);
}

void ApfelGui::DAC_enterText_5_3 ()
{
  DAC_enterText (5, 3);
}

void ApfelGui::DAC_enterText_6_0 ()
{
  DAC_enterText (6, 0);
}

void ApfelGui::DAC_enterText_6_1 ()
{
  DAC_enterText (6, 1);
}

void ApfelGui::DAC_enterText_6_2 ()
{
  DAC_enterText (6, 2);
}
void ApfelGui::DAC_enterText_6_3 ()
{
  DAC_enterText (6, 3);
}

void ApfelGui::DAC_enterText_7_0 ()
{
  DAC_enterText (7, 0);
}

void ApfelGui::DAC_enterText_7_1 ()
{
  DAC_enterText (7, 1);
}
void ApfelGui::DAC_enterText_7_2 ()
{
  DAC_enterText (7, 2);
}
void ApfelGui::DAC_enterText_7_3 ()
{
  DAC_enterText (7, 3);
}

void ApfelGui::DAC_changed (int apfel, int dac, int val)
{
  GOSIP_LOCK_SLOT
  //std::cout << "ApfelGui::DAC__changed, apfel="<<apfel<<", dac="<<dac<<", val="<<val << std::endl;
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  // need to synchronize the text view with the slider positions:
  fDACLineEdit[apfel][dac]->setText (pre + text.setNum (val, fNumberBase));

  // if autoapply mode, immediately set to structure and

  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplyDAC(apfel,dac, val));
  }
  GOSIP_UNLOCK_SLOT

}

void ApfelGui::DAC_changed_0_0 (int val)
{
  DAC_changed (0, 0, val);
}

void ApfelGui::DAC_changed_0_1 (int val)
{
  DAC_changed (0, 1, val);
}

void ApfelGui::DAC_changed_0_2 (int val)
{
  DAC_changed (0, 2, val);
}

void ApfelGui::DAC_changed_0_3 (int val)
{
  DAC_changed (0, 3, val);
}

void ApfelGui::DAC_changed_1_0 (int val)
{
  DAC_changed (1, 0, val);
}

void ApfelGui::DAC_changed_1_1 (int val)
{
  DAC_changed (1, 1, val);
}

void ApfelGui::DAC_changed_1_2 (int val)
{
  DAC_changed (1, 2, val);
}

void ApfelGui::DAC_changed_1_3 (int val)
{
  DAC_changed (1, 3, val);
}

void ApfelGui::DAC_changed_2_0 (int val)
{
  DAC_changed (2, 0, val);
}
void ApfelGui::DAC_changed_2_1 (int val)
{
  DAC_changed (2, 1, val);
}

void ApfelGui::DAC_changed_2_2 (int val)
{
  DAC_changed (2, 2, val);
}

void ApfelGui::DAC_changed_2_3 (int val)
{
  DAC_changed (2, 3, val);
}

void ApfelGui::DAC_changed_3_0 (int val)
{
  DAC_changed (3, 0, val);
}

void ApfelGui::DAC_changed_3_1 (int val)
{
  DAC_changed (3, 1, val);
}

void ApfelGui::DAC_changed_3_2 (int val)
{
  DAC_changed (3, 2, val);
}

void ApfelGui::DAC_changed_3_3 (int val)
{
  DAC_changed (3, 3, val);
}

void ApfelGui::DAC_changed_4_0 (int val)
{
  DAC_changed (4, 0, val);
}

void ApfelGui::DAC_changed_4_1 (int val)
{
  DAC_changed (4, 1, val);
}
void ApfelGui::DAC_changed_4_2 (int val)
{
  DAC_changed (4, 2, val);
}

void ApfelGui::DAC_changed_4_3 (int val)
{
  DAC_changed (4, 3, val);
}

void ApfelGui::DAC_changed_5_0 (int val)
{
  DAC_changed (5, 0, val);
}

void ApfelGui::DAC_changed_5_1 (int val)
{
  DAC_changed (5, 1, val);
}

void ApfelGui::DAC_changed_5_2 (int val)
{
  DAC_changed (5, 2, val);
}

void ApfelGui::DAC_changed_5_3 (int val)
{
  DAC_changed (5, 3, val);
}

void ApfelGui::DAC_changed_6_0 (int val)
{
  DAC_changed (6, 0, val);
}

void ApfelGui::DAC_changed_6_1 (int val)
{
  DAC_changed (6, 1, val);
}

void ApfelGui::DAC_changed_6_2 (int val)
{
  DAC_changed (6, 2, val);
}
void ApfelGui::DAC_changed_6_3 (int val)
{
  DAC_changed (6, 3, val);
}

void ApfelGui::DAC_changed_7_0 (int val)
{
  DAC_changed (7, 0, val);
}

void ApfelGui::DAC_changed_7_1 (int val)
{
  DAC_changed (7, 1, val);
}
void ApfelGui::DAC_changed_7_2 (int val)
{
  DAC_changed (7, 2, val);
}
void ApfelGui::DAC_changed_7_3 (int val)
{
  DAC_changed (7, 3, val);
}



void ApfelGui::AutoCalibrate (int apfel)
{

  if (!checkBox_AA->isChecked ())
  {
    // first show confirm window if not running in auto apply mode:
    char buffer[1024];
    snprintf (buffer, 1024, "Really Do APFEL DAC %d autocalibration for SFP %d Device %d?", apfel, fSFP, fSlave);
    if (QMessageBox::question (this, "APFEL GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }
  if (!fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(DoAutoCalibrate(apfel));
  }
}

void ApfelGui::AutoCalibrate_0 ()
{
  AutoCalibrate (0);
}
void ApfelGui::AutoCalibrate_1 ()
{
  AutoCalibrate (1);
}
void ApfelGui::AutoCalibrate_2 ()
{
  AutoCalibrate (2);
}
void ApfelGui::AutoCalibrate_3 ()
{
  AutoCalibrate (3);
}
void ApfelGui::AutoCalibrate_4 ()
{
  AutoCalibrate (4);
}
void ApfelGui::AutoCalibrate_5 ()
{
  AutoCalibrate (5);
}
void ApfelGui::AutoCalibrate_6 ()
{
  AutoCalibrate (6);
}
void ApfelGui::AutoCalibrate_7 ()
{
  AutoCalibrate (7);
}

void ApfelGui::AutoCalibrate_all ()
{
  if (!checkBox_AA->isChecked ())
  {
    // first show confirm window if not running in auto apply mode:
    char buffer[1024];
    snprintf (buffer, 1024, "Really Do ALL APFEL DAC autocalibration for SFP %d Device %d?", fSFP, fSlave);
    if (QMessageBox::question (this, "APFEL GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }
  if (!fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(DoAutoCalibrateAll());
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      GOSIP_BROADCAST_ACTION(UpdateAfterAutoCalibrate(apfel));
    }
  }
}

void ApfelGui::DAC_spinBox_all_changed (int val)
{
  //std::cout << "ApfelGui::DAC_spinBox_all_changed, val="<<val << std::endl;
  for (int chan = 0; chan < 16; ++chan)
    fDACSpinBoxes[chan]->setValue (val);

}

void ApfelGui::DAC_spinBox_changed (int channel, int val)
{
  GOSIP_LOCK_SLOT
  //std::cout << "ApfelGui::DAC_spinBox_changed, channel="<<channel<<",  val="<<val << std::endl;
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplyRefresh(channel, val));
  }
 GOSIP_UNLOCK_SLOT
}

void ApfelGui::Any_spinBox00_changed (int val)
{
  DAC_spinBox_changed (0, val);
}

void ApfelGui::Any_spinBox01_changed (int val)
{
  DAC_spinBox_changed (1, val);
}

void ApfelGui::Any_spinBox02_changed (int val)
{
  DAC_spinBox_changed (2, val);
}

void ApfelGui::Any_spinBox03_changed (int val)
{
  DAC_spinBox_changed (3, val);
}

void ApfelGui::Any_spinBox04_changed (int val)
{
  DAC_spinBox_changed (4, val);
}

void ApfelGui::Any_spinBox05_changed (int val)
{
  DAC_spinBox_changed (5, val);
}

void ApfelGui::Any_spinBox06_changed (int val)
{
  DAC_spinBox_changed (6, val);
}

void ApfelGui::Any_spinBox07_changed (int val)
{
  DAC_spinBox_changed (7, val);
}

void ApfelGui::Any_spinBox08_changed (int val)
{
  DAC_spinBox_changed (8, val);
}

void ApfelGui::Any_spinBox09_changed (int val)
{
  DAC_spinBox_changed (9, val);
}

void ApfelGui::Any_spinBox10_changed (int val)
{
  DAC_spinBox_changed (10, val);
}

void ApfelGui::Any_spinBox11_changed (int val)
{
  DAC_spinBox_changed (11, val);
}

void ApfelGui::Any_spinBox12_changed (int val)
{
  DAC_spinBox_changed (12, val);
}

void ApfelGui::Any_spinBox13_changed (int val)
{
  DAC_spinBox_changed (13, val);
}

void ApfelGui::Any_spinBox14_changed (int val)
{
  DAC_spinBox_changed (14, val);
}

void ApfelGui::Any_spinBox15_changed (int val)
{
  DAC_spinBox_changed (15, val);
}



void ApfelGui::InverseMapping_changed (int on)
{
  //std::cout << "InverseMapping_changed to" <<  on << std::endl;

  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    QApplication::setOverrideCursor (Qt::WaitCursor);
    GOSIP_BROADCAST_ACTION(SetInverseMapping(on));
    QApplication::restoreOverrideCursor ();
  }

}


void ApfelGui::BaselineInvert_changed (int on)
{
  //std::cout << "BaselineInvert_changed to" <<  on << std::endl;

   if (checkBox_AA->isChecked () && !fBroadcasting)
   {
     EvaluateSlave ();
     GOSIP_BROADCAST_ACTION(SetBaselineInverted(!on));
     //NOTE: "inverted" on gui means the old behaviour (pasem), in fact internally this is the non inverted state
   }


}


void ApfelGui::PulseBroadcast_changed (int on)
{
  //std::cout << "PulseBroadcast_changed to" <<  on << std::endl;
  for (int ap = 0; ap < 8; ++ap)
  {
    fApfelPulseGroup[ap]->setEnabled (!on);
  }

  if (on)
  {

    if (checkBox_AA->isChecked () && !fBroadcasting)
    {
      EvaluateSlave ();
      GOSIP_BROADCAST_ACTION(SetBroadcastPulser());
    }

  }

}


void ApfelGui::PulseTimer_changed (int on)
{
  //std::cout << "PulseTimer_changed to" <<  on << std::endl;

  if (on)
  {
    int period = EvaluatePulserInterval (fApfelWidget->FrequencyComboBox->currentIndex ());
    printm ("Pulser Timer has been started with %d ms period.", period);
    //std::cout << "PulseTimer starts with ms period" <<  period << std::endl;
    fPulserTimer->setInterval (period);
    fPulserTimer->start ();
    fDisplayTimer->start ();
  }
  else
  {
    fPulserTimer->stop ();
    fDisplayTimer->stop ();
    fApfelWidget->PulserProgressBar->reset ();
    printm ("Pulser Timer has been stopped.");
    //std::cout << "PulseTimer has been stopped. " << std::endl;
  }

  fPulserProgressCounter = 0;

}

void ApfelGui::PulseFrequencyChanged (int index)
{
  //std::cout << "PulseFrequencyChanged  to" <<  index << std::endl;
  int period = EvaluatePulserInterval (index);
  fPulserTimer->setInterval (period);
  printm ("Pulser Period has been changed to %d ms.", period);
}

void ApfelGui::BenchmarkTimerCallback ()
{
  // this one does the actual benchmarking procedure:
  static int sequencergain = 0;
  // each time we come here, the next thing on the todo list is handled:

  // TODO: later we need all this as BROADCAST_ACTION function? or prevent broadcasting mode for characterization
  if (!AssertNoBroadcast ())
  {
    printm ("Benchmark is not allowed in slave broadcast mode! stopping test.");
    fSequencerTimer->stop ();
    return;
  }

  // redundant, setup is assigned before timer starts:
  //BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
  //fBenchmark.SetSetup(&theSetup);
//
  int progress = fBenchmark.GetSequencerProgress ();
  fApfelWidget->BenchmarkProgressBar->setValue (progress);

  double seconds = fSequencerStopwatch.elapsed () / 1000.0;
  fApfelWidget->TimeNumber->display (seconds);

  if (!fBenchmark.ProcessBenchmark ())
    fSequencerTimer->stop ();

  GOSIP_BROADCAST_ACTION(RefreshView ());
  // instead of changing gui elements during measurement, we update view completely after each command. avoid unwanted effects by widget signals with auto apply!
  // the GOSIP_BROADCAST_ACTION is just use to supress further signal slot executions
}

void ApfelGui::StartBenchmarkPressed ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  printm ("Benchmark Timer has been started!");



  if(theSetup->IsUsePandaTestBoard())
  {
    // pandatest mode: indiviual chip ids, must all be delivered
  for(int a=0; a< APFEL_NUMCHIPS; ++a)
  {

    QString tag=fApfelSerialLineEdit[a]->text ();
    if (tag.isEmpty ())
     {
       printm ("Missing chip id for position %d. Please specify full id information!",a);
       return;
     }
    theSetup->SetChipID (a,tag);
  }
  }
  else
  {
    // For pasem mode, take only first ids of 4er groups
    for(int a=0; a< APFEL_NUMCHIPS; ++a)
    {
      QString tag;
      if(a<4)  tag=fApfelSerialLineEdit[0]->text ();
      else tag= fApfelSerialLineEdit[4]->text ();
      if (tag.isEmpty ())
        {
            printm ("Missing chip id for position %d. Please specify full id information!",a);
            return;
        }
      theSetup->SetChipID (a,tag);
    }



  }



#ifdef   APFEL_USE_TOELLNER_POWER
  double current=0, voltage=0;
  // here put automatic evaluation of power supply via serial connection:
  if(fApfelWidget->AutoPowerscanCheckBox->isChecked())
  {
    printm ("Reading power supply settings via serial line...");

    ReadToellnerPower(voltage,current);
    fApfelWidget->CurrentDoubleSpinBox->setValue(current*1000.0);
    fApfelWidget->VoltageDoubleSpinBox->setValue(voltage);
  }
  else
  {
     current = fApfelWidget->CurrentDoubleSpinBox->value () / 1000.0;
     voltage = fApfelWidget->VoltageDoubleSpinBox->value ();
  }

  // JAM for test of toellner only, we just skip rest of benchmark:
  //return;

   theSetup->SetCurrentASIC (current);
  //theSetup->SetVoltage(voltage);

#endif





  //2017 TODO: take into account different current test benchmarks





  printm ("Benchmark has been started for sfp %d slave %d", fSFP, fSlave);
//  printm ("Board 1:%s Board2:%s Current=%f A Voltage=%f V", apfel1.toLatin1 ().constData (), apfel2.toLatin1 ().constData (),
//      current, voltage);

  // here we evaluate a to do list that the timer should process:
  fBenchmark.SetSetup (theSetup);






  fBenchmark.ResetSequencerList ();

  // TODO: put sequencer test for current and adressing here
  // for the moment, we rely that this has been done before





  if (fApfelWidget->Gain1groupBox->isChecked ())
  {

    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_1));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));

    if (fApfelWidget->Gain1TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
    if (fApfelWidget->Gain1TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));

    }
    if (fApfelWidget->Gain1TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));

    }
    if (fApfelWidget->Gain1TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }

  if (fApfelWidget->Gain16groupBox->isChecked ())
  {

    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_16));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));


    if (fApfelWidget->Gain16TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
    if (fApfelWidget->Gain16TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (fApfelWidget->Gain16TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (fApfelWidget->Gain16TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }
  if (fApfelWidget->Gain32groupBox->isChecked ())
  {

    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_32));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));

    if (fApfelWidget->Gain32TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
    if (fApfelWidget->Gain32TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (fApfelWidget->Gain32TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (fApfelWidget->Gain32TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));

  }

  fBenchmark.FinalizeSequencerList ();
  fSequencerStopwatch.start ();
  fSequencerTimer->start ();

}

void ApfelGui::CancelBenchmarkPressed ()
{
  printm ("Benchmark has been stopped.");
  fSequencerTimer->stop ();
}

void ApfelGui::ContinueBenchmarkPressed ()
{
  printm ("Benchmark will be continued.");
  fSequencerTimer->start ();
}

void ApfelGui::SaveBenchmarkPressed ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  // 2017 todo: this only works if only one slave is tested. for broadcast mode, ids should not be part of the filename!

  //
   QString filename ="APFELSEM";
   if(theSetup->IsUsePandaTestBoard())
     filename ="PANDAtest";
   QString tag="";
   theSetup->GetChipID (0,tag);
   filename = filename.append("_").append(tag);
   // count number of valid chips:
   int a=0;
   for(a=0; a< APFEL_NUMCHIPS; ++a)
    {
      theSetup->GetChipID (a,tag); // we assume that chip id has been specified before benchmark
      if (tag.isEmpty ()) break;
    }
   filename.append("_%1").arg(a);
   filename.append (".apf");



  DoSaveConfig(filename.toLatin1 ().constData ());
}

void ApfelGui::BenchmarkPressed (QAbstractButton* but)
{
  if (but == fApfelWidget->BenchmarkButtonBox->button (QDialogButtonBox::Apply))
  {
    StartBenchmarkPressed ();
  }
  else if (but == fApfelWidget->BenchmarkButtonBox->button (QDialogButtonBox::Save))
  {

    SaveBenchmarkPressed ();
  }
  else if (but == fApfelWidget->BenchmarkButtonBox->button (QDialogButtonBox::Cancel))
  {
    //printm("Benchmark will be canceled.");
    CancelBenchmarkPressed ();
  }
  else if (but == fApfelWidget->BenchmarkButtonBox->button (QDialogButtonBox::Retry))
  {
    ContinueBenchmarkPressed ();
  }

}


void ApfelGui::ChangeReferenceDataPressed(QAbstractButton* but)
{

  //std::cout << "ChangeReferenceDataPressed" << std::endl;

  if (but == fApfelWidget->ReferenceLoadButtonBox->button (QDialogButtonBox::Open))
    {
      LoadBenchmarkReferences();
    }
  else if (but == fApfelWidget->ReferenceLoadButtonBox->button (QDialogButtonBox::RestoreDefaults))
   {
      fApfelWidget->ReferenceLineEdit->setText("buildin defaults");
      fBenchmark.InitReferenceValues(!fApfelWidget->Baseline_Box_invert->isChecked());
   }


}


void ApfelGui::PlotTabChanged (int num)
{
  //std::cout << "PlotTabChanged to "<<num << std::endl;
  RefreshSampleMaxima(num);

}


void ApfelGui::MeasureCurrentsPushButton_clicked ()
{
  std::cout << "MeasureCurrentsPushButton_clicked" << std::endl;



}

void ApfelGui::InitKeithleyPushButton_clicked ()
{
  std::cout << "InitKeithleyPushButton_clicked" << std::endl;
  InitKeithley();
}


void ApfelGui::AddressScanPushButton_clicked ()
{
  //std::cout << "AddressScanPushButton_clicked" << std::endl;
  if(!fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(DoIdScan());
  }
}


