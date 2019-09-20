#ifndef GAPGGUI_H
#define GAPGGUI_H


#include "BasicGui.h"
#include "GalapagosSetup.h"
#include "GalapagosMacros.h"

#include <vector>

namespace gapg {

class GalPatternWidget;
class GalPatternDisplay;
class GalKernelWidget;

class GalapagosGui:  public gapg::BasicGui
{
  Q_OBJECT


public:
  GalapagosGui (QWidget* parent = 0);
  virtual ~GalapagosGui ();

  /** get register contents to status structure*/
   void GetRegisters ();

   /** Write value to i2c bus address of currently selected slave. mcp433 chip id and local channel id are specified*/
     int WriteDAC_GalapagosI2c (uint8_t mcpchip, uint8_t chan, uint8_t value);

     /** Read value to i2c bus address of currently selected slave. mcp433 chip id and local channel id are specified*/
     int ReadDAC_GalapagosI2c (uint8_t mcpchip, uint8_t chan);


   /** helper function that either does enable i2c on board, or writes such commands to .gos file*/
   void EnableI2C();

   /** helper function that either does disable i2c on board, or writes such commands to .gos file*/
   void DisableI2C ();


protected:

   gapg::GalPatternWidget* fPatternWidget;
   gapg::GalPatternDisplay* fPatternDisplay;
   gapg::GalKernelWidget* fKernelWidget;

   virtual BasicSetup* CreateSetup();

   virtual void ConnectSlots();

 /** reset current slave, i.e. initialize it to defaults*/
  virtual void ResetSlave ();


  /** update register display*/
  void RefreshView ();



  /** overwrite base class method to adjust waittime*/
  virtual void ApplyFileConfig(int gosipwait=0);


  /** copy values from gui to internal status object*/
  void EvaluateView ();


  /** set register from status structure*/
  void SetRegisters ();


  /** dump current ADC values of currently set GAPG*/
  void Dump();





public slots:


virtual void ReadSettings();

virtual void WriteSettings();

};

}// namespace gapg

#endif
