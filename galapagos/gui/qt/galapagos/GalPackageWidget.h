#ifndef GAPG_GALPACKAGEWIDGET_H
#define GAPG_GALPÄACKAGEWIDGET_H

#include "BasicObjectEditorWidget.h"
#include "GalapagosDefines.h"
#include "GalapagosMacros.h"

#include "GalPackageEditor.h"

#include <QRadioButton>
#include <QComboBox>
#include <kled.h>

namespace gapg
{

class GalPackageWidget: public gapg::BasicObjectEditorWidget
{
  Q_OBJECT

protected:

  /** auxiliary references to channel enabled flags*/
  QRadioButton* fCoreEnabledRadio[GAPG_CORES];


  KLed* fCoreActiveLED[GAPG_CORES];

  QComboBox* fCoreKernelCombo[GAPG_CORES];


  gapg::GalPackageEditor* fPackageEditor;

   /** refresh editor content for pattern id, return unique object id*/
   int RefreshObjectIndex (int ix);

   /** load pattern from file fullname. Returns false if no success*/
   bool LoadObject (const QString& fullname);

   /** save pattern from setup to file fullname. Returns false if no success*/
   bool SaveObject (const QString& fullname, BasicObject* pat);

   virtual bool LoadObjectRequest ();

   /** implement in subclass: dedicated requester for kind of object save*/
   virtual bool SaveObjectRequest ();

   /** implement in subclass: dedicated requester for kind of new object creation. */
   virtual bool NewObjectRequest ();

   /** implement in subclass: dedicated requester for kind of new object deletion. */
   virtual bool DeleteObjectRequest ();

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void StartEditing ();

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void CancelEditing ();

   /** implement in subclass: activate dedicated editor for object kind*/
   virtual void ApplyEditing ();


public:
  GalPackageWidget (QWidget* parent = 0);
  virtual ~GalPackageWidget ();

  /** connection to our slots*/
  virtual void ConnectSlots ();

  /** update register display*/
  virtual void RefreshView ();

  /** put values from gui into setup structure*/
  virtual void EvaluateView ();

  /** take values relevant for our widget from Qt settings file*/
  virtual void ReadSettings (QSettings* set);

  /** put values relevant for our widget from Qt settings file*/
  virtual void WriteSettings (QSettings* set);

  /** set pattern generator hardware active.
   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
  void ApplyGeneratorActive (bool on);

  /** apply change of enabled pattern generator channel  channel
   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
  void ApplyCoreEnabled (int channel, bool on);

  /** evaluate change of enabled pattern generator channel channel*/
  void CoreEnabled_toggled (int channel, bool on);

//  /** apply simulation modechannel channel
//   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
//  void ApplyPackageSimulated (int channel, bool on);

//  /** evaluate simulation mode of channel channel*/
//  void PackageSimulated_toggled (int channel, bool on);

  void ApplyCoreKernel (int channel, int ix);

 // void ApplyCorePattern (int channel, int ix);

  /** evaluate sequence index for channel*/
  void CoreKernel_changed (int channel, int ix);

//  /** evaluate pattern index for channel*/
//  void CorePattern_changed (int channel, int ix);

public slots:

  virtual void CoreEnabled_toggled_all(bool on);

  virtual void CoreEnabled_toggled_group0 (bool on);
  virtual void CoreEnabled_toggled_group1 (bool on);

  GALAGUI_DEFINE_MULTICHANNEL_TOGGLED_16(CoreEnabled)
  ;


  virtual void CoreKernel_changed_all (int ix);

  GALAGUI_DEFINE_MULTICHANNEL_CHANGED_16(CoreKernel)
  ;


  virtual void GeneratorActive_clicked (bool checked);

};

}    // namespace

#endif
