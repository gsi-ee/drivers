// implementation file for AwagsGui class with all slot functions

#include "AwagsGui.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>








void AwagsGui::AutoAdjustBtn_clicked ()
{
  //std::cout <<"AutoAdjustBtn_clicked "<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(AutoAdjust());
  QApplication::restoreOverrideCursor ();
}


void AwagsGui::CalibrateADCBtn_clicked ()
{
  //std::cout <<"CalibrateADCBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(CalibrateSelectedADCs());
  QApplication::restoreOverrideCursor ();
}


void AwagsGui::CalibrateResetBtn_clicked ()
{
  //std::cout <<"CalibrateResetBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(CalibrateResetSelectedADCs());
  QApplication::restoreOverrideCursor ();
}



//void AwagsGui::PeakFinderBtn_clicked()
//{
//  //std::cout <<"PeakFinderBtn_clicked"<< std::endl;
//  int channel = fAwagsWidget->PlotTabWidget->currentIndex ();
//  FindPeaks(channel);
//  RefreshSampleMaxima(channel);
//}

void AwagsGui::AcquireSamplesBtn_clicked ()
{
  //std::cout <<"AcquireSamplesBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(AcquireSelectedSamples());
  QApplication::restoreOverrideCursor ();
}

//void AwagsGui::MaximaCellDoubleClicked(int row, int column)
//{
//  //std::cout <<"MaximaCellDoubleClicked("<<row<<","<<column<<")"<< std::endl;
//  int channel = fAwagsWidget->PlotTabWidget->currentIndex ();
//  ZoomSampleToPeak(channel,row);
//}


void AwagsGui::DumpSamplesBtn_clicked ()
{
  //std::cout <<"DumpSamplesBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(ShowSelectedSamples());
  QApplication::restoreOverrideCursor ();
}


void AwagsGui::ZoomSampleBtn_clicked ()
{
  //std::cout <<"ZoomSampleBtn_clicked"<< std::endl;
  int channel = fAwagsWidget->PlotTabWidget->currentIndex ();
  ZoomSample (channel);
}

void AwagsGui::UnzoomSampleBtn_clicked ()
{
  //std::cout <<"UnzoomSampleBtn_clicked"<< std::endl;
  int channel = fAwagsWidget->PlotTabWidget->currentIndex ();
  UnzoomSample (channel);
}

void AwagsGui::RefreshSampleBtn_clicked ()
{
  //std::cout <<"RefreshSampleBtn_clicked"<< std::endl;
  int channel = fAwagsWidget->PlotTabWidget->currentIndex ();
  //std::cout <<"Got current index"<<channel<< std::endl;
  QApplication::setOverrideCursor (Qt::WaitCursor);
  AcquireSample (channel);
  ShowSample (channel);
  RefreshStatus ();
  QApplication::restoreOverrideCursor ();
}








void AwagsGui::SwitchChanged ()
{
  GOSIP_AUTOAPPLY(AutoApplySwitch());

}


//void AwagsGui::SetSimpleSwitches(bool on)
//{
//	//std::cout << "AwagsGui::SetSimpleSwitches( "<<on<<" )" << std::endl;
//	fUseSimpleSwitchAddressing=on;
//}

//void AwagsGui::PulserTimeout ()
//{
//  //std::cout << "AwagsGui::PulserTimeout" << std::endl;
//
//  if (fAwagsWidget->PulseBroadcastCheckBox->isChecked () && !fBroadcasting)
//  {
//    SetBroadcastPulser ();
//  }
//  else
//  {
//    for (uint8_t apf = 0; apf < AWAGS_NUMCHIPS; ++apf)
//    {
//      SetPulser (apf);
//    }
//
//  }
//  fPulserProgressCounter++;
//
//}
//
//void AwagsGui::PulserDisplayTimeout ()
//{
//  //std::cout << "AwagsGui::PulserdisplayTimeout" << std::endl;
//
//  double progress = (fPulserProgressCounter % 100);
//
//  fAwagsWidget->PulserProgressBar->setValue (progress);    // let the progress bar flicker from 0 to 100%
//  //std::cout << "Set Progress to"<<progress << std::endl;
//}
//
//
//
//void AwagsGui::PulserChanged (int awags)
//{
//  GOSIP_AUTOAPPLY(AutoApplyPulser(awags));
//}
//
//void AwagsGui::PulserChanged_0 ()
//{
//  PulserChanged (0);
//}

