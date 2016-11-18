#ifndef APFELTEST_H
#define APFELTEST_H

class ApfelGui;
class BoardSetup;

#include "ApfelDefines.h"
#include "ApfelTestResults.h"
#include "SequencerCommand.h"

#include <QString>

#include <queue>
#include <map>

/** class with all functionality required to perform benchmark tests.
 * This is an aggregate of the main gui*/
class ApfelTest
{
private:

  /** back reference to the GUI */
  ApfelGui* fOwner;

  /** setup structure of the board under test*/
  BoardSetup* fCurrentSetup;


  std::queue <SequencerCommand> fTestSequence;

   /** Initial number of sequencer commands*/
   int fTestLength;


   /** remember gain that was set in previous sequencer commands*/
   int fCurrentGain;


   /** This structure just contains sollwerte for comparison. mapped to gain*/
   std::map<int, ApfelTestResults> fReferenceValues;




public:

  ApfelTest();

  void SetOwner(ApfelGui* parent){fOwner=parent;}

  void SetSetup(BoardSetup* set){fCurrentSetup=set;}


  /** access to reference values for gain 1,16, or 32*/
  ApfelTestResults& GetReferenceValues(int gain);

  /** set reference values for test results. Either from memory or database*/
  void InitReferenceValues();

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
