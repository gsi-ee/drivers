#ifndef GAPGSETUP_H
#define GAPGSETUP_H

#include "BasicGui.h"
#include "GalapagosDefines.h"
#include "GalapagosObjects.h"
#include <stdint.h>

/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class GalapagosSetup: public BasicSetup
{
public:

  /** True if pattern generator is running*/
  bool fGeneratorActive;

  /* mockup of a future channel control/status register
   * each bit may set corresponding channel active*/
  uint32_t fChannelControl_0;

  /* mockup of a future channel control/status register
   * each bit may set corresponding channel active*/
  uint32_t fChannelControl_1;

  /** unique sequence id for each cheannel*/
  uint32_t fChannelSequenceID[GAPG_CHANNELS];

  /** unique pattern  id for each cheannel*/
  uint32_t fChannelPatternID[GAPG_CHANNELS];

  /* list of known pattern sequences as visible in the sequence editor*/
  std::vector<GalapagosSequence> fKnownSequences;

  std::vector<GalapagosPattern> fKnownPatterns;

  /* all initialization here:*/
  GalapagosSetup ();

  bool IsGeneratorActive ()
  {
    return fGeneratorActive;
  }

  void SetGeneratorActive (bool on)
  {
    fGeneratorActive = on;
  }

  uint64_t GetChannelControl ();

  uint32_t GetChannelControl_0 ()
  {
    return fChannelControl_0;
  }

  void SetChannelControl_0 (uint32_t val)
  {
    fChannelControl_0 = val;
  }

  uint32_t GetChannelControl_1 ()
  {
    return fChannelControl_1;
  }
  void SetChannelControl_1 (uint32_t val)
  {
    fChannelControl_1 = val;
  }

  void SetChannelControl (uint64_t val);

  void SetChannelEnabled (uint8_t ch, bool on);

  bool IsChannelEnabled (uint8_t ch);

  void ClearSequences ();

  GalapagosSequence& AddSequence (GalapagosSequence& seq);

  /* Access a known sequence by unique id number. May be redundant if we rely on name*/
  GalapagosSequence* GetSequence (uint32_t id);

  /* Access a known sequence by unique name*/
  GalapagosSequence* GetSequence (const char* name);

  size_t NumKnownSequences ();

  /** access to list of known sequences by index */
  GalapagosSequence* GetKnownSequence (size_t ix);

  /** remove sequence from list by index. also clean up all references in channels*/
  void RemoveKnownSequence (size_t ix);

  bool SetChannelSequence (int chan, uint32_t id);

  bool SetChannelSequence (int chan, const char* name);

  GalapagosSequence* GetChannelSequence (int chan);

  uint32_t GetChannelSequenceID (int chan);
/////////////////////////////77

  GalapagosPattern& AddPattern (GalapagosPattern& pat);

  /** Access a known sequence by unique id number. May be redundant if we rely on name*/
  GalapagosPattern* GetPattern (uint32_t id);

  /** Access a known sequence by unique name*/
  GalapagosPattern* GetPattern (const char* name);

  size_t NumKnownPatterns ();

  /** access to list of known sequences by index */
  GalapagosPattern* GetKnownPattern (size_t ix);

  /** remove pattern from list by index*/
  void RemoveKnownPattern (size_t ix);

  bool SetChannelPattern (int chan, uint32_t id);

  bool SetChannelPattern (int chan, const char* name);

  GalapagosPattern* GetChannelPattern (int chan);

  uint32_t GetChannelPatternID (int chan);

};

#endif