//void AwagsGui::PulserChanged_1 ()
//{
//  PulserChanged (1);
//}
//
//void AwagsGui::PulserChanged_2 ()
//{
//  PulserChanged (2);
//}
//
//void AwagsGui::PulserChanged_3 ()
//{
//  PulserChanged (3);
//}
//
//void AwagsGui::PulserChanged_4 ()
//{
//  PulserChanged (4);
//}
//
//void AwagsGui::PulserChanged_5 ()
//{
//  PulserChanged (5);
//}
//
//void AwagsGui::PulserChanged_6 ()
//{
//  PulserChanged (6);
//}
//
//void AwagsGui::PulserChanged_7 ()
//{
//  PulserChanged (7);
//}

void AwagsGui::GainChanged (int awags)
{
  GOSIP_AUTOAPPLY(AutoApplyGain(awags));
}

void AwagsGui::GainChanged_0 ()
{
  GainChanged (0);
}

void AwagsGui::GainChanged_1 ()
{
  GainChanged (1);
}

void AwagsGui::GainChanged_2 ()
{
  GainChanged (2);
}

void AwagsGui::GainChanged_3 ()
{
  GainChanged (3);
}




//void AwagsGui::GainChanged_4 ()
//{
//  GainChanged (2, 0);
//}
//
//void AwagsGui::GainChanged_5 ()
//{
//  GainChanged (2, 1);
//}
//
//void AwagsGui::GainChanged_6 ()
//{
//  GainChanged (3, 0);
//}
//
//void AwagsGui::GainChanged_7 ()
//{
//  GainChanged (3, 1);
//}
//
//void AwagsGui::GainChanged_8 ()
//{
//  GainChanged (4, 0);
//}
//
//void AwagsGui::GainChanged_9 ()
//{
//  GainChanged (4, 1);
//}
//
//void AwagsGui::GainChanged_10 ()
//{
//  GainChanged (5, 0);
//}
//
//void AwagsGui::GainChanged_11 ()
//{
//  GainChanged (5, 1);
//}
//
//void AwagsGui::GainChanged_12 ()
//{
//  GainChanged (6, 0);
//}
//
//void AwagsGui::GainChanged_13 ()
//{
//  GainChanged (6, 1);
//}
//
//void AwagsGui::GainChanged_14 ()
//{
//  GainChanged (7, 0);
//}
//
//void AwagsGui::GainChanged_15 ()
//{
//  GainChanged (7, 1);
//}
//
//



void AwagsGui::PowerChanged(int awags, int checkstate)
{
  //std::cout <<"PowerChanged slot, awags="<<awags<<", state="<<checkstate  <<std::endl;
  GOSIP_AUTOAPPLY(AutoApplyPower(awags,checkstate));
}



void AwagsGui::PowerChanged_0(int checkstate)
{
  PowerChanged(0,checkstate);
}
void AwagsGui::PowerChanged_1(int checkstate)
{
  PowerChanged(1,checkstate);
}
void AwagsGui::PowerChanged_2(int checkstate)
{
  PowerChanged(2,checkstate);
}
void AwagsGui::PowerChanged_3(int checkstate)
{
  PowerChanged(3,checkstate);
}
//void AwagsGui::PowerChanged_4(int checkstate)
//{
//  PowerChanged(4,checkstate);
//}
//void AwagsGui::PowerChanged_5(int checkstate)
//{
//  PowerChanged(5,checkstate);
//}
//void AwagsGui::PowerChanged_6(int checkstate)
//{
//  PowerChanged(6,checkstate);
//}
//void AwagsGui::PowerChanged_7(int checkstate)
//{
//  PowerChanged(7,checkstate);
//}
//












