#include "FebexGui.h"

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

#include <sstream>
#include <string.h>
#include <errno.h>
#include <math.h>

// *********************************************************






/*
 *  Constructs a FebexGui which is a child of 'parent', with the
 *  name 'name'.'
 */
FebexGui::FebexGui (QWidget* parent) : GosipGui (parent)
{
 
 
 fImplementationName="FEBEX";
 fVersionString="Welcome to FEBEX GUI!\n\t v0.97 of 10-April-2017 by JAM (j.adamczewski@gsi.de)";


  fFebexWidget=new FebexWidget(this);
  Settings_scrollArea->setWidget(fFebexWidget);
  setWindowTitle(QString("%1 GUI").arg(fImplementationName));


  fFebexWidget->DataTraceCheckBox->setDisabled(); // show the switch, but do not let the user change it

  ClearOutputBtn_clicked ();
 
 

  QObject::connect (fFebexWidget->TriggerUseWindowCheckBox, SIGNAL(toggled(bool)), this, SLOT(TriggerUseWindowChecked(bool)));



 
  QObject::connect (fFebexWidget->AutoAdjustButton, SIGNAL (clicked ()), this, SLOT (AutoAdjustBtn_clicked ()));


  QObject::connect (fFebexWidget->DAC_spinBox_all, SIGNAL(valueChanged(int)), this, SLOT(DAC_spinBox_all_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_00, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox00_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_01, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox01_changed(int))); 
  QObject::connect (fFebexWidget->DAC_spinBox_02, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox02_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_03, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox03_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_04, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox04_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_05, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox05_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_06, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox06_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_07, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox07_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_08, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox08_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_09, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox09_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_10, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox10_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_11, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox11_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_12, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox12_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_13, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox13_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_14, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox14_changed(int)));
  QObject::connect (fFebexWidget->DAC_spinBox_15, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox15_changed(int)));



  /** JAM put references to designer checkboxes into array to be handled later easily: */
  fBaselineBoxes[0]=fFebexWidget->Baseline_Box_00;
  fBaselineBoxes[1]=fFebexWidget->Baseline_Box_01;
  fBaselineBoxes[2]=fFebexWidget->Baseline_Box_02;
  fBaselineBoxes[3]=fFebexWidget->Baseline_Box_03;
  fBaselineBoxes[4]=fFebexWidget->Baseline_Box_04;
  fBaselineBoxes[5]=fFebexWidget->Baseline_Box_05;
  fBaselineBoxes[6]=fFebexWidget->Baseline_Box_06;
  fBaselineBoxes[7]=fFebexWidget->Baseline_Box_07;
  fBaselineBoxes[8]=fFebexWidget->Baseline_Box_08;
  fBaselineBoxes[9]=fFebexWidget->Baseline_Box_09;
  fBaselineBoxes[10]=fFebexWidget->Baseline_Box_10;
  fBaselineBoxes[11]=fFebexWidget->Baseline_Box_11;
  fBaselineBoxes[12]=fFebexWidget->Baseline_Box_12;
  fBaselineBoxes[13]=fFebexWidget->Baseline_Box_13;
  fBaselineBoxes[14]=fFebexWidget->Baseline_Box_14;
  fBaselineBoxes[15]=fFebexWidget->Baseline_Box_15;

  fDACSpinBoxes[0] = fFebexWidget->DAC_spinBox_00;
  fDACSpinBoxes[1] = fFebexWidget->DAC_spinBox_01;
  fDACSpinBoxes[2] = fFebexWidget->DAC_spinBox_02;
  fDACSpinBoxes[3] = fFebexWidget->DAC_spinBox_03;
  fDACSpinBoxes[4] = fFebexWidget->DAC_spinBox_04;
  fDACSpinBoxes[5] = fFebexWidget->DAC_spinBox_05;
  fDACSpinBoxes[6] = fFebexWidget->DAC_spinBox_06;
  fDACSpinBoxes[7] = fFebexWidget->DAC_spinBox_07;
  fDACSpinBoxes[8] = fFebexWidget->DAC_spinBox_08;
  fDACSpinBoxes[9] = fFebexWidget->DAC_spinBox_09;
  fDACSpinBoxes[10] = fFebexWidget->DAC_spinBox_10;
  fDACSpinBoxes[11] = fFebexWidget->DAC_spinBox_11;
  fDACSpinBoxes[12] = fFebexWidget->DAC_spinBox_12;
  fDACSpinBoxes[13] = fFebexWidget->DAC_spinBox_13;
  fDACSpinBoxes[14] = fFebexWidget->DAC_spinBox_14;
  fDACSpinBoxes[15] = fFebexWidget->DAC_spinBox_15;

  fADCLineEdit[0] = fFebexWidget->ADC_Value_00;
  fADCLineEdit[1] = fFebexWidget->ADC_Value_01;
  fADCLineEdit[2] = fFebexWidget->ADC_Value_02;
  fADCLineEdit[3] = fFebexWidget->ADC_Value_03;
  fADCLineEdit[4] = fFebexWidget->ADC_Value_04;
  fADCLineEdit[5] = fFebexWidget->ADC_Value_05;
  fADCLineEdit[6] = fFebexWidget->ADC_Value_06;
  fADCLineEdit[7] = fFebexWidget->ADC_Value_07;
  fADCLineEdit[8] = fFebexWidget->ADC_Value_08;
  fADCLineEdit[9] = fFebexWidget->ADC_Value_09;
  fADCLineEdit[10] = fFebexWidget->ADC_Value_10;
  fADCLineEdit[11] = fFebexWidget->ADC_Value_11;
  fADCLineEdit[12] = fFebexWidget->ADC_Value_12;
  fADCLineEdit[13] = fFebexWidget->ADC_Value_13;
  fADCLineEdit[14] = fFebexWidget->ADC_Value_14;
  fADCLineEdit[15] = fFebexWidget->ADC_Value_15;

  GetSFPChainSetup(); // ensure that any slave has a status structure before we begin clicking...
  show ();
}

