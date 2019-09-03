#ifndef GAPG_BASICGUI_H
#define GAPB_BASICGUI_H

#include "ui_BasicGui.h"



//#include "ui_BasicMainwindow.h"
#include <stdio.h>
#include <stdint.h>
#include <QProcess>
#include <QString>
#include <QSettings>

#include <iostream>
#include <vector>

class QSignalMapper;





#define GAPG_ACTION(X) \
if(BasicGui::fInAction==false) { \
  BasicGui::fInAction=true;  \
X; \
BasicGui::fInAction=false;\
}




#define GAPG_LOCK_SLOT \
if(BasicGui::fSlotGuard) return; \
BasicGui::fSlotGuard =true;

#define GAPG_UNLOCK_SLOT \
BasicGui::fSlotGuard =false;


/** JAM2018: the following pattern is used in most of the gui slots for autoapply:*/
#define GAPG_AUTOAPPLY(X)\
if (IsAutoApply() && !BasicGui::fInAction)\
     {\
       GAPG_ACTION(X);\
     }\



///** JAM2017 helper macro to cast setup structure to implementation class later.
// * Afterwards, pointer theSetup-> can be used to access special members*/
#define theSetup_GET_FOR_CLASS(X) \
X* theSetup = dynamic_cast<X*>(fSetup);\
if(theSetup==0) {\
  printm("--- Could not get setup structure X - NEVER COME HERE"); \
  return;\
}

#define theSetup_GET_FOR_CLASS_RETURN(X) \
X* theSetup = dynamic_cast<X*>(fSetup);\
if(theSetup==0) {\
  printm("--- Could not get setup structure X - NEVER COME HERE"); \
  return -1;\
}

#define theSetup_GET_FOR_CLASS_RETURN_BOOL(X) \
X* theSetup = dynamic_cast<X*>(fSetup);\
if(theSetup==0) {\
  printm("--- Could not get setup structure X - NEVER COME HERE"); \
  return false;\
}
/** this define will switch between direct call of galapagos lib or external shell call of galapcmd*
 * note: we need to call "make nogalapagos" if we disable this define here!
 * note2: this define is enabled from top Makefile when building regular "make all"*/
//#define USE_GALAPAGOS_LIB 1

#ifdef USE_GALAPAGOS_LIB
extern "C"
{
#include "galapagos/libgalapagos.h"
}
//#else
//// provide dummy structure although never filled by driver:
//#define PEX_SFP_NUMBER 4
//struct pex_sfp_links{
//    int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
//};

#endif


namespace gapg {

class BasicSubWidget;
class BasicSetup;



class BasicGui: public QMainWindow, public Ui::BasicGui
{
  Q_OBJECT

  friend class BasicSubWidget;
  friend class BasicObjectEditorWidget;

protected:

  std::vector<BasicSubWidget*> fSubWidgets;

  QString fLastFileDir;

public:
  BasicGui (QWidget* parent = 0);
  virtual ~BasicGui ();

  void AddSubWindow(gapg::BasicSubWidget* sub);



  int GetNumberBase(){return fNumberBase;}


  bool IsAutoApply(){return checkBox_AA->isChecked();}


  void AppendTextWindow (const QString& text);

  void AppendTextWindow (const char* txt)
         {
           QString buf (txt);
           AppendTextWindow (buf);
         }

  void FlushTextWindow();

  void ShowStatusMessage (const QString& text);

  BasicSetup* GetSetup(){return fSetup;}



  /** Read from address from sfp and slave, returns value*/
    int ReadGAPG (int address);

    /** Write value to address from sfp and slave*/
    int WriteGAPG (int address, int value);

    /** Save value to currently open *.gos configuration file*/
    int SaveGAPG (int address, int value);

    /** execute (galap) command in shell. Return value is output of command*/
    QString ExecuteGAPGCmd (QString& command,  int timeout=5000);

    /** get register contents from hardware to status structure*/
      virtual void GetRegisters ();


  /** singleton pointer to forward galapagos lib output, also useful without galapagos lib:*/
static BasicGui* fInstance;

/** this flag protects some slots during broadcast write mode ? TODO: redundant with slotguard?*/
 static bool fInAction;