void AwagsGui::DAC_enterText (int awags, int dac)
{
  GOSIP_LOCK_SLOT
  // catch signal editingFinished() from Awags1_DAClineEdit_1 etc.
  // need to synchronize with the sliders anyway:
  int val = fDACLineEdit[awags][dac]->text ().toUInt (0, fNumberBase);
  fDACSlider[awags][dac]->setValue (val & 0x3FF);

  //std::cout << "AwagsGui::DAC_enterText=" << awags << ", dac=" << dac << ", val=" << val << std::endl;
  GOSIP_AUTOAPPLY(AutoApplyDAC(awags,dac, val));
  GOSIP_UNLOCK_SLOT
}

void AwagsGui::DAC_enterText_0_0 ()
{
  DAC_enterText (0, 0);
}

//void AwagsGui::DAC_enterText_0_1 ()
//{
//  DAC_enterText (0, 1);
//}
//
//void AwagsGui::DAC_enterText_0_2 ()
//{
//  DAC_enterText (0, 2);
//}
//
//void AwagsGui::DAC_enterText_0_3 ()
//{
//  DAC_enterText (0, 3);
//}

void AwagsGui::DAC_enterText_1_0 ()
{
  DAC_enterText (1, 0);
}

//void AwagsGui::DAC_enterText_1_1 ()
//{
//  DAC_enterText (1, 1);
//}
//
//void AwagsGui::DAC_enterText_1_2 ()
//{
//  DAC_enterText (1, 2);
//}
//
//void AwagsGui::DAC_enterText_1_3 ()
//{
//  DAC_enterText (1, 3);
//}

void AwagsGui::DAC_enterText_2_0 ()
{
  DAC_enterText (2, 0);
}
//void AwagsGui::DAC_enterText_2_1 ()
//{
//  DAC_enterText (2, 1);
//}
//
//void AwagsGui::DAC_enterText_2_2 ()
//{
//  DAC_enterText (2, 2);
//}
//
//void AwagsGui::DAC_enterText_2_3 ()
//{
//  DAC_enterText (2, 3);
//}

void AwagsGui::DAC_enterText_3_0 ()
{
  DAC_enterText (3, 0);
}

//void AwagsGui::DAC_enterText_3_1 ()
//{
//  DAC_enterText (3, 1);
//}
//
//void AwagsGui::DAC_enterText_3_2 ()
//{
//  DAC_enterText (3, 2);
//}
//
//void AwagsGui::DAC_enterText_3_3 ()
//{
//  DAC_enterText (3, 3);
//}
//
//void AwagsGui::DAC_enterText_4_0 ()
//{
//  DAC_enterText (4, 0);
//}
//
//void AwagsGui::DAC_enterText_4_1 ()
//{
//  DAC_enterText (4, 1);
//}
//void AwagsGui::DAC_enterText_4_2 ()
//{
//  DAC_enterText (4, 2);
//}
//
//void AwagsGui::DAC_enterText_4_3 ()
//{
//  DAC_enterText (4, 3);
//}
//
//void AwagsGui::DAC_enterText_5_0 ()
//{
//  DAC_enterText (5, 0);
//}
//
//void AwagsGui::DAC_enterText_5_1 ()
//{
//  DAC_enterText (5, 1);
//}
//
//void AwagsGui::DAC_enterText_5_2 ()
//{
//  DAC_enterText (5, 2);
//}
//
//void AwagsGui::DAC_enterText_5_3 ()
//{
//  DAC_enterText (5, 3);
//}
//
//void AwagsGui::DAC_enterText_6_0 ()
//{
//  DAC_enterText (6, 0);
//}
//
//void AwagsGui::DAC_enterText_6_1 ()
//{
//  DAC_enterText (6, 1);
//}
//
//void AwagsGui::DAC_enterText_6_2 ()
//{
//  DAC_enterText (6, 2);
//}
//void AwagsGui::DAC_enterText_6_3 ()
//{
//  DAC_enterText (6, 3);
//}
//
//void AwagsGui::DAC_enterText_7_0 ()
//{
//  DAC_enterText (7, 0);
//}
//
//void AwagsGui::DAC_enterText_7_1 ()
//{
//  DAC_enterText (7, 1);
//}
//void AwagsGui::DAC_enterText_7_2 ()
//{
//  DAC_enterText (7, 2);
//}
//void AwagsGui::DAC_enterText_7_3 ()
//{
//  DAC_enterText (7, 3);
//}