FebexGui::~FebexGui ()
{
}






void FebexGui::EnableI2C ()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000080);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2000020);
}

void FebexGui::DisableI2C ()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000000);
}


void FebexGui::ResetSlave()
{
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x00);
  usleep (4000);

//        // disable test data length
  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000);

//        // specify trace length in slices
  WriteGosip (fSFP, fSlave, REG_FEB_TRACE_LEN, FEB_TRACE_LEN);
  // note: we skip verify read here to let this work in broadcast mode!




//        // specify trigger delay in slices
  WriteGosip (fSFP, fSlave,  REG_FEB_TRIG_DELAY, FEB_TRIG_DELAY);
  // note: we skip verify read here to let this work in broadcast mode!


  //        // disable trigger acceptance in febex
  WriteGosip (fSFP, fSlave,   REG_FEB_CTRL, 0);


  //        // enable trigger acceptance in febex
  WriteGosip (fSFP, fSlave,   REG_FEB_CTRL, 1);


//        // set channels used for self trigger signal

  // JAM: the following is reduced version of mbs sample code. instead of arrays for each slave, we just
  // take settings for first device at sfp 0. Should be sufficient for baseline setup until mbs configures all?
  long l_sfp0_feb_ctrl0= 0x01000000;
  long l_sfp0_feb_ctrl1 = 0x0;
  long l_sfp0_feb_ctrl2= 0xffff;
  long l_ev_od_or = (l_sfp0_feb_ctrl0 >> 20) & 0x1;
  long l_pol = (l_sfp0_feb_ctrl0 >> 28) & 0x1;
  long l_trig_mod = (l_sfp0_feb_ctrl0 >> 24) & 0xf;
  long l_dis_cha  =  l_sfp0_feb_ctrl0       & 0x1ffff;
  long l_dat_redu =  l_sfp0_feb_ctrl1       & 0x1ffff;
  long l_ena_trig =  l_sfp0_feb_ctrl2       & 0xffff;


  int trigenabchan= ((l_ev_od_or<<21)|(l_pol<<20)|(l_trig_mod<<16)|l_ena_trig);
  WriteGosip (fSFP, fSlave,  REG_FEB_SELF_TRIG, trigenabchan);



//        // set the step size for self trigger and data reduction
  long l_thresh=0x1ff;
  for (int l_k=0; l_k < FEBEX_CH ; l_k++){
    WriteGosip (fSFP, fSlave, REG_FEB_STEP_SIZE, ( l_k<<24 ) | l_thresh );
  }




  //
//        // reset the time stamp and set the clock source for time stamp counter
  if(fSFP==0 && fSlave==0) // assume clock source at first slave on first chain here
  {
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x0 );
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x7 );
  }
  else
  {
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x0 );
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x5 );
  }




  //
//        // enable/disable no hit in trace data suppression of channel
  WriteGosip (fSFP, fSlave,  REG_DATA_REDUCTION, l_dat_redu);

//        // set channels used for self trigger signal
  WriteGosip (fSFP, fSlave,  REG_MEM_DISABLE, l_dis_cha );

//        // write SFP id for channel header

  WriteGosip (fSFP, fSlave,  REG_HEADER, fSFP);



//        // set trapez parameters for trigger/hit finding
  WriteGosip (fSFP, fSlave,  TRIG_SUM_A_REG, TRIG_SUM_A);
  WriteGosip (fSFP, fSlave,   TRIG_GAP_REG, TRIG_SUM_A + TRIG_GAP);
  WriteGosip (fSFP, fSlave,   TRIG_SUM_B_REG, TRIG_SUM_A  + TRIG_GAP + TRIG_SUM_B );


