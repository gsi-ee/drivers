#include "BasicGui.h"
#include "BasicSetup.h"
#include "BasicSubWidget.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <stdlib.h>
#include <errno.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QMdiSubWindow>

#include <sstream>




#include <stdarg.h>





void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
//printf ("%s", c_str);
  if (gapg::BasicGui::fInstance)
  {
    gapg::BasicGui::fInstance->AppendTextWindow (c_str);
    gapg::BasicGui::fInstance->FlushTextWindow ();
  }
  else
  {
    printf ("%s \n", c_str);
  }
  va_end(args);
}

namespace gapg{

#ifdef USE_GALAPAGOS_LIB
/** this one is used to speed down direct galapagos io:*/
void BasicGui::I2c_sleep ()
{
  //usleep(300);

  usleep(900); // JAM2016 need to increase wait time since some problems with adc read?
}

#endif



// this we need to implement for output of galapagos library, but also useful to format output without it:
BasicGui* BasicGui::fInstance = 0;

bool BasicGui::fInAction=false;
bool BasicGui::fSlotGuard=false;

// *********************************************************

/*
 *  Constructs a BasicGui which is a child of 'parent', with the
 *  name 'name'.'
 */
BasicGui::BasicGui (QWidget* parent) :
    QMainWindow (parent), fSettings(0), fWinMapper(0), fSetup(0), fDebug (false), fSaveConfig(false),
    fFullScreen(false), fMdiTabViewMode(true),
      fConfigFile(NULL)
{

 //Q_INIT_RESOURCE(galapicons);


  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to galapcmd from parent process
#endif


  fImplementationName="Galapagos";
  fVersionString="Welcome to GalapaGUI!\n\t v0.1 of 06-Aug-2019 by JAM (j.adamczewski@gsi.de)";


  fNumberBase=10;



  TextOutput->setCenterOnScroll (false);
  ClearOutputBtn_clicked ();





  QObject::connect(actionConfigure, SIGNAL(triggered()), this, SLOT(ConfigBtn_clicked ()));
  QObject::connect(actionSave, SIGNAL(triggered()), this, SLOT(SaveConfigBtn_clicked ()));

  QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
  QObject::connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));

//  QObject::connect(actionCascade, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));
//  QObject::connect(actionTile, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));


  QObject::connect(actionSaveSettings, SIGNAL(triggered()), this, SLOT(WriteSettings()));

  connect(menuWindows, SIGNAL(aboutToShow()), this, SLOT(windowsMenuAboutToShow()));
  // need to create menu item with F11
  windowsMenuAboutToShow();


  QAction* refreshaction=ControlToolBar->addAction( QIcon( ":/icons/refresh.png" ), "Show current hardware status (Refresh GUI) - F5",
                        this, SLOT(ShowBtn_clicked ()));
  refreshaction->setShortcut(Qt::Key_F5);

  QAction* applyaction=ControlToolBar->addAction(  QIcon( ":/icons/left.png" ), "Apply GUI setup to hardware - F12",
                          this, SLOT(ApplyBtn_clicked ()));
  applyaction->setShortcut(Qt::Key_F12);

  ControlToolBar->addSeparator();
  ControlToolBar->addAction(  QIcon( ":/icons/fileopen.png" ), "Load configuration from file into GUI",
                           this, SLOT(ConfigBtn_clicked ()));

  ControlToolBar->addAction(  QIcon( ":/icons/filesave.png" ), "Save current hardware configuration into setup file",
                             this, SLOT(SaveConfigBtn_clicked ()));

  ControlToolBar->addSeparator();
  ControlToolBar->addAction(  QIcon( ":/icons/analysiswin.png" ), "Dump main register contents of selected device to output window",
                              this, SLOT(DumpBtn_clicked ()));

  ControlToolBar->addAction(  QIcon( ":/icons/clear.png" ), "Clear output text window",
                               this, SLOT(ClearOutputBtn_clicked ()));


  SFPToolBar->addAction( QIcon( ":/icons/control.png" ), "Initialize GALAPAGOS device test",
                          this, SLOT(ResetSlaveBtn_clicked ()));

  SFPToolBar->addAction( QIcon( ":/icons/killanal.png" ), "Reset basic engine on GALAPAGOS board",
                            this, SLOT(ResetBoardBtn_clicked ()));