void AwagsGui::DAC_changed (int awags, int dac, int val)
{
  GOSIP_LOCK_SLOT
  //std::cout << "AwagsGui::DAC__changed, awags="<<awags<<", dac="<<dac<<", val="<<val << std::endl;
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  // need to synchronize the text view with the slider positions:
  fDACLineEdit[awags][dac]->setText (pre + text.setNum (val, fNumberBase));

  // if autoapply mode, immediately set to structure and
  GOSIP_AUTOAPPLY(AutoApplyDAC(awags,dac, val));
  GOSIP_UNLOCK_SLOT

}

void AwagsGui::DAC_changed_0_0 (int val)
{
  DAC_changed (0, 0, val);
}

//void AwagsGui::DAC_changed_0_1 (int val)
//{
//  DAC_changed (0, 1, val);
//}
//
//void AwagsGui::DAC_changed_0_2 (int val)
//{
//  DAC_changed (0, 2, val);
//}
//
//void AwagsGui::DAC_changed_0_3 (int val)
//{
//  DAC_changed (0, 3, val);
//}

void AwagsGui::DAC_changed_1_0 (int val)
{
  DAC_changed (1, 0, val);
}

//void AwagsGui::DAC_changed_1_1 (int val)
//{
//  DAC_changed (1, 1, val);
//}
//
//void AwagsGui::DAC_changed_1_2 (int val)
//{
//  DAC_changed (1, 2, val);
//}
//
//void AwagsGui::DAC_changed_1_3 (int val)
//{
//  DAC_changed (1, 3, val);
//}

void AwagsGui::DAC_changed_2_0 (int val)
{
  DAC_changed (2, 0, val);
}
//void AwagsGui::DAC_changed_2_1 (int val)
//{
//  DAC_changed (2, 1, val);
//}
//
//void AwagsGui::DAC_changed_2_2 (int val)
//{
//  DAC_changed (2, 2, val);
//}
//
//void AwagsGui::DAC_changed_2_3 (int val)
//{
//  DAC_changed (2, 3, val);
//}

void AwagsGui::DAC_changed_3_0 (int val)
{
  DAC_changed (3, 0, val);
}

//void AwagsGui::DAC_changed_3_1 (int val)
//{
//  DAC_changed (3, 1, val);
//}
//
//void AwagsGui::DAC_changed_3_2 (int val)
//{
//  DAC_changed (3, 2, val);
//}
//
//void AwagsGui::DAC_changed_3_3 (int val)
//{
//  DAC_changed (3, 3, val);
//}
//
//void AwagsGui::DAC_changed_4_0 (int val)
//{
//  DAC_changed (4, 0, val);
//}
//
//void AwagsGui::DAC_changed_4_1 (int val)
//{
//  DAC_changed (4, 1, val);
//}
//void AwagsGui::DAC_changed_4_2 (int val)
//{
//  DAC_changed (4, 2, val);
//}
//
//void AwagsGui::DAC_changed_4_3 (int val)
//{
//  DAC_changed (4, 3, val);
//}
//
//void AwagsGui::DAC_changed_5_0 (int val)
//{
//  DAC_changed (5, 0, val);
//}
//
//void AwagsGui::DAC_changed_5_1 (int val)
//{
//  DAC_changed (5, 1, val);
//}
//
//void AwagsGui::DAC_changed_5_2 (int val)
//{
//  DAC_changed (5, 2, val);
//}
//
//void AwagsGui::DAC_changed_5_3 (int val)
//{
//  DAC_changed (5, 3, val);
//}
//
//void AwagsGui::DAC_changed_6_0 (int val)
//{
//  DAC_changed (6, 0, val);
//}
//
//void AwagsGui::DAC_changed_6_1 (int val)
//{
//  DAC_changed (6, 1, val);
//}
//
//void AwagsGui::DAC_changed_6_2 (int val)
//{
//  DAC_changed (6, 2, val);
//}
//void AwagsGui::DAC_changed_6_3 (int val)
//{
//  DAC_changed (6, 3, val);
//}
//
//void AwagsGui::DAC_changed_7_0 (int val)
//{
//  DAC_changed (7, 0, val);
//}
//
//void AwagsGui::DAC_changed_7_1 (int val)
//{
//  DAC_changed (7, 1, val);
//}
//void AwagsGui::DAC_changed_7_2 (int val)
//{
//  DAC_changed (7, 2, val);
//}
//void AwagsGui::DAC_changed_7_3 (int val)
//{
//  DAC_changed (7, 3, val);
//}