#ifdef ENABLE_ENERGY_FILTER
#ifdef TRAPEZ
//
//        // set trapez parameters for energy estimation
  WriteGosip (fSFP, fSlave,  ENERGY_SUM_A_REG, ENERGY_SUM_A);
  WriteGosip (fSFP, fSlave,  ENERGY_GAP_REG, ENERGY_SUM_A + ENERGY_GAP);
  WriteGosip (fSFP, fSlave,  ENERGY_SUM_B_REG, ENERGY_SUM_A  + ENERGY_GAP + ENERGY_SUM_B );


#endif // TRAPEZ
#endif // ENABLE_ENERGY_FILTER
  usleep(50);
// enabling after "ini" of all registers (Ivan - 16.01.2013):
  WriteGosip (fSFP, fSlave,   DATA_FILT_CONTROL_REG, DATA_FILT_CONTROL_DAT);
  sleep (1);
  printm("Did Initialize FEBEX for SFP %d Slave %d",fSFP,fSlave);
}



void FebexGui::AutoAdjustBtn_clicked ()
{
  //std::cout <<"AutoAdjustBtn_clicked "<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  GOSIP_BROADCAST_ACTION(AutoAdjust());
  QApplication::restoreOverrideCursor ();
}


void FebexGui::AutoAdjust()
{
  if(!AssertChainConfigured()) return;
   QString targetstring=fFebexWidget->ADCAdjustValue->text ();
   unsigned targetvalue =targetstring.toUInt (0, fNumberBase);
   //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
   for(int channel=0; channel<16;++channel)
     {
       if(fBaselineBoxes[channel]->isChecked())
       {
           int dac=AdjustBaseline(channel,targetvalue);
           fDACSpinBoxes[channel]->setValue (dac);
           AutoApplyRefresh(channel, dac); // once again apply dac settings to immediately see the baseline on gui
           printm("--- Auto adjusted baselines of sfp:%d FEBEX:%d channel:%d to value:%d =>%d permille DAC",fSFP, fSlave,channel, targetvalue, dac);
       }
    }
}



int FebexGui::AdjustBaseline(int channel, int adctarget)
{
  int dac=500; // dac setup in per mille here, start in the middle
  int dacstep=250;
  int validdac=-1;
  int adc=0;
  int escapecounter=10;
  bool upwards=true; // scan direction up or down
  bool changed=false; // do we have changed scan direction?
  bool initial=true; // supress evaluation of scan direction at first cycle
  //std::cout << "FebexGui::AdjustBaseline of channel "<<channel<<" to "<<adctarget<< std::endl;

  double resolution= 1.0/FEBEX_MCP433_MAXVAL * 0xFFF /2; // for 12 bit
    // test if FEBEX is for 14 bit values:
  if(autoApply(channel, 1000)>0xFFF)
        resolution*=4;
  do{
     adc=autoApply(channel, dac); // this gives already mean value of 3 adc samples
     if(adc<0) break; // avoid broadcast
     validdac=dac;
     //std::cout << "FebexGui::AdjustBaseline after autoApply of dac:"<<dac<<" gets adc:"<<adc<<", resolution:"<<resolution<< std::endl;
     if(adc<adctarget){
       dac+=dacstep;
       changed=(!upwards ? true : false);
       upwards=true;
       if(dac>1000) dac = 1000;
     }
     else if (adc>adctarget){
       dac-=dacstep;
       changed=(upwards ? true : false);
       upwards=false;
       if(dac<0) dac=0;
     }
     else break;
     if(changed && !initial && dacstep > 1) dacstep = dacstep >> 1;
     if(dacstep <1) break;
     if(!changed || dacstep==1) escapecounter--; // get us out of loop if user wants to reach value outside adc range, or if we oscillate around target value
     initial=false;
  } while (fabs(adc-adctarget) >= resolution && escapecounter);
  //std::cout << "   FebexGui::AdjustBaseline after loop dac:"<<validdac<<" adc:"<<adc<<", resolution:"<<resolution<< std::endl;
  return validdac;
}









