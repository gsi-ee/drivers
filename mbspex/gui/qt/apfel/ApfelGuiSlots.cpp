// implementation file for ApfelGui class with all slot functions

#include "ApfelGui.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>

void ApfelGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "ApfelGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
  if (checked)
  {
    fSFPSave = SFPspinBox->value ();
    fSlaveSave = SlavespinBox->value ();
    SFPspinBox->setValue (-1);
    SlavespinBox->setValue (-1);
  }
  else
  {
    RefreshChains ();
    SFPspinBox->setValue (fSFPSave);
    SlavespinBox->setValue (fSlaveSave);

  }
}




void ApfelGui::ShowBtn_clicked ()
{
  //std::cout << "ApfelGui::ShowBtn_clicked()"<< std::endl;
  EvaluateSlave ();
  GetSFPChainSetup ();

  if (!AssertNoBroadcast (false))
    return;
  if (!AssertChainConfigured ())
    return;
  GetRegisters ();
  RefreshView ();
}

void ApfelGui::ApplyBtn_clicked ()
{
//std::cout << "ApfelGui::ApplyBtn_clicked()"<< std::endl;

  EvaluateSlave ();

// JAM maybe disable confirm window ?
//  char buffer[1024];
//  snprintf (buffer, 1024, "Really apply APFEL settings  to SFP %d Device %d?", fSFP, fSlave);
//  if (QMessageBox::question (this, "APFEL GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
//      != QMessageBox::Yes)
//  {
//    return;
//  }

  GetSFPChainSetup ();
  if (AssertNoBroadcast (false) && !AssertChainConfigured ())
    return;

  // JAM: since we keep all slave set ups in vector/array, we must handle broadcast mode explicitely
  // no implicit driver broadcast via -1 indices anymore!
  APFEL_BROADCAST_ACTION(ApplyGUISettings());

}


void ApfelGui::SaveConfigBtn_clicked (const char* selectfile)
{
//std::cout << "ApfelGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  QString apf_filter ("apfel characterization file (*.apf)");
  QStringList filters;
  filters << gos_filter << apf_filter;

  QFileDialog fd (this, "Write Apfel configuration file");

  // ".", "nyxor setup file (*.txt);;gosipcmd file (*.gos);;context dump file (*.dmp)");
  fd.setNameFilters (filters);
  fd.selectNameFilter (apf_filter);
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);

  if (selectfile)
  {
    fd.selectFile (QString (selectfile));
  }
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString fileName = flst[0];

  // complete suffix if user did not
  if (fd.selectedNameFilter () == gos_filter)
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");

    // open file
    if (OpenConfigFile (fileName) != 0)
      return;

    WriteConfigFile (QString ("#Format *.gos"));
    WriteConfigFile (QString ("#usage: gosipcmd -x -c file.gos \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("#sfp slave address value\n"));
    APFEL_BROADCAST_ACTION(SaveRegisters());
    // refresh actual setup from hardware and write it to open file
    CloseConfigFile ();
    snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
    AppendTextWindow (buffer);
  }
  else if (fd.selectedNameFilter () == apf_filter)
  {
    if (!fileName.endsWith (".apf"))
      fileName.append (".apf");

    OpenTestFile (fileName);
    APFEL_BROADCAST_ACTION(SaveTestResults());
    // dump figures of merit of current slave, or of all slaves
    CloseTestFile ();
    snprintf (buffer, 1024, "Saved test result to file '%s' .\n", fileName.toLatin1 ().constData ());
    AppendTextWindow (buffer);
  }

  else
  {
    std::cout << "ApfelGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

}


void ApfelGui::InitChainBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
//std::cout << "InitChainBtn_clicked()"<< std::endl;
  bool ok;
  snprintf (buffer, 1024, "Please specify NUMBER OF DEVICES to initialize at SFP %d ?", fSFP);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  int numslaves = QInputDialog::getInt (this, tr ("Number of Slaves?"), tr (buffer), 1, 1, 1024, 1, &ok);
#else
  int numslaves = QInputDialog::getInteger(this, tr("Number of Slaves?"),
      tr(buffer), 1, 1, 1024, 1, &ok);

#endif
  if (!ok)
    return;
  if (fSFP < 0)
  {
    AppendTextWindow ("--- Error: Broadcast not allowed for init chain!");
    return;
  }
#ifdef USE_MBSPEX_LIB
  int rev = mbspex_slave_init (fPexFD, fSFP, numslaves);

#else
  snprintf (buffer, 1024, "gosipcmd -i  %d %d", fSFP, numslaves);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
#endif

  GetSFPChainSetup ();
  RefreshChains ();
}