void AwagsGui::AutoCalibrate (int awags)
{

  if (!checkBox_AA->isChecked ())
  {
    // first show confirm window if not running in auto apply mode:
    char buffer[1024];
    snprintf (buffer, 1024, "Really Do AWAGS DAC %d autocalibration for SFP %d Device %d?", awags, fSFP, fSlave);
    if (QMessageBox::question (this, "AWAGS GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }
  if (!fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(DoAutoCalibrate(awags));
  }
}

void AwagsGui::AutoCalibrate_0 ()
{
  AutoCalibrate (0);
}
void AwagsGui::AutoCalibrate_1 ()
{
  AutoCalibrate (1);
}
void AwagsGui::AutoCalibrate_2 ()
{
  AutoCalibrate (2);
}
void AwagsGui::AutoCalibrate_3 ()
{
  AutoCalibrate (3);
}
//void AwagsGui::AutoCalibrate_4 ()
//{
//  AutoCalibrate (4);
//}
//void AwagsGui::AutoCalibrate_5 ()
//{
//  AutoCalibrate (5);
//}
//void AwagsGui::AutoCalibrate_6 ()
//{
//  AutoCalibrate (6);
//}
//void AwagsGui::AutoCalibrate_7 ()
//{
//  AutoCalibrate (7);
//}

void AwagsGui::AutoCalibrate_all ()
{
  if (!checkBox_AA->isChecked ())
  {
    // first show confirm window if not running in auto apply mode:
    char buffer[1024];
    snprintf (buffer, 1024, "Really Do ALL AWAGS DAC autocalibration for SFP %d Device %d?", fSFP, fSlave);
    if (QMessageBox::question (this, "AWAGS GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }
  if (!fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(DoAutoCalibrateAll());
    for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
    {
      GOSIP_BROADCAST_ACTION(UpdateAfterAutoCalibrate(awags));
    }
  }
}

void AwagsGui::DAC_spinBox_all_changed (int val)
{
  //std::cout << "AwagsGui::DAC_spinBox_all_changed, val="<<val << std::endl;
  for (int chan = 0; chan < 16; ++chan)
    fDACSpinBoxes[chan]->setValue (val);

}

void AwagsGui::DAC_spinBox_changed (int channel, int val)
{
  GOSIP_LOCK_SLOT
  //std::cout << "AwagsGui::DAC_spinBox_changed, channel="<<channel<<",  val="<<val << std::endl;
  GOSIP_AUTOAPPLY(AutoApplyRefresh(channel, val));
 GOSIP_UNLOCK_SLOT
}

void AwagsGui::Any_spinBox00_changed (int val)
{
  DAC_spinBox_changed (0, val);
}

void AwagsGui::Any_spinBox01_changed (int val)
{
  DAC_spinBox_changed (1, val);
}

void AwagsGui::Any_spinBox02_changed (int val)
{
  DAC_spinBox_changed (2, val);
}

void AwagsGui::Any_spinBox03_changed (int val)
{
  DAC_spinBox_changed (3, val);
}

void AwagsGui::Any_spinBox04_changed (int val)
{
  DAC_spinBox_changed (4, val);
}

void AwagsGui::Any_spinBox05_changed (int val)
{
  DAC_spinBox_changed (5, val);
}

void AwagsGui::Any_spinBox06_changed (int val)
{
  DAC_spinBox_changed (6, val);
}

void AwagsGui::Any_spinBox07_changed (int val)
{
  DAC_spinBox_changed (7, val);
}

void AwagsGui::Any_spinBox08_changed (int val)
{
  DAC_spinBox_changed (8, val);
}

void AwagsGui::Any_spinBox09_changed (int val)
{
  DAC_spinBox_changed (9, val);
}

void AwagsGui::Any_spinBox10_changed (int val)
{
  DAC_spinBox_changed (10, val);
}

void AwagsGui::Any_spinBox11_changed (int val)
{
  DAC_spinBox_changed (11, val);
}

void AwagsGui::Any_spinBox12_changed (int val)
{
  DAC_spinBox_changed (12, val);
}

void AwagsGui::Any_spinBox13_changed (int val)
{
  DAC_spinBox_changed (13, val);
}

void AwagsGui::Any_spinBox14_changed (int val)
{
  DAC_spinBox_changed (14, val);
}

void AwagsGui::Any_spinBox15_changed (int val)
{
  DAC_spinBox_changed (15, val);
}



//void AwagsGui::InverseMapping_changed (int on)
//{
//  //std::cout << "InverseMapping_changed to" <<  on << std::endl;
//
//  if (checkBox_AA->isChecked () && !fBroadcasting)
//  {
//    EvaluateSlave ();
//    QApplication::setOverrideCursor (Qt::WaitCursor);
//    GOSIP_BROADCAST_ACTION(SetInverseMapping(on));
//    QApplication::restoreOverrideCursor ();
//  }
//
//}
//
//
//void AwagsGui::BaselineInvert_changed (int on)
//{
//  //std::cout << "BaselineInvert_changed to" <<  on << std::endl;
//  GOSIP_AUTOAPPLY(SetBaselineInverted(!on));
//}


//void AwagsGui::PulseBroadcast_changed (int on)
//{
//  //std::cout << "PulseBroadcast_changed to" <<  on << std::endl;
//  for (int ap = 0; ap < 8; ++ap)
//  {
//    fAwagsPulseGroup[ap]->setEnabled (!on);
//  }
//
//  if (on)
//  {
//    GOSIP_AUTOAPPLY(SetBroadcastPulser());
//  }
//
//}


//void AwagsGui::PulseTimer_changed (int on)
//{
//  //std::cout << "PulseTimer_changed to" <<  on << std::endl;
//
//  if (on)
//  {
//    int period = EvaluatePulserInterval (fAwagsWidget->FrequencyComboBox->currentIndex ());
//    printm ("Pulser Timer has been started with %d ms period.", period);
//    //std::cout << "PulseTimer starts with ms period" <<  period << std::endl;
//    fPulserTimer->setInterval (period);
//    fPulserTimer->start ();
//    fDisplayTimer->start ();
//  }
//  else
//  {
//    fPulserTimer->stop ();
//    fDisplayTimer->stop ();
//    fAwagsWidget->PulserProgressBar->reset ();
//    printm ("Pulser Timer has been stopped.");
//    //std::cout << "PulseTimer has been stopped. " << std::endl;
//  }
//
//  fPulserProgressCounter = 0;
//
//}
//
//void AwagsGui::PulseFrequencyChanged (int index)
//{
//  //std::cout << "PulseFrequencyChanged  to" <<  index << std::endl;
//  int period = EvaluatePulserInterval (index);
//  fPulserTimer->setInterval (period);
//  printm ("Pulser Period has been changed to %d ms.", period);
//}

void AwagsGui::BenchmarkTimerCallback ()
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
  fAwagsWidget->BenchmarkProgressBar->setValue (progress);

  double seconds = fSequencerStopwatch.elapsed () / 1000.0;
  fAwagsWidget->TimeNumber->display (seconds);

  if (!fBenchmark.ProcessBenchmark ())
    fSequencerTimer->stop ();

  GOSIP_BROADCAST_ACTION(RefreshView ());
  // instead of changing gui elements during measurement, we update view completely after each command. avoid unwanted effects by widget signals with auto apply!
  // the GOSIP_BROADCAST_ACTION is just use to supress further signal slot executions
}

void AwagsGui::StartBenchmarkPressed ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);




  if (!theSetup->IsUsePrototypeBoard())
  {
    // pandatest mode: remember carrier board id
    QString temperatur= fAwagsWidget->EnvironmentTemperature->text ();
    if (temperatur.isEmpty ())
    {
      printm ("Missing environment temperature information! Please type in a value (in �C)!");
      return;
    }
    theSetup->SetTemperature(temperatur);

    QString carrier= fAwagsWidget->CarrierSerialNum->text ();
    if (carrier.isEmpty ())
          {
            printm ("Missing carrier board id! Please specify full label information with QR code reader!");
            return;
          }

    theSetup->SetBoardID(carrier);

    // pandatest mode: indiviual chip ids, must all be delivered
    for (int a = 0; a < AWAGS_NUMCHIPS; ++a)
    {

      QString tag = fAwagsSerialLineEdit[a]->text ();
      if (tag.isEmpty ())
      {
        printm ("Missing chip id for position %d. Please specify full id information with QR code reader!", a);
        return;
      }
      theSetup->SetChipID (a, tag);
    }





  }
  else
  {
    // For prototype mode, take only first ids of 2er groups
    for (int a = 0; a < AWAGS_NUMCHIPS; ++a)
    {
      QString tag;
      if (a < 4)
        tag = fAwagsSerialLineEdit[0]->text ();
      else
        tag = fAwagsSerialLineEdit[2]->text ();
      if (tag.isEmpty ())
      {
        printm ("Missing chip id for position %d. Please specify full id information!", a);
        return;
      }
      theSetup->SetChipID (a, tag);
    }




  }



#ifdef   AWAGS_USE_TOELLNER_POWER
  double current=0, voltage=0;
  // here put automatic evaluation of power supply via serial connection:
  if(fAwagsWidget->AutoPowerscanCheckBox->isChecked())
  {
    printm ("Reading power supply settings via serial line...");

    ReadToellnerPower(voltage,current);
    fAwagsWidget->CurrentDoubleSpinBox->setValue(current*1000.0);
    fAwagsWidget->VoltageDoubleSpinBox->setValue(voltage);
  }
  else
  {
     current = fAwagsWidget->CurrentDoubleSpinBox->value () / 1000.0;
     voltage = fAwagsWidget->VoltageDoubleSpinBox->value ();
  }

  // JAM for test of toellner only, we just skip rest of benchmark:
  //return;

   theSetup->SetCurrentASIC (current);
  //theSetup->SetVoltage(voltage);

#endif



  printm ("Benchmark has been started for sfp %d slave %d", fSFP, fSlave);
//  printm ("Board 1:%s Board2:%s Current=%f A Voltage=%f V", awags1.toLatin1 ().constData (), awags2.toLatin1 ().constData (),
//      current, voltage);

  // here we evaluate a to do list that the timer should process:
  fBenchmark.SetSetup (theSetup);

  fBenchmark.ResetSequencerList ();

  // first execute put sequencer test for current and adressing here
//  if(fAwagsWidget->AddressTestCheckBox->isChecked())
//   {
//     for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
//           fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_ADDRESS_SCAN, awags));
//   }

   if(fAwagsWidget->CurrentMeasurementCheckBox->isChecked())
    {
      for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
            fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURRENT_MEASUEREMENT, awags));
    }



//////////////////// GAIN 1
  if (fAwagsWidget->Gain1groupBox->isChecked ())
  {
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_1));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));
    if (fAwagsWidget->Gain1TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
     if (fAwagsWidget->Gain1TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (fAwagsWidget->Gain1TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (fAwagsWidget->Gain1TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }

////////////////////////// GAIN 2
  if (fAwagsWidget->Gain16groupBox->isChecked ())
  {
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_2));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));
    if (fAwagsWidget->Gain16TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
     if (fAwagsWidget->Gain16TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (fAwagsWidget->Gain16TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (fAwagsWidget->Gain16TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }

  /////////////////////////////////// GAIN 4
  if (fAwagsWidget->Gain32groupBox->isChecked ())
  {
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_4));// ensure that we really have gain 32 set???
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));
    if (fAwagsWidget->Gain32TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
     if (fAwagsWidget->Gain32TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (fAwagsWidget->Gain32TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (fAwagsWidget->Gain32TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }

  /////////////////////////////////// GAIN 8
  if (fAwagsWidget->Gain64groupBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_8));// ensure that we really have gain 32 set???
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));
      if (fAwagsWidget->Gain64TestAutocalCheckBox->isChecked ())
      {
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
      }
       if (fAwagsWidget->Gain64TestBaselineCheckBox->isChecked ())
      {
        for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
          fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
      }
      if (fAwagsWidget->Gain64TestSigmaCheckBox->isChecked ())
      {
        for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
          fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
      }
      if (fAwagsWidget->Gain64TestCurveCheckBox->isChecked ())
      {
        for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
          fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
      }
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
    }

  fBenchmark.FinalizeSequencerList ();
  fSequencerStopwatch.start ();
  fSequencerTimer->start ();

}

