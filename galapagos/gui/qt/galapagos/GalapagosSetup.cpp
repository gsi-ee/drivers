#include "GalapagosSetup.h"
#include <QByteArray>
#include <stdint.h>

namespace gapg {


GalapagosSetup::GalapagosSetup () :
    BasicSetup (), fGeneratorActive (false), fChannelControl_0 (0), fChannelControl_1 (0)
{

  fKnownPatterns.clear ();
  fKnownSequences.clear ();
  for (int i = 0; i < GAPG_CHANNELS; ++i)
  {
    fChannelSequenceID[i] = 0;
    fChannelPatternID[i] = 0;
  }

}

uint64_t GalapagosSetup::GetChannelControl ()
{
  uint64_t reg = 0;
  reg = fChannelControl_0 | (fChannelControl_1 << 32);
  return reg;
}

void GalapagosSetup::SetChannelControl (uint64_t val)
{
  fChannelControl_0 = (val & 0xFFFFFFFF);
  fChannelControl_1 = (val >> 32) & 0xFFFFFFFF;
  //std::cout<< "GalapagosSetup::SetChannelControl for"<< std::hex << val<< "sets 0:"<< std::hex << fChannelControl_0<<", 1:" << std::hex<< fChannelControl_1 << std::endl;
}

void GalapagosSetup::SetChannelEnabled (uint8_t ch, bool on)
{
  //std::cout<< "GalapagosSetup::SetChannelEnabled ch="<< (int) ch<<", on="<<on << std::endl;
  if (ch > GAPG_CHANNELS)
    return;
  uint64_t reg = GetChannelControl ();
  uint64_t flags = (1 << ch);
  if (on)
    reg |= flags;
  else
    reg &= ~flags;
  SetChannelControl (reg);
}

bool GalapagosSetup::IsChannelEnabled (uint8_t ch)
{
  if (ch > GAPG_CHANNELS)
    return true;
  uint64_t reg = GetChannelControl ();
  uint64_t flags = (1 << ch);
  bool rev = ((reg & flags) == flags);
  return rev;
}

void GalapagosSetup::ClearSequences ()
{
  fKnownSequences.clear ();
  for (int i = 0; i < GAPG_CHANNELS; ++i)
    fChannelSequenceID[i] = 0;
}

GalapagosSequence& GalapagosSetup::AddSequence (GalapagosSequence& seq)
{
  // TODO: may need ot check if sequence of this id/name already exists?
  fKnownSequences.push_back (seq);
  return seq;
}

/* Access a known sequence by unique id number. May be redundant if we rely on name*/
GalapagosSequence* GalapagosSetup::GetSequence (uint32_t id)
{
  for (int t = 0; t < fKnownSequences.size (); ++t)
  {
    if (fKnownSequences[t].Id () == id)
      return &(fKnownSequences[t]);
  }
  return 0;
}

/* Access a known sequence by unique name*/
GalapagosSequence* GalapagosSetup::GetSequence (const char* name)
{
  for (int t = 0; t < fKnownSequences.size (); ++t)
  {
    if (fKnownSequences[t].EqualsName (name))
      return &(fKnownSequences[t]);
  }
  return 0;
}

size_t GalapagosSetup::NumKnownSequences ()
{
  return fKnownSequences.size ();
}

/** access to list of known sequences by index */
GalapagosSequence* GalapagosSetup::GetKnownSequence (size_t ix)
{
  if (ix > fKnownSequences.size ())
    return 0;
  return &(fKnownSequences[ix]);
}

/** remove sequence from list by index. also clean up all references in channels*/
void GalapagosSetup::RemoveKnownSequence (size_t ix)
{
  uint32_t sid = fKnownSequences[ix].Id ();
  fKnownSequences.erase (fKnownSequences.begin () + ix);
  uint32_t newsid = 0;
  if (fKnownSequences.size () > 0)
    newsid = fKnownSequences[0].Id ();    // channels with erased seqs will be assigned to first sequence
  for (int i = 0; i < GAPG_CHANNELS; ++i)
  {
    if (fChannelSequenceID[i] == sid)
      fChannelSequenceID[i] = newsid;
  }

}

//       /** discard pattern of id from list*/
//       bool GalapagosSetup::RemoveSequence(uint32_t id)
//       {
//         for(int t=0; t<fKnownSequences.size();++t)
//         {
//           if(fKnownSequences[t].Id()==id)
//           {
//             RemoveKnownSequence(t);
//             return true;
//           }
//         }
//         return false;
//       }

bool GalapagosSetup::SetChannelSequence (int chan, uint32_t id)
{
  if (chan > GAPG_CHANNELS)
    return false;
  GalapagosSequence* seq = GetSequence (id);
  if (seq == 0)
    return false;
  fChannelSequenceID[chan] = seq->Id ();
  return true;
}

bool GalapagosSetup::SetChannelSequence (int chan, const char* name)
{
  if (chan > GAPG_CHANNELS)
    return false;
  GalapagosSequence* seq = GetSequence (name);
  if (seq == 0)
    return false;
  fChannelSequenceID[chan] = seq->Id ();
  return true;
}

GalapagosSequence* GalapagosSetup::GetChannelSequence (int chan)
{
  if (chan > GAPG_CHANNELS)
    return 0;
  uint32_t id = GetChannelSequenceID (chan);
  if (id == 0)
    return 0;
  return GetSequence (id);
}

uint32_t GalapagosSetup::GetChannelSequenceID (int chan)
{
  if (chan > GAPG_CHANNELS)
    return 0;
  return fChannelSequenceID[chan];
}
/////////////////////////////77

GalapagosPattern& GalapagosSetup::AddPattern (GalapagosPattern& pat)
{
  // TODO: may need ot check if sequence of this id/name already exists?
  fKnownPatterns.push_back (pat);
  return pat;
}

/* Access a known sequence by unique id number. May be redundant if we rely on name*/
GalapagosPattern* GalapagosSetup::GetPattern (uint32_t id)
{
  for (int t = 0; t < fKnownPatterns.size (); ++t)
  {
    if (fKnownPatterns[t].Id () == id)
      return &(fKnownPatterns[t]);
  }
  return 0;
}

/* Access a known sequence by unique name*/
GalapagosPattern*GalapagosSetup::GetPattern (const char* name)
{
  for (int t = 0; t < fKnownPatterns.size (); ++t)
  {
    if (fKnownPatterns[t].EqualsName (name))
      return &(fKnownPatterns[t]);
  }
  return 0;
}

size_t GalapagosSetup::NumKnownPatterns ()
{
  return fKnownPatterns.size ();
}

/** access to list of known sequences by index */
GalapagosPattern* GalapagosSetup::GetKnownPattern (size_t ix)
{
  if (ix > fKnownPatterns.size ())
    return 0;
  return &(fKnownPatterns[ix]);
}

/** remove pattern from list by index*/
void GalapagosSetup::RemoveKnownPattern (size_t ix)
{
  uint32_t pid = fKnownPatterns[ix].Id ();
  fKnownPatterns.erase (fKnownPatterns.begin () + ix);
  uint32_t newpid = 0;
  if (fKnownPatterns.size () > 0)
    newpid = fKnownPatterns[0].Id ();    // channels with erased patterns will be assigned to first pattern
  for (int i = 0; i < GAPG_CHANNELS; ++i)
  {
    if (fChannelPatternID[i] == pid)
      fChannelPatternID[i] = newpid;
  }
}

bool GalapagosSetup::SetChannelPattern (int chan, uint32_t id)
{
  if (chan > GAPG_CHANNELS)
    return false;
  GalapagosPattern* pat = GetPattern (id);
  if (pat == 0)
    return false;
  fChannelPatternID[chan] = pat->Id ();
  return true;
}

bool GalapagosSetup::SetChannelPattern (int chan, const char* name)
{
  if (chan > GAPG_CHANNELS)
    return false;
  GalapagosPattern* pat = GetPattern (name);
  if (pat == 0)
    return false;
  fChannelPatternID[chan] = pat->Id ();
  return true;
}

GalapagosPattern* GalapagosSetup::GetChannelPattern (int chan)
{
  if (chan > GAPG_CHANNELS)
    return 0;
  uint32_t id = GetChannelPatternID (chan);
  if (id == 0)
    return 0;
  return GetPattern (id);
}

uint32_t GalapagosSetup::GetChannelPatternID (int chan)
{
  if (chan > GAPG_CHANNELS)
    return 0;
  return fChannelPatternID[chan];
}

} // gapg