void FebexGui::DumpSlave()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if(!AssertChainConfigured()) return;

    printm("SFP %d DEV:%d :)",fSFP, fSlave);
    for(int adc=0; adc<FEBEX_ADC_NUMADC; ++adc){
      for (int chan=0; chan<FEBEX_ADC_NUMCHAN; ++chan){
        int val=ReadADC_Febex(adc,chan);
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

void FebexGui::ApplyFileConfig(int )
{
    GosipGui::ApplyFileConfig(900); // adjust bus wait time to 900 us
}




void FebexGui::TriggerUseWindowChecked(bool checked)
{
  std::cout << "FebexGui::TriggerUseWindowChecked, val="<<checked << std::endl;
  fFebexWidget->WindowSpeedComboBox->setEnabled(checked);

}


 void FebexGui::DAC_spinBox_all_changed(int val)
{
  //std::cout << "FebexGui::DAC_spinBox_all_changed, val="<<val << std::endl;
   for(int chan=0;chan<16;++chan)
     fDACSpinBoxes[chan]->setValue (val);
  
}


 void FebexGui::DAC_spinBox_changed (int channel, int val)
{
  //if (checkBox_AA->isChecked () && AssertNoBroadcast (false))
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    GOSIP_BROADCAST_ACTION(AutoApplyRefresh(channel, val));
  }

}


 void FebexGui::Any_spinBox00_changed(int val)
{
   DAC_spinBox_changed (0,val);
 }

void FebexGui::Any_spinBox01_changed(int val)
{
  DAC_spinBox_changed (1,val);
}

void FebexGui::Any_spinBox02_changed(int val)
{
  DAC_spinBox_changed (2,val);
}

void FebexGui::Any_spinBox03_changed(int val)
{
  DAC_spinBox_changed (3,val);
}

void FebexGui::Any_spinBox04_changed(int val)
{
  DAC_spinBox_changed (4,val);
}

void FebexGui::Any_spinBox05_changed(int val)
{
  DAC_spinBox_changed (5,val);
}

void FebexGui::Any_spinBox06_changed(int val)
{
  DAC_spinBox_changed (6,val);
}

void FebexGui::Any_spinBox07_changed(int val)
{
  DAC_spinBox_changed (7,val);
}

void FebexGui::Any_spinBox08_changed(int val)
{
  DAC_spinBox_changed (8,val);
}

void FebexGui::Any_spinBox09_changed(int val)
{
  DAC_spinBox_changed (9,val);
}

void FebexGui::Any_spinBox10_changed(int val)
{
  DAC_spinBox_changed (10,val);
}

void FebexGui::Any_spinBox11_changed(int val)
{
  DAC_spinBox_changed (11,val);
}

void FebexGui::Any_spinBox12_changed(int val)
{
  DAC_spinBox_changed (12,val);
}

void FebexGui::Any_spinBox13_changed(int val)
{
  DAC_spinBox_changed (13,val);
}

void FebexGui::Any_spinBox14_changed(int val)
{
  DAC_spinBox_changed (14,val);
}

void FebexGui::Any_spinBox15_changed(int val)
{
  DAC_spinBox_changed (15,val);
}


void FebexGui::AutoApplyRefresh(int channel, int dac)
{
     QString text;
     QString pre;
     fNumberBase == 16 ? pre = "0x" : pre = "";
     int Adc = autoApply (channel, dac);
     fADCLineEdit[channel]->setText (pre + text.setNum (Adc, fNumberBase));
     RefreshStatus ();
}


int FebexGui::autoApply(int channel, int dac)

{ 
  int dacchip,dacchannel, adcchip, adcchannel;
  int value=255-round((dac*255.0/1000.0)) ;
  dacchip= channel/FEBEX_MCP433_NUMCHAN ;
  dacchannel= channel-dacchip*FEBEX_MCP433_NUMCHAN;
  theSetup_GET_FOR_SLAVE_RETURN(FebexSetup);
  theSetup->SetDACValue(dacchip,dacchannel, value);
   
   EnableI2C ();  
   WriteDAC_FebexI2c (dacchip, dacchannel, theSetup->GetDACValue(dacchip, dacchannel));
   DisableI2C ();
   if (!AssertNoBroadcast ())
      return -1;
   int Adc=AcquireBaselineSample(channel);
   return Adc;
  
}


int FebexGui::AcquireBaselineSample(uint8_t febexchan)
{
  if(febexchan >= FEBEX_ADC_NUMADC*FEBEX_ADC_NUMCHAN) return -1;
  int adcchip= febexchan/FEBEX_ADC_NUMCHAN;
  int adcchannel= febexchan-adcchip * FEBEX_ADC_NUMCHAN ;
  int Adc=0;
  for(int t=0; t<FEBEX_ADC_BASELINESAMPLES;++t)
    {
      Adc+=ReadADC_Febex(adcchip,adcchannel);
    }
  Adc=Adc/FEBEX_ADC_BASELINESAMPLES;
  return Adc;
}







void FebexGui::RefreshView ()
{

  std::cout << "FebexGui::RefreshView"<<std::endl;
// display setup structure to gui:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  theSetup_GET_FOR_SLAVE(FebexSetup);


  fFebexWidget->TraceLengthSpinBox->setValue(theSetup->GetTraceLength());
  std::cout << "FebexGui::RefreshView with trace length "<< theSetup->GetTraceLength()<<std::endl;
  fFebexWidget->TrigDelayspinBox->setValue(theSetup->GetTriggerDelay());
  std::cout << "FebexGui::RefreshView with trigger delay "<< theSetup->GetTriggerDelay()<<std::endl;

  fFebexWidget->PolarityCheckBox->setChecked(theSetup->IsPolarityNegative());
  std::cout << "FebexGui::RefreshView with polarity negative  "<< theSetup->IsPolarityNegative()<<std::endl;
  fFebexWidget->EvenOddOrCheckBox->setChecked(theSetup->IsEvenOddOr());
  std::cout << "FebexGui::RefreshView with evenoddor "<< theSetup->IsEvenOddOr()<<std::endl;

//  std::cout << "FebexGui::RefreshView with trigsuma="<< theSetup->GetTriggerSumA()<<std::endl;
//  std::cout << "FebexGui::RefreshView with trigsumb="<< theSetup->GetTriggerSumB()<<std::endl;
//  std::cout << "FebexGui::RefreshView with triggap="<< theSetup->GetTriggerGap()<<std::endl;

  fFebexWidget->TrigSumAspinBox->setValue(theSetup->GetTriggerSumA());
  fFebexWidget->TrigSumBspinBox->setValue(theSetup->GetTriggerSumB());
  fFebexWidget->TrigGapspinBox->setValue(theSetup->GetTriggerGap());


//  std::cout << "FebexGui::RefreshView with esuma="<< theSetup->GetEnergySumA()<<std::endl;
//  std::cout << "FebexGui::RefreshView with esumb="<< theSetup->GetEnergySumB()<<std::endl;
//  std::cout << "FebexGui::RefreshView with egap="<< theSetup->GetEnergyGap()<<std::endl;

  fFebexWidget->EnergySumAspinBox->setValue(theSetup->GetEnergySumA());
  fFebexWidget->EnergySumBspinBox->setValue(theSetup->GetEnergySumB());
  fFebexWidget->EnergyGapspinBox->setValue(theSetup->GetEnergyGap());

  uint8_t tm =theSetup->GetTriggerMethod();
  printf("FebexGui::RefreshView with trigger method 0x%x\n",tm);
  bool usewindow (tm != 0);
  fFebexWidget->TriggerUseWindowCheckBox->setChecked(usewindow);
  if (usewindow)
  {
    int i = 0;
    for (i = 0; i < 4; ++i)
    {
      if ((tm >> i) & 0x1 == 0x1)
        break;
    }
    fFebexWidget->WindowSpeedComboBox->setCurrentIndex (i);
    fFebexWidget->WindowSpeedComboBox->setEnabled(true);
  }
  else
  {
    fFebexWidget->WindowSpeedComboBox->setEnabled(false);
  }


  fFebexWidget->DataTraceCheckBox->setChecked(theSetup->IsSendDataTrace());
  fFebexWidget->FilterTraceCheckBox->setChecked(theSetup->IsSendFilterTrace());
  fFebexWidget->MoreThanOneHitCheckBox->setChecked(theSetup->IsSendMoreThanOneHitsOnly());


  // below for tum addon only:


  for(int channel=0; channel<16;++channel)
     {
          int val=theSetup->GetDACValue(channel);
          int permille=1000 - round((val*1000.0/255.0)) ;
          fDACSpinBoxes[channel]->setValue(permille);
          int adc=AcquireBaselineSample(channel);
          fADCLineEdit[channel]->setText (pre+text.setNum (adc, fNumberBase));
     }
  RefreshChains();
  RefreshStatus();
}





void FebexGui::EvaluateView ()
{
  //std::cout << "FebexGui::EvaluateView"<<std::endl;

  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_SLAVE(FebexSetup);


  theSetup->SetTraceLength(fFebexWidget->TraceLengthSpinBox->value());
  theSetup->SetTriggerDelay(fFebexWidget->TrigDelayspinBox->value());

  printf("FebexGui::EvaluateView with raw control word before: 0x%x\n", theSetup->GetRawControl_0());


  //std::cout << "FebexGui::EvaluateView with polarity negative  "<< fFebexWidget->PolarityCheckBox->isChecked()<<std::endl;
  theSetup->SetPolarityNegative(fFebexWidget->PolarityCheckBox->isChecked());
  //std::cout << "FebexGui::EvaluateView with evenoddor  "<< fFebexWidget->EvenOddOrCheckBox->isChecked()<<std::endl;
  theSetup->SetEvenOddOr(fFebexWidget->EvenOddOrCheckBox->isChecked());

  theSetup->SetTriggerSumA(fFebexWidget->TrigSumAspinBox->value());
  theSetup->SetTriggerSumB(fFebexWidget->TrigSumBspinBox->value());
  theSetup->SetTriggerGap(fFebexWidget->TrigGapspinBox->value());
//
  theSetup->SetEnergySumA(fFebexWidget->EnergySumAspinBox->value());
    theSetup->SetEnergySumB(fFebexWidget->EnergySumBspinBox->value());
    theSetup->SetEnergyGap(fFebexWidget->EnergyGapspinBox->value());


// TODO: trigger method
    uint8_t tm=0;
    bool usewindow (fFebexWidget->TriggerUseWindowCheckBox->isChecked());
    if(!usewindow)
    {
        tm=0x0;
    }
    else
    {
      int ix= fFebexWidget->WindowSpeedComboBox->currentIndex();
      if(ix>3) ix=3;
      tm = (0x01 << ix);
    }
    //printf("FebexGui::EvaluateView finds trigger method 0x%x\n",tm);
    theSetup->SetTriggerMethod(tm);

    // end trigger method


    printf("FebexGui::EvaluateView with raw control word after set trigger method: 0x%x\n", theSetup->GetRawControl_0());



    theSetup->SetSendDataTrace(fFebexWidget->DataTraceCheckBox->isChecked());
    theSetup->SetSendFilterTrace(fFebexWidget->FilterTraceCheckBox->isChecked());
    theSetup->SetSendMoreThanOneHitsOnly(fFebexWidget->MoreThanOneHitCheckBox->isChecked());



// JAM improved this by looping over spinbox references
  for(int channel=0; channel<16;++channel)
     {
          int permille=fDACSpinBoxes[channel]->value();
          int value=255 - round((permille*255.0/1000.0)) ;
          theSetup->SetDACValue(channel, value);
     }

}



void FebexGui::SetRegisters ()
{
  theSetup_GET_FOR_SLAVE(FebexSetup);

  QApplication::setOverrideCursor (Qt::WaitCursor);


  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000); // disable test data length

  WriteGosip (fSFP, fSlave, REG_FEB_TRACE_LEN, theSetup->GetTraceLength());
  WriteGosip (fSFP, fSlave, REG_FEB_TRIG_DELAY, theSetup->GetTriggerDelay());


  // disable trigger acceptance in febex
  WriteGosip (fSFP, fSlave, REG_FEB_CTRL, 0);
  // enable trigger acceptance in febex
  WriteGosip (fSFP, fSlave, REG_FEB_CTRL, 1);

  // set channels used for self trigger signal
  int l_12_14, l_pol, l_trig_mod, l_ev_od_or, l_dis_cha, l_dat_redu, l_ena_trig;
  l_pol = theSetup->IsPolarityNegative () ? 0x1 : 0;
  l_ev_od_or = theSetup->IsEvenOddOr () ? 0x1 : 0;
  l_trig_mod = theSetup->GetTriggerMethod () & 0xF;
  l_ena_trig = theSetup->GetRawControl_2 () & 0xFFFF;


  printf("FebexGui::SetRegisters with raw control word: 0x%x\n", theSetup->GetRawControl_0());
  printf("FebexGui::SetRegisters with evenoddor 0x%x\n",l_ev_od_or);
  printf("FebexGui::SetRegisters with polarity 0x%x\n",l_pol);
  printf("FebexGui::SetRegisters with trigger method 0x%x\n",l_trig_mod);
  printf("FebexGui::SetRegisters with trigger enabled 0x%x\n",l_ena_trig);


  WriteGosip (fSFP, fSlave, REG_FEB_SELF_TRIG, (l_ev_od_or<<21)|(l_pol<<20)|(l_trig_mod<<16)|l_ena_trig);

  // set the step size for self trigger and data reduction
  for (int c = 0; c < FEBEX_CH; ++c)
        {
          int l_thresh= theSetup->GetThreshold(c);
          WriteGosip (fSFP, fSlave, REG_FEB_STEP_SIZE, ( c << 24 ) | l_thresh);
        }

  // TODO TODO
  // reset the time stamp and set the clock source for time stamp counter
//         if( l_i==clk_source[0] &&   l_j==clk_source[1] )
//         {
//           l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME,0x0 );
//           l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME,0x7 );
//           //l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME,0x3 );
//         }
//         else
//         {
//           l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME, 0x0 );
//           l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME, 0x5 );
//           //l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME, 0x1 );
//         }


  // enable/disable no hit in trace data suppression of channel
  l_dat_redu=theSetup->GetRawControl_1() & 0x1ffff;
   WriteGosip (fSFP, fSlave, REG_DATA_REDUCTION, l_dat_redu);


   // set channels used for self trigger signal
   l_dis_cha=theSetup->GetRawControl_0() & 0x1FFFF;
   WriteGosip (fSFP, fSlave, REG_MEM_DISABLE, l_dis_cha);



  // set trapez parameters for trigger/hit finding:
  WriteGosip (fSFP, fSlave, TRIG_SUM_A_REG, theSetup->GetTriggerSumA());
  WriteGosip (fSFP, fSlave, TRIG_GAP_REG, theSetup->GetTriggerSumA() + theSetup->GetTriggerGap());
  WriteGosip (fSFP, fSlave, TRIG_SUM_B_REG, theSetup->GetTriggerSumA() + theSetup->GetTriggerGap() + theSetup->GetTriggerSumB() +1);

  WriteGosip (fSFP, fSlave, ENERGY_SUM_A_REG, theSetup->GetEnergySumA());
  WriteGosip (fSFP, fSlave, ENERGY_GAP_REG,  theSetup->GetEnergySumA() + theSetup->GetEnergyGap());
  WriteGosip (fSFP, fSlave, ENERGY_SUM_B_REG, theSetup->GetEnergySumA() + theSetup->GetEnergyGap() + theSetup->GetEnergySumB()+1);




  usleep(50);
    // enabling after "ini" of all registers (Ivan - 16.01.2013):
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, theSetup->GetFilterControl());





  EnableI2C ();    // must be done since mbs setup program may shut i2c off at the end


      for (int m = 0; m < FEBEX_MCP433_NUMCHIPS; ++m)
    {
      for (int c = 0; c < FEBEX_MCP433_NUMCHAN; ++c)
       {
          WriteDAC_FebexI2c (m, c, theSetup->GetDACValue(m, c));
       }
    }

  DisableI2C ();
  QApplication::restoreOverrideCursor ();

}

void FebexGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  theSetup_GET_FOR_SLAVE(FebexSetup);
  QApplication::setOverrideCursor (Qt::WaitCursor);

  std::cout << "FebexGui::GetRegisters()"<<std::endl;

  // start with basic settings:
  int dat=0;

  dat = ReadGosip (fSFP, fSlave, REG_FEB_TRACE_LEN);
    theSetup->SetTraceLength(dat & 0xFFFF);

  dat = ReadGosip (fSFP, fSlave, REG_FEB_TRIG_DELAY);
  theSetup->SetTriggerDelay(dat & 0xFFFF);


  uint16_t trigsum_a, trigsum_b, trig_gap;
  dat = ReadGosip (fSFP, fSlave, TRIG_SUM_A_REG);
  trigsum_a = (dat & 0xFFFF);
  dat = ReadGosip (fSFP, fSlave, TRIG_GAP_REG);
  trig_gap= (dat & 0xFFFF) - trigsum_a;
  dat = ReadGosip (fSFP, fSlave, TRIG_SUM_B_REG);
  trigsum_b = (dat & 0xFFFF) - trig_gap - trigsum_a - 1;
  theSetup->SetTriggerSumA(trigsum_a);
  theSetup->SetTriggerSumB(trigsum_b);
  theSetup->SetTriggerGap(trig_gap);

  uint16_t esum_a, esum_b, e_gap;


  dat = ReadGosip (fSFP, fSlave, ENERGY_SUM_A_REG);
  esum_a=(dat & 0xFFFF);
  dat = ReadGosip (fSFP, fSlave, ENERGY_GAP_REG);
  e_gap= (dat & 0xFFFF) - esum_a;
  dat = ReadGosip (fSFP, fSlave, ENERGY_SUM_B_REG);
  esum_b = (dat & 0xFFFF) - e_gap - esum_a - 1;
  theSetup->SetEnergySumA(esum_a);
  theSetup->SetEnergySumB(esum_b);
  theSetup->SetEnergyGap(e_gap);


  //  l_12_14   [0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 31) & 0x1;
  //       l_pol     [0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 28) & 0x1;
  //       l_trig_mod[0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 24) & 0xf;
  //       l_ev_od_or[0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 20) & 0x1;
  //       l_dis_cha [0][l_j] =  l_sfp0_feb_ctrl0[l_j]        & 0x1ffff;
  //       l_dat_redu[0][l_j] =  l_sfp0_feb_ctrl1[l_j]        & 0x1ffff;
  //       l_ena_trig[0][l_j] =  l_sfp0_feb_ctrl2[l_j]        & 0xffff;


  //  l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_SELF_TRIG,
  //    ((l_ev_od_or[l_i][l_j]<<21)|(l_pol[l_i][l_j]<<20)|(l_trig_mod[l_i][l_j]<<16)|l_ena_trig[l_i][l_j]) );
  //
  //

  //  l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_STEP_SIZE, (( l_k<<24 ) | l_thresh[l_i][l_j]

  //l_stat = f_pex_slave_wr (l_i, l_j, REG_DATA_REDUCTION, l_dat_redu[l_i][l_j]);

  //l_stat = f_pex_slave_wr (l_i, l_j, REG_MEM_DISABLE, l_dis_cha[l_i][l_j] );

  int l_12_14, l_pol, l_trig_mod, l_ev_od_or, l_dis_cha, l_dat_redu, l_ena_trig;

  dat= ReadGosip (fSFP, fSlave,REG_MEM_DISABLE);
  l_dis_cha = dat & 0x1FFFF;
  uint32_t reg=theSetup->GetRawControl_0();
  reg = reg & ~0x1FFFF; // clear old mask first
  reg |= l_dis_cha; // activate current mask
  theSetup->SetRawControl_0(reg);

  dat= ReadGosip (fSFP, fSlave,REG_FEB_SELF_TRIG);
  l_ev_od_or = (dat >> 21) & 0x1;
  l_pol = (dat >> 20) & 0x1;
  l_trig_mod = (dat >> 16) & 0xF;
  l_ena_trig = dat % 0xFFFF;

  printf("FebexGui::GetRegisters with evenoddor 0x%x\n",l_ev_od_or);
  printf("FebexGui::GetRegisters with polarity 0x%x\n",l_pol);
  printf("FebexGui::GetRegisters with trigger method 0x%x\n",l_trig_mod);
  printf("FebexGui::GetRegisters with trigger enabled 0x%x\n",l_ena_trig);


  theSetup->SetPolarityNegative(l_pol);
  theSetup->SetEvenOddOr(l_ev_od_or);
  theSetup->SetTriggerMethod(l_trig_mod);
  theSetup->SetRawControl_2(l_ena_trig);

  dat = ReadGosip (fSFP, fSlave,REG_DATA_REDUCTION);
  theSetup->SetRawControl_1(dat & 0x1ffff);


  // TODO: how to select channel for reading back the thresholds?
  for (int c = 0; c < FEBEX_CH; ++c)
      {
        dat = ReadGosip (fSFP, fSlave, REG_FEB_STEP_SIZE);
        int channel = (dat >> 24) & 0xF; // maybe just shift register with channel masked.
        //std::cout << "FebexGui::GetRegisters with threshold("<< channel<<") = "<< (dat & 0x1FF) <<std::endl;
        theSetup->SetThreshold(channel, dat & 0x1FF);
      }


  dat=ReadGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG);
  theSetup->SetFilterControl(dat);
  printf("FebexGui::GetRegisters with data filter control 0x%x\n",dat);


