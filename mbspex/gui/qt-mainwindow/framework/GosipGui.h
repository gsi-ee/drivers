#ifndef GOSIPGUI_H
#define GOSIPGUI_H

//#include "ui_GosipGui.h"
#include "ui_GosipMainwindow.h"
#include <stdio.h>
#include <stdint.h>
#include <QProcess>
#include <QString>
#include <QSettings>

class QSignalMapper;


/** JAM The following nice define handles all explicit broadcast actions depending on the currently set slave*/
#define GOSIP_BROADCAST_ACTION(X) \
if(fBroadcasting==false) { \
fBroadcasting=true;  \
int oldslave = fSlave; \
int oldchan = fSFP; \
if (AssertNoBroadcast (false)) \
 { \
   fBroadcasting=false;\
   if (AssertChainConfigured(true)) X; \
 } \
 else if (fSFP < 0) \
 { \
   for (int sfp = 0; sfp < 4; ++sfp) \
   {\
     if (fSFPChains.numslaves[sfp] == 0) \
       continue; \
     fSFP = sfp; \
     if (fSlave < 0) \
     { \
       for (int feb = 0; feb < fSFPChains.numslaves[sfp]; ++feb) \
       { \
         fSlave = feb; \
         X; \
       } \
     } \
     else \
     { \
       X;\
     }\
   }\
 } \
 else if (fSlave< 0) \
 { \
   for (int feb = 0; feb < fSFPChains.numslaves[fSFP]; ++feb) \
       { \
         fSlave = feb; \
         X; \
       } \
 } \
 else \
 { \
   AppendTextWindow ("--- NEVER COME HERE: undefined broadcast mode ---:"); \
 } \
fSlave= oldslave;\
fSFP= oldchan; \
fBroadcasting=false;\
}


//#define GOSIP_LOCK_SLOT \
//if(fSlotGuard) return; \
//fSlotGuard =true;
//
//#define GOSIP_UNLOCK_SLOT \
//fSlotGuard =false;

// JAM2020: redefine this macros for usage in non-subclasses
#define GOSIP_LOCK_SLOT \
if(GosipGui::fInstance->IsSlotGuard()) return; \
GosipGui::fInstance->SetSlotGuard(true);

#define GOSIP_UNLOCK_SLOT \
GosipGui::fInstance->SetSlotGuard(false);


/** JAM2018: the following pattern is used in most of the gui slots for autoapply:*/
#define GOSIP_AUTOAPPLY(X)\
if (IsAutoApply() && !fBroadcasting)\
     {\
       EvaluateSlave ();\
       GOSIP_BROADCAST_ACTION(X);\
     }\



/** JAM2017 helper macro to cast setup structure to implementation class later.
 * Afterwards, pointer theSetup-> can be used to access special members*/
#define theSetup_GET_FOR_SLAVE(X) \
if (!AssertNoBroadcast(false) || !AssertChainConfigured(true)) return;\
X* theSetup = dynamic_cast<X*>(fSetup[fSFP].at (fSlave));\
if(theSetup==0) {\
  printm("--- Could not get setup structure X for sfp:%d slave:%d",fSFP,fSlave); \
  return;\
}

#define theSetup_GET_FOR_SLAVE_RETURN(X) \
if(!AssertNoBroadcast(false) || !AssertChainConfigured(true)) return -1;\
X* theSetup = dynamic_cast<X*>(fSetup[fSFP].at (fSlave));\
if(theSetup==0) {\
  printm("--- Could not get setup structure X for sfp:%d slave:%d",fSFP,fSlave); \
  return -1;\
}


/** this define will switch between direct call of mbspex lib or external shell call of gosipcmd*
 * note: we need to call "make nombspex" if we disable this define here!
 * note2: this define is enabled from top Makefile when building regular "make all"*/
//#define USE_MBSPEX_LIB 1

#ifdef USE_MBSPEX_LIB
extern "C"
{
#include "mbspex/libmbspex.h"
}
#else
// provide dummy structure although never filled by driver:
#define PEX_SFP_NUMBER 4
struct pex_sfp_links{
    int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
};

/** also need to declare this here without library include!*/
 void printm (char *, ...);


#endif

/** the (A)BC for all frontend setup structures */
class GosipSetup
{

  public:

  GosipSetup(){;}
  virtual ~GosipSetup(){;}
  virtual void Dump(){printm("Empty status structure - Please implement a new frontend first");}

};



class GosipGui: public QMainWindow, public Ui::GosipMainWindow
{
  Q_OBJECT

public:
  GosipGui (QWidget* parent = 0);
  virtual ~GosipGui ();

  int GetNumberBase(){return fNumberBase;}


  bool IsAutoApply(){return checkBox_AA->isChecked();}


