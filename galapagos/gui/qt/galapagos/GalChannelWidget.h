#ifndef GALCHANNELGWIDGET_H
#define GALCHANNELGWIDGET_H

#include "ui_GalChannelWidget.h"
#include "GalSubWidget.h"


#include <QRadioButton>
#include <QComboBox>
#include <kled.h>

class GalChannelWidget: public GalSubWidget, public Ui::GalChannelWidget
{
  Q_OBJECT

protected:

   /** auxiliary references to channel enabled flags*/
   QRadioButton* fChannelEnabledRadio[GAPG_CHANNELS];

    /** auxiliary references to channel simulation flags*/
   QRadioButton* fChannelSimulatedRadio[GAPG_CHANNELS];

   KLed* fChannelActiveLED[GAPG_CHANNELS];

   QComboBox* fChannelSequenceCombo[GAPG_CHANNELS];
   QComboBox* fChannelPatternCombo[GAPG_CHANNELS];

public:
 GalChannelWidget (QWidget* parent = 0);
  virtual ~GalChannelWidget ();



 /** connection to our slots*/
 virtual void ConnectSlots();


 /** update register display*/
 virtual void RefreshView ();

 /** put values from gui into setup structure*/
 virtual void EvaluateView ();

 /** set pattern generator hardware active.
   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
  void ApplyGeneratorActive(bool on);

  /** apply change of enabled pattern generator channel  channel
   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
  void ApplyChannelEnabled(int channel, bool on);

    /** evaluate change of enabled pattern generator channel channel*/
  void ChannelEnabled_toggled(int channel, bool on);

  /** apply simulation modechannel channel
    * This function is capable of usage in GAPG_AUTOAPPLY macro*/
   void ApplyChannelSimulated(int channel, bool on);

     /** evaluate simulation mode of channel channel*/
   void ChannelSimulated_toggled(int channel, bool on);

   void ApplyChannelSequence(int channel, int ix);

   void ApplyChannelPattern(int channel, int ix);


   /** evaluate sequence index for channel*/
   void ChannelSequence_changed(int channel, int ix);

   /** evaluate pattern index for channel*/
   void ChannelPattern_changed (int channel, int ix);


public slots:


virtual void ChannelEnabled_toggled_all(bool on);

virtual void ChannelEnabled_toggled_group0(bool on);
virtual void ChannelEnabled_toggled_group1(bool on);


 GALAGUI_DEFINE_MULTICHANNEL_TOGGLED_16(ChannelEnabled);


 virtual void ChannelSimulated_toggled_all(bool on);

 GALAGUI_DEFINE_MULTICHANNEL_TOGGLED_16(ChannelSimulated);


 virtual void ChannelSequence_changed_all(int ix);

 GALAGUI_DEFINE_MULTICHANNEL_CHANGED_16(ChannelSequence);

 virtual void ChannelPattern_changed_all(int ix);

 GALAGUI_DEFINE_MULTICHANNEL_CHANGED_16(ChannelPattern);

virtual void GeneratorActive_clicked(bool checked);



};

#endif