/// TODO: mode to disable tum addon!!!!!!!!!!!!!!!!!

 // this one is for TUM addon:
 EnableI2C ();
 for (int m = 0; m < FEBEX_MCP433_NUMCHIPS; ++m)
    {
      for (int c = 0; c < FEBEX_MCP433_NUMCHAN; ++c)
       {
         
	  int val=ReadDAC_FebexI2c (m, c); 
	  //std::cout<<"GetRegisters val="<<val<<std::endl;
  
	  if(val<0){
	  AppendTextWindow("GetRegisters has error!");
	  return; // TODO error message 
	  }
	  theSetup->SetDACValue(m, c,val);
	 
       }
    }
  DisableI2C ();
  QApplication::restoreOverrideCursor ();
}






    
int FebexGui::ReadDAC_FebexI2c (uint8_t mcpchip, uint8_t chan)
{
  int val = 0;
  if(mcpchip>4){
     AppendTextWindow ("#Error: ReadDAC_FebexI2c with illegal chip number!");
     return -1;
   }
  int channeloffset=GetChannelOffsetDAC(chan);
  if(channeloffset<0) return -2;

  int dat=FEBEX_MCP433_BASE_READ + mcpchip* FEBEX_MCP433_OFFSET + channeloffset;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat); // first send read request address
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, FEBEX_MCP433_REQUEST_READ); // read request command

  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1); // read out the value
  if(val < 0) return val; // error case, propagate it upwards
  return (val & 0xFF); // mask to use only l.s. byte
}