 /** protect mutually dependent slots by this.*/
  static bool fSlotGuard;

protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif


  /** handle mainwindow settings by this*/
  QSettings* fSettings;

  /* for mdi windows Signals*/
  QSignalMapper*     fWinMapper;

  /** Name of the gui implementation*/
  QString fImplementationName;

  /** Versionstring to be printed on terminal*/
  QString fVersionString;

  /** for saving of configuration, we now have setup structures for all slaves.
   * array index is sfp, vector index is febex in chain
   * new: for gui framework use pointers + factory method*/
  //std::vector<BasicSetup*> fSetup[4];

  /** we only need one setup structure since we do not have different slaves**/
  BasicSetup* fSetup;



  /** text debug mode*/
  bool fDebug;

  /** save configuration file instead of setting device values*/
  bool fSaveConfig;



   /** indicate full screen mode*/
   bool fFullScreen;


   /** may change display of subwindows (tabs=true, separate windows=false)mode*/
   bool fMdiTabViewMode;

  /** base for number display (10 or 16)*/
  int fNumberBase;





#ifdef USE_GALAPAGOS_LIB



  /** file descriptor on galapagos device*/
  int fPexFD;

  /** speed down galapagos io with this function from Nik*/
  void I2c_sleep ();

#endif





  /** configuration output file handle*/
    FILE* fConfigFile;



  /** open configuration file for writing*/
    int OpenConfigFile(const QString& fname);

    /** guess what...*/
    int CloseConfigFile();

    /** append text to currently open config file*/
    int WriteConfigFile(const QString& text);


//  /** update febex device index display*/
  void RefreshStatus ();


  void DebugTextWindow (const char*txt)
  {
    if (fDebug)
      AppendTextWindow (txt);
  }
  void DebugTextWindow (const QString& text)
  {
    if (fDebug)
      AppendTextWindow (text);
  }


  void BuildSetup()
  {
    if(fSetup==0) fSetup=CreateSetup();
  }

  /**
   * The following methods are the interface to the implementation:
   */

  /** initialize all required slots. By default, we init slots of subwindos*/
  virtual void ConnectSlots();

  /** update gui display from status structure*/
  virtual void RefreshView ();

  /** put gui values into status structure*/
   virtual void EvaluateView ();

   /** set register contents from status structure to hardware*/
   virtual void SetRegisters ();

   /** evaluate gui display and put it immediately to hardware.
    * default beaviour is using just  EvaluateView() and SetRegisters (),
    * this may be overwritten*/
   virtual void ApplyGUISettings();


   /** get registers and write them to config file*/
   virtual void SaveRegisters();

   /** generic reset/init call for slave*/
   virtual void ResetSlave();

   /** generic call for dump registers*/
   virtual void Dump();

   /** save configuration to a file*/
   virtual void SaveConfig();


   /** apply configuration from a file to the hardware.
    * Subclass implementation may call this functions with galapwait  time in microseconds
    * for crucial frontend timing*/
   virtual void ApplyFileConfig(int galapwait=0);


   /* Factory method for the frontend setup structures.
    * To be re-implemented in subclass*/
   virtual BasicSetup* CreateSetup();


   virtual void closeEvent( QCloseEvent * ce );



   void storePanelGeometry(QWidget* w, const QString& kind);

   QSize lastPanelSize(const QString& kind, int dfltwidth=450, int dfltheight=250);

   QPoint lastPanelPos(const QString& kind);


public slots:
  virtual void ShowBtn_clicked();
  virtual void ApplyBtn_clicked ();
  virtual void ResetBoardBtn_clicked ();
  virtual void ResetSlaveBtn_clicked ();
  virtual void DumpBtn_clicked ();
  virtual void ClearOutputBtn_clicked ();
  virtual void ConfigBtn_clicked ();
  virtual void SaveConfigBtn_clicked ();
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void about();
  void windowsMenuAboutToShow();
  void windowsMenuActivated( int id );
  void MinAllWindows();
  void ToggleFullScreenSlot();
  void ToggleSubwindowModeSlot();

  /** read Qt settings (window size etc.)*/
   virtual void ReadSettings();

   /** write Qt settings (window size etc.)*/
   virtual void WriteSettings();


};


} // namespace

#endif