void ApfelGui::ResetBoardBtn_clicked ()
{
//std::cout << "ApfelGui::ResetBoardBtn_clicked"<< std::endl;
  if (QMessageBox::question (this, "APFEL GUI", "Really Reset gosip on pex board?", QMessageBox::Yes | QMessageBox::No,
      QMessageBox::Yes) != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
#ifdef USE_MBSPEX_LIB
  mbspex_reset (fPexFD);
  AppendTextWindow ("Reset PEX board with mbspex_reset()");

#else

  char buffer[1024];
  snprintf (buffer, 1024, "gosipcmd -z");
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);

#endif

  GetSFPChainSetup ();
  RefreshChains ();
}

void ApfelGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really initialize APFEL device at SFP %d, Slave %d ?", fSFP, fSlave);
  if (QMessageBox::question (this, "Apfel GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
  APFEL_BROADCAST_ACTION(InitApfel());

}

void ApfelGui::AutoAdjustBtn_clicked ()
{
  //std::cout <<"AutoAdjustBtn_clicked "<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  APFEL_BROADCAST_ACTION(AutoAdjust());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::CalibrateADCBtn_clicked ()
{
  //std::cout <<"CalibrateADCBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  APFEL_BROADCAST_ACTION(CalibrateSelectedADCs());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::CalibrateResetBtn_clicked ()
{
  //std::cout <<"CalibrateResetBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  APFEL_BROADCAST_ACTION(CalibrateResetSelectedADCs());
  QApplication::restoreOverrideCursor ();
}



void ApfelGui::PeakFinderBtn_clicked()
{
  //std::cout <<"PeakFinderBtn_clicked"<< std::endl;
  int channel = PlotTabWidget->currentIndex ();
  FindPeaks(channel);
  RefreshSampleMaxima(channel);
}

void ApfelGui::AcquireSamplesBtn_clicked ()
{
  //std::cout <<"AcquireSamplesBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  APFEL_BROADCAST_ACTION(AcquireSelectedSamples());
  QApplication::restoreOverrideCursor ();
}

void ApfelGui::MaximaCellDoubleClicked(int row, int column)
{
  //std::cout <<"MaximaCellDoubleClicked("<<row<<","<<column<<")"<< std::endl;
  int channel = PlotTabWidget->currentIndex ();
  ZoomSampleToPeak(channel,row);
}


void ApfelGui::DumpSamplesBtn_clicked ()
{
  //std::cout <<"DumpSamplesBtn_clicked"<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  APFEL_BROADCAST_ACTION(ShowSelectedSamples());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::ZoomSampleBtn_clicked ()
{
  //std::cout <<"ZoomSampleBtn_clicked"<< std::endl;
  int channel = PlotTabWidget->currentIndex ();
  ZoomSample (channel);
}

void ApfelGui::UnzoomSampleBtn_clicked ()
{
  //std::cout <<"UnzoomSampleBtn_clicked"<< std::endl;
  int channel = PlotTabWidget->currentIndex ();
  UnzoomSample (channel);
}

void ApfelGui::RefreshSampleBtn_clicked ()
{
  //std::cout <<"RefreshSampleBtn_clicked"<< std::endl;
  int channel = PlotTabWidget->currentIndex ();
  //std::cout <<"Got current index"<<channel<< std::endl;
  QApplication::setOverrideCursor (Qt::WaitCursor);
  AcquireSample (channel);
  ShowSample (channel);
  RefreshStatus ();
  QApplication::restoreOverrideCursor ();
}

void ApfelGui::DumpBtn_clicked ()
{
//std::cout << "ApfelGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
  EvaluateSlave ();
  AppendTextWindow ("--- ADC Dump ---:");
  APFEL_BROADCAST_ACTION(DumpADCs());

}

void ApfelGui::ClearOutputBtn_clicked ()
{
//std::cout << "ApfelGui::ClearOutputBtn_clicked()"<< std::endl;
  TextOutput->clear ();
  TextOutput->setPlainText ("Welcome to APFEL GUI!\n\t v0.985 of 12-January-2017 by JAM (j.adamczewski@gsi.de)\n");

}


void ApfelGui::ConfigBtn_clicked ()
{
//std::cout << "ApfelGui::ConfigBtn_clicked" << std::endl;

// here file requester and application of set up via gosipcmd
  QFileDialog fd (this, "Select APFEL configuration file", ".", "gosipcmd file (*.gos)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  char buffer[1024];
  // JAM: need to increase default bus wait time to 900us first for febex i2c!
  snprintf (buffer, 1024, "setGosipwait.sh 900");    // output redirection inside QProcess does not work, use helper script
  QString tcom (buffer);
  QString tresult = ExecuteGosipCmd (tcom, 10000);
  AppendTextWindow (tresult);
  QString fileName = flst[0];
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
    snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());
  }
  QString com (buffer);
  QString result = ExecuteGosipCmd (com, 10000);    // this will just execute the command in shell, gosip or not
  AppendTextWindow (result);

  snprintf (buffer, 1024, "setGosipwait.sh 0");    // set back to zero bus wait since we have explicit i2c_sleep elsewhere!
  QString zcom (buffer);
  QString zresult = ExecuteGosipCmd (zcom, 10000);
  AppendTextWindow (zresult);

  ShowBtn_clicked ();
}

void ApfelGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
  fDebug = on;
}

void ApfelGui::HexBox_changed (int on)
{

  unsigned adjustvalue = ADCAdjustValue->text ().toUInt (0, fNumberBase);    // save value in auto adjust field
  fNumberBase = (on ? 16 : 10);
//std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;

  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  ADCAdjustValue->setText (pre + text.setNum (adjustvalue, fNumberBase));    // recover with new base

  int oldsfp = fSFP;
  int oldslave = fSlave;

  if (!AssertNoBroadcast (false))
  {
    fSFP = fSFPSave;
    fSlave = fSlaveSave;
  }
  RefreshView ();    // need to workaround the case that any broadcast was set. however, we do not need full APFEL_BROADCAST_ACTION
  if (!AssertNoBroadcast (false))
  {
    fSFP = oldsfp;
    fSlave = oldslave;
  }

}

void ApfelGui::Slave_changed (int)
{
  //std::cout << "ApfelGui::Slave_changed" << std::endl;
  EvaluateSlave ();
  bool refreshable = AssertNoBroadcast (false);
  RefreshButton->setEnabled (refreshable);

  RefreshChains ();
  if (refreshable)
  {
    // JAM note that we had a problem of prelling spinbox here (arrow buttons only, keyboard arrows are ok)
    // probably caused by too long response time of this slot?
    // workaround is to refresh the view delayed per single shot timer:
    //std::cout << "Timer started" << std::endl;
    QTimer::singleShot (10, this, SLOT (ShowBtn_clicked ()));
    //std::cout << "Timer end" << std::endl;
  }
}


void ApfelGui::SwitchChanged ()
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplySwitch());
  }
}

