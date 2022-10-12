#ifndef AWAGSTEST_H
#define AWAGSTEST_H

class AwagsGui;
class BoardSetup;

#include "AwagsDefines.h"
#include "AwagsTestResults.h"
#include "SequencerCommand.h"

#include <QString>

#include <queue>
#include <map>

/** class with all functionality required to perform benchmark tests.
 * This is an aggregate of the main gui*/
class AwagsTest
{
private:

  /** back reference to the GUI */
  AwagsGui* fOwner;

  /** setup structure of the board under test*/
  BoardSetup* fCurrentSetup;


  std::queue <SequencerCommand> fTestSequence;

   /** Initial number of sequencer commands*/
   int fTestLength;


   /** remember gain that was set in previous sequencer commands*/
   int fCurrentGain;



#ifdef AWAGS_NOSTDMAP
  AwagsTestResults fReferenceValues_1;
  AwagsTestResults fReferenceValues_16;
  AwagsTestResults fReferenceValues_32;

#else
  /**

   /** This structure just contains sollwerte for comparison. mapped to gain*/
   std::map<int, AwagsTestResults> fReferenceValues;

#endif
   bool fMultiPulserMode;

public:

  AwagsTest();

  virtual ~AwagsTest(){;}

  void SetOwner(AwagsGui* parent){fOwner=parent;}

  void SetSetup(BoardSetup* set){fCurrentSetup=set;}


  void SetMultiPulserMode(bool on){fMultiPulserMode=on;}
  bool IsMultiPulserMode(){return fMultiPulserMode;}


  /** access to reference values for gain 1,16, or 32*/
  AwagsTestResults& GetReferenceValues(int gain);

  /** set reference values for test results. Either from memory or database*/
  void InitReferenceValues(bool invertedslope=true);

  /** get reference values from file*/
  void LoadReferenceValues(const QString& file);

  /** put a command for the test sequence to the list*/
   void AddSequencerCommand(SequencerCommand com);

   /** get next command from sequencer list and remove it*/
   SequencerCommand NextSequencerCommand();

   /** Mark Sequencer list as complete*/
   void FinalizeSequencerList();

   /** clear all entries for sequencer*/
   void ResetSequencerList();

   /** returns percentage of sequencer commands already done.*/
   int GetSequencerProgress();

   /** Called by timer. Executes next step of benchmark actions in the sequencer list.*/
   bool ProcessBenchmark();


};


#endif