  void AppendTextWindow (const QString& text);

  void AppendTextWindow (const char* txt)
         {
           QString buf (txt);
           AppendTextWindow (buf);
         }

  void FlushTextWindow();


  /** singleton pointer to forward mbspex lib output, also useful without mbspex lib:*/
static GosipGui* fInstance;


	bool IsSlotGuard(){
		return fSlotGuard;
	}

	void SetSlotGuard(bool on){
		fSlotGuard=on;
	}

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
  std::vector<GosipSetup*> fSetup[4];

  // TODO: solve inheritance problems here!!!


  /** contains currently configured slaves at the chains.*/
  struct pex_sfp_links fSFPChains;


  /** text debug mode*/
  bool fDebug;

  /** save configuration file instead of setting device values*/
  bool fSaveConfig;

  /** this flag protects some slots during broadcast write mode*/
  bool fBroadcasting;


  /** protect mutually dependent slots by this.*/
   bool fSlotGuard;

   /** indicate full screen mode*/
   bool fFullScreen;


   /** may change display of subwindows (tabs=true, separate windows=false)mode*/
   bool fMdiTabViewMode;

  /** base for number display (10 or 16)*/
  int fNumberBase;

  /** index of sfp channel,   -1 for broadcast */
  int fSFP;
  /** index of slave device , -1 for broadcast*/
  int fSlave;

  /** remember sfp channel to recover after broadcast*/
  int fSFPSave;

  /** remember slave channel to recover after broadcast*/
  int fSlaveSave;




#ifdef USE_MBSPEX_LIB



  /** file descriptor on mbspex device*/
  int fPexFD;

  /** speed down mbspex io with this function from Nik*/
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

 /** update initilized chain display and slave limit*/
  void RefreshChains();


  /** copy sfp and slave from gui to variables*/
  void EvaluateSlave ();

  /** retrieve slave configuration from driver*/
  void GetSFPChainSetup();


  /** Read from address from sfp and slave, returns value*/
  int ReadGosip (int sfp, int slave, int address);

  /** Write value to address from sfp and slave*/
  int WriteGosip (int sfp, int slave, int address, int value);

  /** Save value to currently open *.gos configuration file*/
  int SaveGosip (int sfp, int slave, int address, int value);

  /** execute (gosip) command in shell. Return value is output of command*/
  QString ExecuteGosipCmd (QString& command,  int timeout=5000);

 

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
  /** Check if broadast mode is not set. If set, returns false and prints error message if verbose is true*/
  bool AssertNoBroadcast (bool verbose=true);


  /** Check if chain for given sfp and slave index is configured correctly*/
  bool AssertChainConfigured (bool verbose=true);



  /**
   * The following methods are the interface to the implementation:
   */

  /** update gui display from status structure*/
  virtual void RefreshView ();

  /** put gui values into status structure*/
   virtual void EvaluateView ();

  /** get register contents from hardware to status structure*/
   virtual void GetRegisters ();

   /** set register contents from status structure to hardware*/
   virtual void SetRegisters ();

   /** evaluate gui display and put it immediately to hardware.
    * default beaviour is using just  EvaluateView() and SetRegisters (),
    * this may be overwritten*/
   virtual void ApplyGUISettings();


   /** get registers and write them to config file*/
   virtual void SaveRegisters();

   /** generic reset/init call for sfp slave*/
   virtual void ResetSlave();

   /** generic reset/init call for dumpint sfp slave registers*/
   virtual void DumpSlave();

   /** save configuration to a file*/
   virtual void SaveConfig();


   /** apply configuration from a file to the hardware.
    * Subclass implementation may call this functions with gosipwait  time in microseconds
    * for crucial frontend timing*/
   virtual void ApplyFileConfig(int gosipwait=0);


   /* Factory method for the frontend setup structures.
    * To be re-implemented in subclass*/
   virtual GosipSetup* CreateSetup()
     {
       return new GosipSetup();
     }



   virtual void closeEvent( QCloseEvent * ce );



   void storePanelGeometry(QWidget* w, const QString& kind);

   QSize lastPanelSize(const QString& kind, int dfltwidth=450, int dfltheight=250);

   QPoint lastPanelPos(const QString& kind);


public slots:
  virtual void ShowBtn_clicked();
  virtual void ApplyBtn_clicked ();
  virtual void InitChainBtn_clicked ();
  virtual void ResetBoardBtn_clicked ();
  virtual void ResetSlaveBtn_clicked ();
  virtual void BroadcastBtn_clicked (bool checked);
  virtual void DumpBtn_clicked ();
  virtual void ClearOutputBtn_clicked ();
  virtual void ConfigBtn_clicked ();
  virtual void SaveConfigBtn_clicked ();
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void Slave_changed(int val);
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

#endif