void AwagsGui::CancelBenchmarkPressed ()
{
  printm ("Benchmark has been stopped.");
  fSequencerTimer->stop ();
}

void AwagsGui::ContinueBenchmarkPressed ()
{
  printm ("Benchmark will be continued.");
  fSequencerTimer->start ();
}

void AwagsGui::SaveBenchmarkPressed ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  // 2017 todo: this only works if only one slave is tested. for broadcast mode, ids should not be part of the filename!

  //
   QString filename ="AWAGS";
   if(theSetup->IsUsePrototypeBoard())
     filename ="Prototype";
   QString tag="";
   tag=theSetup->GetBoardID(); // carrier board
   filename = filename.append("_").append(tag);
   theSetup->GetChipID (0,tag); // first awags chip id
   filename = filename.append("_").append(tag);
   // count number of valid chips:
   int a=0;
   for(a=0; a< AWAGS_NUMCHIPS; ++a)
    {
      theSetup->GetChipID (a,tag); // we assume that chip id has been specified before benchmark
      if (tag.isEmpty ()) break;
    }
   filename.append(QString("_%1").arg(a)); // number of first connected chips
   filename.append (".apf");



  DoSaveConfig(filename.toLatin1 ().constData ());
}

void AwagsGui::BenchmarkPressed (QAbstractButton* but)
{
  if (but == fAwagsWidget->BenchmarkButtonBox->button (QDialogButtonBox::Apply))
  {
    StartBenchmarkPressed ();
  }
  else if (but == fAwagsWidget->BenchmarkButtonBox->button (QDialogButtonBox::Save))
  {

    SaveBenchmarkPressed ();
  }
  else if (but == fAwagsWidget->BenchmarkButtonBox->button (QDialogButtonBox::Cancel))
  {
    //printm("Benchmark will be canceled.");
    CancelBenchmarkPressed ();
  }
  else if (but == fAwagsWidget->BenchmarkButtonBox->button (QDialogButtonBox::Retry))
  {
    ContinueBenchmarkPressed ();
  }

}


