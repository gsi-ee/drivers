#ifndef GALSUBWIDGET_H
#define GALSUBWIDGET_H


#include <QWidget>

class BasicSetup;
class QSettings;
#include "GalapagosGui.h";
#include "GalapagosMacros.h"
#include "GalapagosDefines.h"

/** base class for all subwidgets in the galapagos mdi gui.
 * Provides interface for refresh and update and shortcut to setup*/

class GalSubWidget: public QWidget
{
  Q_OBJECT

protected:

  /** shortcut to parent setup structure. TODO: put this into common base class**/
   BasicSetup* fSetup;

   /** Backpointer to parent gui*/
   GalapagosGui* fParent;

   /** Descriptor of this subwindow*/
   QString fImpName;

   /** remember most recent file directory*/
   QString fLastFileDir;


public:
 GalSubWidget (QWidget* parent = 0);
  virtual ~GalSubWidget ();


  void SetGalParent(GalapagosGui* parent)
    {
      fParent=parent;
      fSetup=fParent->GetSetup();
      fImpName=fParent->fImplementationName;
    }


  bool IsAutoApply()
    {
      return fParent->IsAutoApply();
    }

 /** connection to our slots*/
 virtual void ConnectSlots() {;}


 /** update register display*/
   virtual void RefreshView () {;}

   /** put values from gui into setup structure*/
   virtual void EvaluateView () {;}

   /** take values relevant for our widget from Qt settings file*/
   virtual void ReadSettings(QSettings* set){;}

   /** put values relevant for our widget from Qt settings file*/
   virtual void WriteSettings(QSettings* set){;}





};

#endif