void ApfelGui::PulserTimeout ()
{
  //std::cout << "ApfelGui::PulserTimeout" << std::endl;

  if (PulseBroadcastCheckBox->isChecked () && !fBroadcasting)
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

  PulserProgressBar->setValue (progress);    // let the progress bar flicker from 0 to 100%
  //std::cout << "Set Progress to"<<progress << std::endl;
}



void ApfelGui::PulserChanged (int apfel)
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplyPulser(apfel));
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
    APFEL_BROADCAST_ACTION(AutoApplyGain(apfel,channel));
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

void ApfelGui::DAC_enterText (int apfel, int dac)
{
  // catch signal editingFinished() from Apfel1_DAClineEdit_1 etc.
  // need to synchronize with the sliders anyway:
  int val = fDACLineEdit[apfel][dac]->text ().toUInt (0, fNumberBase);
  fDACSlider[apfel][dac]->setValue (val & 0x3FF);

  std::cout << "ApfelGui::DAC_enterText=" << apfel << ", dac=" << dac << ", val=" << val << std::endl;
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplyDAC(apfel,dac, val));
  }
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
    APFEL_BROADCAST_ACTION(AutoApplyDAC(apfel,dac, val));
  }

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
    APFEL_BROADCAST_ACTION(DoAutoCalibrate(apfel));
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
    APFEL_BROADCAST_ACTION(DoAutoCalibrateAll());
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      APFEL_BROADCAST_ACTION(UpdateAfterAutoCalibrate(apfel));
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
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplyRefresh(channel, val));
  }

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
    APFEL_BROADCAST_ACTION(SetInverseMapping(on));
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
      APFEL_BROADCAST_ACTION(SetBroadcastPulser());
    }

  }

}