QObject::connect(DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
QObject::connect(HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));


// JAM2017: some more signals for the autoapply feature:



#ifdef USE_GALAPAGOS_LIB
// open handle to driver file:
  fPexFD = galapagos_open (0);    // we restrict to board number 0 here
  if (fPexFD < 0)
  {
    printf ("GALAPAGUI: ERROR opening /dev/galapagos%d - please check with 'lsmod | grep galapagos' if kernel module is loaded!\n", 0);
    exit (1);
  }
#endif
  fInstance = this;

  // NOTE: this MUST be done in subclass constructor, otherwise factory method for setup structure will fail!
  //    (not tested, but to be expected so...)
  //BuildSetup(); // ensure that we have a status structure before we begin clicking...
  //////////// end NOTE

   //show();
}

BasicGui::~BasicGui ()
{
#ifdef USE_GALAPAGOS_LIB
  galapagos_close (fPexFD);
#endif

if(fSettings) delete fSettings;

}


void BasicGui::AddSubWindow(gapg::BasicSubWidget* widget)
{
  Qt::WindowFlags wflags= Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint;
  QMdiSubWindow* sub=mdiArea->addSubWindow(widget,wflags);
  sub->setAttribute(Qt::WA_DeleteOnClose, false);
  sub->setOption(QMdiSubWindow::RubberBandResize);
  sub->setOption(QMdiSubWindow::RubberBandMove); // JAM required for qt5 performance

  fSubWidgets.push_back(widget);
  widget->SetBasicParent(this);
}


