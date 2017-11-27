#ifndef SEQUENCERCOMMANDS_H
#define SEQUENCERCOMMANDS_H

//#include "ApfelDefines.h"

/** command action id to be put into the test sequencer list*/

enum SequencerAction
{
  SEQ_NONE,
  SEQ_INIT,
  SEQ_FINALIZE,
  SEQ_GAIN_1,
  SEQ_GAIN_16,
  SEQ_GAIN_32,
  SEQ_AUTOCALIB,
  SEQ_NOISESAMPLE,
  SEQ_BASELINE,
  SEQ_CURVE,
  SEQ_ADDRESS_SCAN,
  SEQ_CURRENT_MEASUEREMENT
};

/** command token to be put into the test sequencer list*/
class SequencerCommand
{
private:

  /** action command id to be executed*/
  SequencerAction fAction;

  /** optional channel (or chip) number. used to explictely execute loops with low command granularity in timer*/
  int fChannel;

public:

  SequencerCommand (SequencerAction actionid, int channel = 0) :
      fAction (actionid), fChannel (channel)
  {
    ;
  }

  SequencerAction GetAction ()
  {
    return fAction;
  }
  int GetChannel ()
  {
    return fChannel;
  }
};

#endif
