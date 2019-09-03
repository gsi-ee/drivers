#ifndef GAPG_BASIC_SUBWIDGET_H
#define GAPG_BASIC_SUBWIDGET_H

#include <QWidget>

class QSettings;

/** base class for all subwidgets in the galapagos mdi gui.
 * Provides interface for refresh and update and shortcut to setup*/

namespace gapg
{

class BasicSetup;
class BasicGui;

class BasicSubWidget: public QWidget
{
  Q_OBJECT

protected:

  /** shortcut to parent setup structure. TODO: put this into common base class**/
  BasicSetup* fSetup;

  /** Backpointer to parent gui*/
  BasicGui* fParent;

  /** Descriptor of this subwindow*/
  QString fImpName;

  /** remember most recent file directory*/
  QString fLastFileDir;

public:
  BasicSubWidget (QWidget* parent = 0);

  virtual ~BasicSubWidget ();

  void SetBasicParent (gapg::BasicGui* parent);

  bool IsAutoApply ();

  /** connection to our slots*/
  virtual void ConnectSlots () =0;

  /** update register display*/
  virtual void RefreshView () =0;

  /** put values from gui into setup structure*/
  virtual void EvaluateView ()=0;

  /** take values relevant for our widget from Qt settings file*/
  virtual void ReadSettings (QSettings* set)=0;

  /** put values relevant for our widget from Qt settings file*/
  virtual void WriteSettings (QSettings* set) =0;

};

}    // namespace

#endif