void AwagsGui::ChangeReferenceDataPressed(QAbstractButton* but)
{

  //std::cout << "ChangeReferenceDataPressed" << std::endl;

  if (but == fAwagsWidget->ReferenceLoadButtonBox->button (QDialogButtonBox::Open))
    {
      LoadBenchmarkReferences();
    }
  else if (but == fAwagsWidget->ReferenceLoadButtonBox->button (QDialogButtonBox::RestoreDefaults))
   {
      fAwagsWidget->ReferenceLineEdit->setText("buildin defaults");
      fBenchmark.InitReferenceValues(false);
      //fBenchmark.InitReferenceValues(!fAwagsWidget->Baseline_Box_invert->isChecked());
   }


}


void AwagsGui::PlotTabChanged (int num)
{
  //std::cout << "PlotTabChanged to "<<num << std::endl;
  //RefreshSampleMaxima(num);

}
//

void AwagsGui::MeasureCurrentsPushButton_clicked ()
{
  //std::cout << "MeasureCurrentsPushButton_clicked" << std::endl;

//  printm ("Have read momentary current:%f", ReadKeithleyCurrent());
//
//  return; // DEBUG
  // later we do the procedure
  if(!fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(DoCurrentScan());
  }

}

void AwagsGui::InitKeithleyPushButton_clicked ()
{
  //std::cout << "InitKeithleyPushButton_clicked" << std::endl;
  InitKeithley();
}


//void AwagsGui::AddressScanPushButton_clicked ()
//{
//  //std::cout << "AddressScanPushButton_clicked" << std::endl;
//  if(!fBroadcasting)
//  {
//    EvaluateSlave ();
//    GOSIP_BROADCAST_ACTION(DoIdScan());
//  }
//}