int  FebexGui::ReadADC_Febex (uint8_t adc, uint8_t chan)
{
  if(adc>FEBEX_ADC_NUMADC || chan > FEBEX_ADC_NUMCHAN) return -1;

  int val=0;
  int dat=(adc << 3) + chan; //l_wr_d  = (l_k*4) + l_l;

  WriteGosip (fSFP, fSlave, FEBEX_ADC_PORT, dat); // first specify channel number

  val = ReadGosip (fSFP, fSlave, FEBEX_ADC_PORT); // read back the value

  // check if channel id matches the requested ones:
  if ( ((val >> 24) & 0xf) != dat)
      {
         printm ("#Error: ReadADC_Febex channel id mismatch, requested 0x%x, received 0x%x",dat, (val>>24));
         return -1;
      }


  return (val & 0x3fff);


}







int FebexGui::GetChannelOffsetDAC(uint8_t chan)
{
  int channeloffset;
  switch(chan)
   {
     case 0:
       channeloffset=0x1000; // JAM note that order of first channels is swapped (verified with go4 readout)
       break;
     case 1:
       channeloffset=0x0000;
     break;
     case 2:
       channeloffset=0x6000;
       break;
     case 3:
       channeloffset=0x7000;
     break;
     default:
       AppendTextWindow ("#Error: GetChannelOffsetDAC with illegal channel number!");
       return -2;
   };
  return channeloffset;

}


int FebexGui::WriteDAC_FebexI2c (uint8_t mcpchip, uint8_t chan, uint8_t value)
{
  // value byte: 80: middle, max: ff, min:0
  //(0:  shift to max adc value)
  if(mcpchip>4){
    AppendTextWindow ("#Error: WriteDAC_FebexI2c with illegal chip number!");
    return -1;
  }
  int channeloffset=GetChannelOffsetDAC(chan);
  if(channeloffset<0) return -2;

  int dat=FEBEX_MCP433_BASE_WRITE + mcpchip* FEBEX_MCP433_OFFSET + channeloffset;
  dat+=(value & 0xFF);

  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  return 0;
}