void ApfelGui::PulseTimer_changed (int on)
{
  //std::cout << "PulseTimer_changed to" <<  on << std::endl;

  if (on)
  {
    int period = EvaluatePulserInterval (FrequencyComboBox->currentIndex ());
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
    PulserProgressBar->reset ();
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
  BenchmarkProgressBar->setValue (progress);

  double seconds = fSequencerStopwatch.elapsed () / 1000.0;
  TimeNumber->display (seconds);

  if (!fBenchmark.ProcessBenchmark ())
    fSequencerTimer->stop ();

  APFEL_BROADCAST_ACTION(RefreshView ());
  // instead of changing gui elements during measurement, we update view completely after each command. avoid unwanted effects by widget signals with auto apply!
  // the APFEL_BROADCAST_ACTION is just use to supress further signal slot executions
}

void ApfelGui::StartBenchmarkPressed ()
{
  BoardSetup& theSetup = fSetup[fSFP].at (fSlave);
  printm ("Benchmark Timer has been started!");
  QString apfel1 = ApfelID1_lineEdit->text ();
  QString apfel2 = ApfelID2_lineEdit->text ();
  if (apfel1.isEmpty () || apfel2.isEmpty ())
  {
    printm ("Please specify full id information!");
    return;
  }



  theSetup.SetBoardID (0, apfel1);
  theSetup.SetBoardID (1, apfel2);

  double current=0, voltage=0;
  // here put automatic evaluation of power supply via serial connection:
  if(AutoPowerscanCheckBox->isChecked())
  {
    printm ("Reading power supply settings via serial line...");

    ReadToellnerPower(voltage,current);
    CurrentDoubleSpinBox->setValue(current*1000.0);
    VoltageDoubleSpinBox->setValue(voltage);
  }
  else
  {
     current = CurrentDoubleSpinBox->value () / 1000.0;
     voltage = VoltageDoubleSpinBox->value ();
  }



  // JAM for test of toellner only, we just skip rest of benchmark:
  //return;


  theSetup.SetCurrent (current);
  theSetup.SetVoltage(voltage);

  printm ("Benchmark has been started for sfp %d slave %d", fSFP, fSlave);
  printm ("Board 1:%s Board2:%s Current=%f A Voltage=%f V", apfel1.toLatin1 ().constData (), apfel2.toLatin1 ().constData (),
      current, voltage);

  // here we evaluate a to do list that the timer should process:
  fBenchmark.SetSetup (&theSetup);






  fBenchmark.ResetSequencerList ();
  if (Gain1groupBox->isChecked ())
  {

    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_1));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));

    if (Gain1TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
    if (Gain1TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));

    }
    if (Gain1TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));

    }
    if (Gain1TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }

  if (Gain16groupBox->isChecked ())
  {

    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_16));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));


    if (Gain16TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
    if (Gain16TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (Gain16TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (Gain16TestCurveCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_CURVE, channel));
    }
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_FINALIZE));
  }
  if (Gain32groupBox->isChecked ())
  {

    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_GAIN_32));
    fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_INIT));

    if (Gain32TestAutocalCheckBox->isChecked ())
    {
      fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_AUTOCALIB));
    }
    if (Gain32TestSigmaCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_NOISESAMPLE, channel));
    }
    if (Gain32TestBaselineCheckBox->isChecked ())
    {
      for (int channel = 0; channel < APFEL_ADC_CHANNELS; ++channel)
        fBenchmark.AddSequencerCommand (SequencerCommand (SEQ_BASELINE, channel));
    }
    if (Gain32TestCurveCheckBox->isChecked ())
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
  QString apfel1 = ApfelID1_lineEdit->text ();
  QString apfel2 = ApfelID2_lineEdit->text ();
  QString filename = apfel1.append ("_").append (apfel2).append (".apf");
  SaveConfigBtn_clicked (filename.toLatin1 ().constData ());
}

void ApfelGui::BenchmarkPressed (QAbstractButton* but)
{
  if (but == BenchmarkButtonBox->button (QDialogButtonBox::Apply))
  {
    StartBenchmarkPressed ();
  }
  else if (but == BenchmarkButtonBox->button (QDialogButtonBox::Save))
  {

    SaveBenchmarkPressed ();
  }
  else if (but == BenchmarkButtonBox->button (QDialogButtonBox::Cancel))
  {
    //printm("Benchmark will be canceled.");
    CancelBenchmarkPressed ();
  }
  else if (but == BenchmarkButtonBox->button (QDialogButtonBox::Retry))
  {
    ContinueBenchmarkPressed ();
  }

}


void ApfelGui::ChangeReferenceDataPressed(QAbstractButton* but)
{

  //std::cout << "ChangeReferenceDataPressed" << std::endl;

  if (but == ReferenceLoadButtonBox->button (QDialogButtonBox::Open))
    {
      LoadBenchmarkReferences();
    }
  else if (but == ReferenceLoadButtonBox->button (QDialogButtonBox::RestoreDefaults))
   {
      ReferenceLineEdit->setText("buildin defaults");
      fBenchmark.InitReferenceValues();
   }


}


void ApfelGui::PlotTabChanged (int num)
{
  //std::cout << "PlotTabChanged to "<<num << std::endl;
  RefreshSampleMaxima(num);

}