void BasicGui::ConnectSlots()
{
  for (std::vector<gapg::BasicSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
    {
        (*it)->ConnectSlots();
    }
}



void BasicGui::ShowBtn_clicked ()
{

  //std::cout << "BasicGui::ShowBtn_clicked()"<< std::endl;
  BuildSetup();

  GAPG_LOCK_SLOT
  GetRegisters ();
  RefreshView ();
  GAPG_UNLOCK_SLOT

}

void BasicGui::ApplyBtn_clicked ()
{
//std::cout << "BasicGui::ApplyBtn_clicked()"<< std::endl;
  if (!checkBox_AA->isChecked ())
  {
    QString message = QString("Really apply GUI Settings  to %1?").arg(fImplementationName);
    if (QMessageBox::question (this, fImplementationName, message, QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }

  GAPG_ACTION(ApplyGUISettings());
}










void BasicGui::ResetBoardBtn_clicked ()
{
//std::cout << "BasicGui::ResetBoardBtn_clicked"<< std::endl;
if (QMessageBox::question (this, fImplementationName, "Really Reset galap on pex board?", QMessageBox::Yes | QMessageBox::No,
    QMessageBox::Yes) != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}


#ifdef USE_GALAPAGOS_LIB
  galapagos_reset (fPexFD);
  AppendTextWindow ("Reset PEX board with galapagos_reset()");

#else

char buffer[1024];
snprintf (buffer, 1024, "galapcmd -z");
QString com (buffer);
QString result = ExecuteGAPGCmd (com);
AppendTextWindow (result);

#endif

}

void BasicGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  snprintf (buffer, 1024, "Really reset %s device ?", fImplementationName.toLatin1 ().constData () );
  if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
  GAPG_ACTION(ResetSlave());
}

void BasicGui::ResetSlave ()
{

 printm("ResetSlave() not yet implemented, please design a subclass of BasicGui!\n");
}

void BasicGui::EvaluateView()
{
 for (std::vector<gapg::BasicSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
 {
     (*it)->EvaluateView();
 }
}

void BasicGui::SetRegisters ()
{

  printm("SetRegisters() not yet implemented, please design a subclass of BasicGui!\n");
}

void BasicGui::GetRegisters ()
{
  printm("GetRegisters()  not yet implemented, please design a subclass of BasicGui!\n");
}



void BasicGui::ApplyGUISettings()
{
     //std::cout << "BasicGui::ApplyGUISettings()"<< std::endl;
    // default behaviour, may be overwritten
    EvaluateView(); // from gui to memory
    SetRegisters(); // from memory to device
}




void BasicGui::DumpBtn_clicked ()
{
//std::cout << "BasicGui::DumpBtn_clicked"<< std::endl;
// dump register contents from galapcmd into TextOutput (QPlainText)
AppendTextWindow ("--- Register Dump ---:");
GAPG_ACTION(Dump());
}


void BasicGui::Dump()
{
  // most generic form, just uses dump function of setup structure
  // for detailed data fifo dump etc. please overwrite this method!
  GetRegisters();
  printm("Dump setup...");
  fSetup->Dump();

}


void BasicGui::ClearOutputBtn_clicked ()
{
//std::cout << "BasicGui::ClearOutputBtn_clicked()"<< std::endl;
TextOutput->clear ();
TextOutput->setPlainText (fVersionString);

}

void BasicGui::ConfigBtn_clicked ()
{
//std::cout << "BasicGui::ConfigBtn_clicked" << std::endl;
  ApplyFileConfig();
}


void BasicGui::ApplyFileConfig(int galapwait)
{
  // most generic form
  // TODO: probably put here argument to set galap waiting time before executing the script
  QFileDialog fd (this, "Select GAPG configuration file", ".", "galapcmd file (*.gos)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  if (galapwait > 0)
  {
    QString tcom=QString("setGAPGwait.sh %1").arg(galapwait);
    QString tresult = ExecuteGAPGCmd (tcom, 10000);
    AppendTextWindow (tresult);
  }

  QString fileName = flst[0];
  if (!fileName.endsWith (".gal"))
    fileName.append (".gal");
  char buffer[1024];
  snprintf (buffer, 1024, "galapcmd -x -c %s ", fileName.toLatin1 ().constData ());
  QString com (buffer);
  QString result = ExecuteGAPGCmd (com);
  AppendTextWindow (result);
  if(galapwait>0)
   {
    QString zcom="setGAPGwait.sh 0";
    QString zresult=ExecuteGAPGCmd (zcom, 10000);
    AppendTextWindow (zresult);
   }

}



void BasicGui::DebugBox_changed (int on)
{
 GAPG_LOCK_SLOT
//std::cout << "DebugBox_changed to "<< on << std::endl;
fDebug = on;
GAPG_UNLOCK_SLOT
}

void BasicGui::HexBox_changed(int on)
{
  GAPG_LOCK_SLOT
  fNumberBase= (on ? 16 :10);
  //std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;
  RefreshView ();
  GAPG_UNLOCK_SLOT
}





void BasicGui::RefreshStatus ()
{
  QString text;
  QString statustext;
//   statustext.append ("SFP ");
//   statustext.append (text.setNum (fSFP));
//   statustext.append (" DEV ");
//   statustext.append (text.setNum (fSlave));
   statustext.append (" - Last refresh:");
   statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
   ShowStatusMessage(statustext);
//   statusBar()->showMessage(statustext);
}



void BasicGui::RefreshView ()
{
// display setup structure to gui:

  // treat all registered subwindows:
  for (std::vector<gapg::BasicSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
      {
          (*it)->RefreshView();
      }

RefreshStatus();
}





void BasicGui::SaveConfigBtn_clicked ()
{
  //std::cout << "BasicGui::SaveConfigBtn_clicked()"<< std::endl;
  SaveConfig();
}




void BasicGui::SaveConfig()
{
  // default: write a galapcmd script file
  // this one may be reimplemented in subclass if other file formats shall be supported.
  static char buffer[1024];
  QString gos_filter ("galapcmd file (*.gal)");
  //QString dmp_filter ("data dump file (*.dmp)");
  QStringList filters;
  filters << gos_filter;// << dmp_filter;

  QFileDialog fd (this, "Write GAPG configuration file");

  fd.setNameFilters (filters);
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString fileName = flst[0];

  // complete suffix if user did not
   if (fd.selectedNameFilter () == gos_filter)
  {
    if (!fileName.endsWith (".gal"))
      fileName.append (".gal");
  }
  else
  {
    std::cout << "BasicGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

  // open file
  if (OpenConfigFile (fileName) != 0)
    return;

  if (fileName.endsWith (".gal"))
  {
    WriteConfigFile (QString ("# Format *.gal\n"));
    WriteConfigFile (QString ("# usage: galapcmd -x -c file.gal \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("# address value\n"));


    GAPG_ACTION(SaveRegisters()); // refresh actual setup from hardware and write it to open file
  }
  else
  {
    std::cout << "BasicGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

  // close file
  CloseConfigFile ();
  snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
  AppendTextWindow (buffer);
}





int BasicGui::OpenConfigFile (const QString& fname)
{
  fConfigFile = fopen (fname.toLatin1 ().constData (), "w");
  if (fConfigFile == NULL)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error opening Configuration File '%s': %s\n", fname.toLatin1 ().constData (),
        strerror (errno));
    AppendTextWindow (buffer);
    return -1;
  }
  QString timestring = QDateTime::currentDateTime ().toString ("ddd dd.MM.yyyy hh:mm:ss");
  WriteConfigFile (QString ("# Galapagos configuration file saved on ") + timestring + QString ("\n"));
  return 0;
}

int BasicGui::CloseConfigFile ()
{
  int rev = 0;
  if (fConfigFile == NULL)
    return 0;
  if (fclose (fConfigFile) != 0)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error closing Configuration File! (%s)\n", strerror (errno));
    AppendTextWindow (buffer);
    rev = -1;
  }
  fConfigFile = NULL;    // must not use handle again even if close fails
  return rev;
}

int BasicGui::WriteConfigFile (const QString& text)
{
  if (fConfigFile == NULL)
    return -1;
  if (fprintf (fConfigFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}




void BasicGui::SaveRegisters ()
{
  // default implementation. might be overwritten by frontend subclass -
  // this may be called in explicit broadcast mode, so it is independent of the view on gui
  GetRegisters(); // refresh actual setup from hardware
  fSaveConfig = true;    // switch to file output mode
  SetRegisters();    // register settings are written to file
  fSaveConfig = false;
}




int BasicGui::ReadGAPG (int address)
{
int value = -1;
#ifdef USE_GALAPAGOS_LIB
  int rev = 0;
  long int dat = 0;
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = galapagos_register_rd (fPexFD, GAPG_REGISTERS_BAR, address, &dat);
  I2c_sleep ();
  value = dat;
  if (fDebug)
  {
    char buffer[1024];
    if (rev == 0)
    {
      snprintf (buffer, 1024, "galapagos_register_rd(0x%x) -> 0x%x", address, value);
    }
    else
    {
      snprintf (buffer, 1024, "ERROR %d from galapagos_register_rd(0x%x)", rev, address);
    }
    QString msg (buffer);
    AppendTextWindow (msg);

  }
  //QApplication::restoreOverrideCursor ();
#else
char buffer[1024];
snprintf (buffer, 1024, "galapcmd -r -- 0x%x",address);
QString com (buffer);
QString result = ExecuteGAPGCmd (com);
if (result != "ERROR")
{
  DebugTextWindow (result);
  value = result.toInt (0, 0);
}
else
{

  value = -1;
}
#endif

return value;
}

int BasicGui::WriteGAPG (int address, int value)
{
  int rev = 0;
  if (fSaveConfig)
      return SaveGAPG (address, value);

#ifdef USE_GALAPAGOS_LIB
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = galapagos_register_wr (fPexFD, GAPG_REGISTERS_BAR,  address, value);
  I2c_sleep ();
  if (fDebug)
  {
    char buffer[1024];
    snprintf (buffer, 1024, "galapagos_register_wr(0x%x 0x%x)", address, value);
    QString msg (buffer);
    AppendTextWindow (msg);
  }
  //QApplication::restoreOverrideCursor ();
#else



char buffer[1024];
snprintf (buffer, 1024, "galapcmd -w -- 0x%x 0x%x", address, value);
QString com (buffer);
QString result = ExecuteGAPGCmd (com);
if (result == "ERROR")
  rev = -1;

#endif
return rev;
}

int BasicGui::SaveGAPG (int address, int value)
{
//std::cout << "# SaveBasic" << std::endl;
  static char buffer[1024] = { };
  snprintf (buffer, 1024, "%x %x \n", address, value);
  QString line (buffer);
  return (WriteConfigFile (line));
}



QString BasicGui::ExecuteGAPGCmd (QString& com, int timeout)
{
// interface to shell galapcmd
// TODO optionally some remote call via ssh for Go4 gui?
QString result;
QProcess proc;
DebugTextWindow (com);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
proc.setProcessEnvironment (fEnv);
#endif
proc.setReadChannel (QProcess::StandardOutput);
QApplication::setOverrideCursor( Qt::WaitCursor );

proc.start (com);
// if(proc.waitForReadyRead (1000)) // will give termination warnings after leaving this function
if (proc.waitForFinished (timeout))    // after process is finished we can still read stdio buffer
{
  // read back stdout of proc here
  result = proc.readAll ();
}
else
{

 std::stringstream buf;
    buf << "! Warning: ExecuteGAPGCmd not finished after " << timeout / 1000 << " s timeout !!!" << std::endl;
    std::cout << " BasicGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
    result = "ERROR";
}
QApplication::restoreOverrideCursor();
return result;
}

void BasicGui::AppendTextWindow (const QString& text)
{
TextOutput->appendPlainText (text);
TextOutput->update ();
}



void BasicGui::FlushTextWindow ()
{
  TextOutput->repaint ();
}

void  BasicGui::ShowStatusMessage (const QString& text)
{
  statusBar()->showMessage(text);
}



void BasicGui::windowsMenuAboutToShow()
{
    //std::cout<< "windowsMenuAboutToShow..." << std::endl;
    menuWindows->clear();

    bool on = ! mdiArea->subWindowList().isEmpty();


    menuWindows->addAction("Cascade", mdiArea, SLOT(cascadeSubWindows()), Qt::Key_F7)->setEnabled(on);

    menuWindows->addAction("Tile", mdiArea, SLOT(tileSubWindows()), Qt::Key_F8)->setEnabled(on);
    menuWindows->addAction("Minimize all", this, SLOT(MinAllWindows()),Qt::Key_F9)->setEnabled(on);
    menuWindows->addAction((fMdiTabViewMode ? "Tabbed subwindows" : "Separate subwindows"), this, SLOT(ToggleSubwindowModeSlot()), Qt::Key_F6);


    menuWindows->addAction((fFullScreen ? "Normal window" : "Full screen"), this, SLOT(ToggleFullScreenSlot()), Qt::Key_F11);


    menuWindows->addSeparator();


    delete fWinMapper;
    fWinMapper = new QSignalMapper(this);
    connect(fWinMapper, SIGNAL(mapped(int)), this, SLOT(windowsMenuActivated(int)));

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    for (int i=0; i< windows.count(); i++ ) {
       QAction* act = new QAction(windows.at(i)->widget()->windowTitle(), fWinMapper);
       act->setCheckable(true);
       act->setChecked(mdiArea->activeSubWindow() == windows.at(i));

       menuWindows->addAction(act);

       connect(act, SIGNAL(triggered()), fWinMapper, SLOT(map()) );
       fWinMapper->setMapping(act, i);
    }
}

void BasicGui::windowsMenuActivated( int id )
{
 // std::cout<< "windowsMenuActivated..." << std::endl;
   QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
   if ((id>=0) && (id<windows.count())) {
      windows.at(id)->widget()->showNormal();
      windows.at(id)->widget()->setFocus();
   }
}

void BasicGui::MinAllWindows()
{
   QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
   for ( int i = 0; i < windows.count(); i++ )
       windows.at(i)->widget()->showMinimized();
}



void BasicGui::ToggleFullScreenSlot()
{
   if (fFullScreen) showNormal();
               else showFullScreen();
   fFullScreen = !fFullScreen;
}

void BasicGui::ToggleSubwindowModeSlot()
{
   if (fMdiTabViewMode){
     mdiArea->setViewMode(QMdiArea::TabbedView);
   }
   else{
     mdiArea->setViewMode(QMdiArea::SubWindowView);
   }
   fMdiTabViewMode = !fMdiTabViewMode;
}



void BasicGui::about()
{

   QMessageBox AboutWindow(fImplementationName, fVersionString, QMessageBox::NoIcon,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton ,this);
   AboutWindow.setIconPixmap(QPixmap( ":/icons/galapagos.png"));
   AboutWindow.setTextFormat(Qt::RichText);
   AboutWindow.exec();
}


// stolen from Go4 to store/restore the subwindow sizes:
void BasicGui::storePanelGeometry(QWidget* w, const QString& kind)
{
  if(fSettings)
    {
     fSettings->setValue(QString("/") + kind + QString("/Width"), w->width() );
     fSettings->setValue(QString("/") + kind + QString("/Height"), w->height() );
     fSettings->setValue(QString("/") + kind + QString("/X"), w->pos().x());
     fSettings->setValue(QString("/") + kind + QString("/Y"), w->pos().y());
    // std::cout<<"storePanelSize for "<<kind.toStdString().c_str() <<" saves width:"<<w->width()<<", height:"<< w->height();
     //std::cout<<" x:"<< w->pos().x()<<", y:"<<w->pos().y()<< std::endl;
    }
}

QSize BasicGui::lastPanelSize(const QString& kind, int dfltwidth, int dfltheight)
{
    if(fSettings)
    {
      QSize rect(fSettings->value(QString("/") + kind + QString("/Width"), dfltwidth).toInt(),
              fSettings->value(QString("/") + kind + QString("/Height"), dfltheight).toInt());


     //std::cout<<"lastPanelSize for "<<kind.toStdString().c_str()<<" gets width:"<<rect.width()<<", height:"<< rect.height() << std::endl;

     return rect;
    }
    else
      return QSize( dfltwidth, dfltheight);
}

QPoint BasicGui::lastPanelPos(const QString& kind)
{
    if(fSettings)
    {
      QPoint pt(fSettings->value(QString("/") + kind + QString("/X")).toInt(),
              fSettings->value(QString("/") + kind + QString("/Y")).toInt());


    // std::cout<<"lastPanelPosition for "<<kind.toStdString().c_str()<<" gets x:"<<pt.x()<<", y:"<< pt.y() << std::endl;

     return pt;
    }
    else
      return QPoint( 0, 0);
}



void BasicGui::ReadSettings()
{
  //std::cout<< "ReadSettings..." << std::endl;
  if(fSettings)
    {
      // have to lock the slots of the hexbox to avoid that we trigger a refresh view before everything is ready!
    GAPG_LOCK_SLOT
      bool autoapp=fSettings->value("/ModeControl/isAutoApply", false).toBool();
      checkBox_AA->setChecked(autoapp);

      fNumberBase=fSettings->value("/ModeControl/numberBase", 10).toInt();
      HexBox->setChecked(fNumberBase>10 ? true : false);
      fDebug= fSettings->value("/ModeControl/isVerbose", false).toBool();
      DebugBox->setChecked(fDebug);

      fMdiTabViewMode=!(fSettings->value("/Mdi/isTabbedMode",false).toBool());
      ToggleSubwindowModeSlot(); // set the desired mode and invert the flag again for correct menu display!



      restoreState(fSettings->value("/MainWindow/State").toByteArray());
      restoreGeometry(fSettings->value("/MainWindow/Geometry").toByteArray());

// JAM2019: the following will not work to store window sizes of mdi subwindows
// the settings file do not get byte array from subwindows, but only "false" property
// => not intended to use qsettings for mdi subwindows directly
//      mdiArea->restoreGeometry(fSettings->value("/MdiArea/Geometry").toByteArray());
//      QList<QMdiSubWindow*> subwinlist= mdiArea->subWindowList();
//         for (int i = 0; i < subwinlist.size(); ++i)
//         {
//           QMdiSubWindow* win= subwinlist.at(i);
//           win->restoreGeometry(fSettings->value( QString ("/MdiArea/SubWindow%1").arg (i)).toByteArray());
//           std::cout <<"ReadSettings restored subwindow "<< i << std::endl;
//         }
//////////// end comment on failed try,

      QList<QMdiSubWindow*> subwinlist= mdiArea->subWindowList();
      for (int i = 0; i < subwinlist.size(); ++i)
      {
        QMdiSubWindow* win= subwinlist.at(i);
        win->resize(lastPanelSize(QString ("SubWindow%1").arg (i)));
        win->move(lastPanelPos(QString ("SubWindow%1").arg (i)));
        //std::cout <<"ReadSettings restored subwindow panel geometry "<< i << std::endl;
      }
      //std::cout<< "ReadSettings gets numberbase:"<<fNumberBase<<", fDebug:"<<fDebug<<", autoapply:"<<autoapp << std::endl;

      // here we treat individual settings of each subwindow:
      //theSetup_GET_FOR_CLASS(GalapagosSetup);
      for (std::vector<gapg::BasicSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
             {
                 (*it)->ReadSettings(fSettings);
             }


      GAPG_UNLOCK_SLOT
    }
}

void BasicGui::WriteSettings()
{
  //std::cout<< "WriteSettings..." << std::endl;
  if(fSettings)
  {
    fSettings->setValue("/Mdi/isTabbedMode", fMdiTabViewMode);

    fSettings->setValue("/ModeControl/isVerbose", fDebug);
    fSettings->setValue("/ModeControl/numberBase", fNumberBase);
    fSettings->setValue("/ModeControl/isAutoApply", IsAutoApply());

    fSettings->setValue("/MainWindow/State", saveState());
    fSettings->setValue("/MainWindow/Geometry", saveGeometry());

// scan geometry of subwindows in mdiarea:
    QList<QMdiSubWindow*> subwinlist= mdiArea->subWindowList();
    for (int i = 0; i < subwinlist.size(); ++i)
    {
      QMdiSubWindow* win= subwinlist.at(i);
      storePanelGeometry(win,QString ("SubWindow%1").arg (i)); // stolen from go4
     // std::cout <<"WriteSettings stored subwindow panel size "<< i << std::endl;
    }

    //theSetup_GET_FOR_CLASS(GalapagosSetup);
    if(fSettings)
        {
        for (std::vector<gapg::BasicSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
           {
               (*it)->ReadSettings(fSettings);
           }
        }


  }
}


BasicSetup* BasicGui::CreateSetup()
    {
      //std::cout <<"BasicGui:: CreateSetup" <<std::endl;
      return new BasicSetup();
    }


void BasicGui::closeEvent( QCloseEvent* ce)
{

// new for Qt4:
   if(QMessageBox::question( this, fImplementationName, "Really Exit GalapaGUI window?",
         QMessageBox::Yes | QMessageBox::No ,
         QMessageBox::Yes) != QMessageBox::Yes ) {
            //std::cout <<"QMessageBox does not return yes! "<< std::endl;
            ce->ignore();
            return;
      }
   ce->accept();
}

} // namespace
