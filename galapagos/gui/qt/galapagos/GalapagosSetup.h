#ifndef GAPGSETUP_H
#define GAPGSETUP_H

#include "BasicGui.h"
#include <QByteArray>
#include <stdint.h>

// magic number for pattern file header,  ASCII identifier GAPG:
#define PATTERN_FILE_TAG 0x47415047

//0xFA1AEAF0

// pattern file format version
#define PATTERN_FILE_VERSION 0x1


/** total number of output channels of pattern generator*/
//#define GAPG_CHANNELS 64
#define GAPG_CHANNELS 16
#define GAPG_SEQUENCES 128
#define GAPG_PATTLETS 128

/** following just fake addresses at the moment to test kernel module*/
#define GAPG_DRAM        0x100000
/*for register testing: use pexor sfp memory*/
#define GAPG_MAIN_CONTROL GAPG_DRAM

#define GAPG_CHANNEL_ENABLE_LOW GAPG_DRAM +4
#define GAPG_CHANNEL_ENABLE_HI  GAPG_DRAM + 8

/** register set to assign sequence id to channels*/
#define GAPG_CHANNEL_SEQUENCE_BASE  GAPG_DRAM + 12

/** start updating sequence by writing id into control word*/
#define GAPG_KNOWN_SEQUENCE_CONTROL  GAPG_DRAM + 256

/** port address fifo to send sequence command tokens*/
#define GAPG_KNOWN_SEQUENCE_PORT  GAPG_DRAM + 264


#define GAPG_BIT_MAIN_ENABLE 0x1

////// end FAKE register setup

/////////////////// JAM 2019 -
///////// below are old definitions from febex, not used.
// keep them as some example what we could do


//#define GOS_I2C_DWR  0x208010  // i2c data write reg.   addr
//#define GOS_I2C_DRR1 0x208020  // i2c data read  reg. 1 addr
//
//
///** i2c address of first mcp443x/5x/ chip on febex for writing values. Used as base
// * to evaluate values for all 4 chips on board with 4 channels each:*/
//#define GAPG_MCP433_BASE_WRITE 0x62580000
//
///** i2c address of first mcp443x/5x/ chip on febex for read request. Used as base
// * to evaluate values for all 4 chips on board with 4 channels each:*/
//#define GAPG_MCP433_BASE_READ  0xe2580c00
//
///** this value is i2c adressing offset between mcp chips*/
//#define GAPG_MCP433_OFFSET 0x20000
//
///** i2c command value to request a data read from mcp433 */
//#define GAPG_MCP433_REQUEST_READ 0x86000000
//
///** number of dac chips on febex */
//#define GAPG_MCP433_NUMCHIPS 4
//
///** number of dac  channels per chip*/
//#define GAPG_MCP433_NUMCHAN 4
//
///** maximum value to set for DAC*/
//#define GAPG_MCP433_MAXVAL 0xFF
//
///** adress to read actual adc value. adc id and channel must be
// * written to this address first*/
//#define GAPG_ADC_PORT  0x20001c
//
///** number of adc units per febex*/
//#define GAPG_ADC_NUMADC 2
//
///** number of channels per adc unit*/
//#define GAPG_ADC_NUMCHAN 8
//
//
///** total number of channels on febex*/
//#define GAPG_CH 16
//
//
///* number of samples to evaluate average adc baseline value*/
//#define GAPG_ADC_BASELINESAMPLES 3
//
//
////////////////////////////////////////////////////////////////////////7
//
///* The following is taken from mbs code for initialization of febex after startup:*/
//
//
//#define FEB_TRACE_LEN   300  // in nr of samples
//#define FEB_TRIG_DELAY   30  // in nr.of samples
//
//
//
//#define REG_BUF0_DATA_LEN     0xFFFD00  // buffer 0 submemory data length
//#define REG_BUF1_DATA_LEN     0xFFFE00  // buffer 1 submemory data length
//
//
//#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
//#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
//#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
//#define REG_MEM_FLAG_1      0xFFFFBc  // read only:
//
//
//#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
//#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
//#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
//#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
//
//#define REG_MODID     0xFFFFE0
//#define REG_HEADER    0xFFFFE4
//#define REG_FOOTER    0xFFFFE8
//#define REG_DATA_LEN  0xFFFFEC
//
//#define REG_RST 0xFFFFF4
//#define REG_LED 0xFFFFF8
//#define REG_VERSION 0xFFFFFC
//
//#define REG_FEB_CTRL       0x200000
//#define REG_FEB_TRIG_DELAY 0x200004
//#define REG_FEB_TRACE_LEN  0x200008
//#define REG_FEB_SELF_TRIG  0x20000C
//#define REG_FEB_STEP_SIZE  0x200010
//#define REG_FEB_SPI        0x200014
//#define REG_FEB_TIME       0x200018
//#define REG_FEB_XXX        0x20001C
//
//
//
//#define DATA_FILT_CONTROL_REG 0x2080C0
//#define DATA_FILT_CONTROL_DAT 0x80         // (0x80 E,t summary always +  data trace                 always
//                                           // (0x82 E,t summery always + (data trace + filter trace) always
//                                           // (0x84 E,t summery always +  data trace                 if > 1 hit
//                                           // (0x86 E,t summery always + (data trace + filter trace) if > 1 hit
//// Trigger/Hit finder filter
//
//#define TRIG_SUM_A_REG    0x2080D0
//#define TRIG_GAP_REG      0x2080E0
//#define TRIG_SUM_B_REG    0x2080F0
//
//#define TRIG_SUM_A     8  // for 12 bit: 8, 4 ,9 (8+1); for 14 bit: 14, 4, 15 (14 + 1).
//#define TRIG_GAP       4
//#define TRIG_SUM_B     9 // 8 + 1: one has to be added.
//
//// Energy Filters and Modes
//
//#define ENABLE_ENERGY_FILTER 1
//
//#define TRAPEZ               1  // if TRAPEZ is off, MWD will be activated
//
//#ifdef ENABLE_ENERGY_FILTER
// #ifdef TRAPEZ
//  #define ENERGY_SUM_A_REG  0x208090
//  #define ENERGY_GAP_REG    0x2080A0
//  #define ENERGY_SUM_B_REG  0x2080B0
//
//  #define ENERGY_SUM_A  64
//  #define ENERGY_GAP    32
//  #define ENERGY_SUM_B  65  // 64 + 1: one has to be added.
// #endif

//#endif


/** Base class for identifieable objects*/
class GalapagosObject
{
  public:

  /** unique id number */
  uint32_t fId;
  /** user readable name, same as in sequencer control window */
  std::string fName;

  GalapagosObject(uint16_t id, const char* name): fId(id), fName(name){}

  GalapagosObject(const GalapagosObject& ob)
  {
    //std::cout<< "GalapagosObject copy ctor for "<<ob.fName.c_str() << std::endl;
    fId=ob.fId;
    fName=ob.fName;

  }

  GalapagosObject& operator=(const GalapagosObject& rhs)
  {
    if (this != &rhs) {
    //std::cout<< "GalapagosObject operator= for "<<rhs.fName.c_str() << std::endl;
    fId=rhs.fId;
    fName=rhs.fName;
    }
    return *this;
  }

  virtual ~GalapagosObject(){}

  uint32_t Id() {return fId;}

  void SetId(uint32_t val){fId=val;}

  const char* Name() {return fName.c_str();}

  void SetName(const char* nm){fName=nm;}

  bool EqualsName(const char* nm)
  {
    return (fName.compare(std::string(nm)) ==0) ? true : false;
  }

};


/** container to keep the different basic bit patterns used in the signal sequences*/
class GalapagosPattern : public GalapagosObject
{

public:

  /** the full stream of the bit pattern will be chunked and put into infinitively extendable vector ?*/
  std::vector<uint8_t> fPattern;


  GalapagosPattern(uint16_t id, const char* name): GalapagosObject(id, name)
    {
      Clear();
    }

  GalapagosPattern(const GalapagosPattern& seq) : GalapagosObject(seq)
   {
     //std::cout<< "GalapagosPattern copy ctor " << std::endl;
     fPattern=seq.fPattern;
   }

  GalapagosPattern& operator=(const GalapagosPattern& rhs)
      {
        if (this != &rhs) {
          GalapagosObject::operator=(rhs);
          fPattern=rhs.fPattern;
        }
        return *this;
      }



  virtual ~GalapagosPattern(){}

  void Clear()
    {
      fPattern.clear();
    }

  /** fill pattern*/
  void AddByte (uint8_t patty)
  {
    fPattern.push_back(patty);
  }

  size_t NumBits()
  {
    /** for the moment, do not support patterns that are below one byte units*/
    return fPattern.size() * sizeof(uint8_t);
  }

  size_t NumBytes()
  {
    return fPattern.size();
  }

  /** read next byte of bit pattern at given vector index.*/
  uint8_t GetByte(int ix)
  {
    if(ix>fPattern.size()) return 0;
    return fPattern[ix];
  }

  // provide tempory bytearray:
  QByteArray GetByteArray()
  {
     QByteArray theByteArray;
     for(int c=0; c<NumBytes(); ++c)
       theByteArray.append(GetByte(c));
     return theByteArray;
  }
};




/** this container will keep the bit sequence to play for each channel */
class GalapagosSequence : public GalapagosObject
{

public:


  /** the actual command sequence as strings*/
  std::vector<std::string> fCommandSkript;

  /* to do: translate in bit sequence recognizable in fpga?*/
  std::vector<uint32_t> fCommandTokens;


  GalapagosSequence(uint16_t id, const char* name): GalapagosObject(id, name)
  {
    Clear();
  }

  GalapagosSequence(const GalapagosSequence& seq) : GalapagosObject(seq)
  {
    //std::cout<< "GalapagosSequence copy ctor " << std::endl;
    fCommandSkript=seq.fCommandSkript;
    fCommandTokens=seq.fCommandTokens;
  }

  GalapagosSequence& operator=(const GalapagosSequence& rhs)
    {
      if (this != &rhs) {
        //std::cout<< "GalapagosSequence operator= "<< std::endl;
        GalapagosObject::operator=(rhs);
        fCommandSkript=rhs.fCommandSkript;
        fCommandTokens=rhs.fCommandTokens;
      }
      return *this;
    }

  virtual ~GalapagosSequence(){}



  void AddCommand(const char* cmd)
  {
    if(cmd==0) return;
    fCommandSkript.push_back(std::string(cmd));
  }


  const char* GetCommandLine(int ix)
   {
     if(ix>=fCommandSkript.size()) return 0;
     return fCommandSkript[ix].c_str();
   }

  void Compile()
  {
    fCommandTokens.clear();

    // TODO: translate command language into the fpga byte code here
    fCommandTokens.push_back(42);
    fCommandTokens.push_back(0);
    fCommandTokens.push_back(1); // just dummies to test io

  }

  /** read command token of given index*/
   uint32_t GetCommandToken(int ix)
   {
     if(ix>=fCommandTokens.size()) return 0;
     return fCommandTokens[ix];
   }

   size_t NumCommandTokens()
    {
      return fCommandTokens.size();
    }

  void Clear()
  {
    fCommandSkript.clear();
    fCommandTokens.clear();
  }

};



/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class GalapagosSetup : public BasicSetup
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


     /** references to sequence to be activated*/
     //GalapagosSequence* fChannelSequence[GAPG_CHANNELS];

      /** unique sequence id for each cheannel*/
     uint32_t fChannelSequenceID[GAPG_CHANNELS];

     /** unique pattern  id for each cheannel*/
     uint32_t fChannelPatternID[GAPG_CHANNELS];


     /* list of known pattern sequences as visible in the sequence editor*/
     std::vector<GalapagosSequence> fKnownSequences;

     std::vector<GalapagosPattern> fKnownPatterns;









  /* all initialization here:*/
  GalapagosSetup (): BasicSetup(),
      fGeneratorActive(false), fChannelControl_0(0),fChannelControl_1(0)  {

      fKnownPatterns.clear();
      fKnownSequences.clear();
      for (int i=0; i< GAPG_CHANNELS; ++i)
        {
          fChannelSequenceID[i]=0;
          fChannelPatternID[i]=0;
        }

  }

    bool IsGeneratorActive()
    {
      return fGeneratorActive;
    }

    void SetGeneratorActive(bool on)
    {
      fGeneratorActive=on;
    }

    uint64_t GetChannelControl()
    {
      uint64_t reg=0;
      reg = fChannelControl_0 | (fChannelControl_1 << 32);
      return reg;
    }

    uint32_t GetChannelControl_0()
      {
        return fChannelControl_0;
      }

    void SetChannelControl_0(uint32_t val)
    {
      fChannelControl_0=val;
    }


    uint32_t GetChannelControl_1()
         {
           return fChannelControl_1;
         }
    void SetChannelControl_1(uint32_t val)
           {
             fChannelControl_1=val;
           }

    void SetChannelControl (uint64_t val)
    {
      fChannelControl_0 = (val & 0xFFFFFFFF);
      fChannelControl_1 = (val >> 32) & 0xFFFFFFFF;
      //std::cout<< "GalapagosSetup::SetChannelControl for"<< std::hex << val<< "sets 0:"<< std::hex << fChannelControl_0<<", 1:" << std::hex<< fChannelControl_1 << std::endl;
    }

   void SetChannelEnabled(uint8_t ch, bool on)
   {
       //std::cout<< "GalapagosSetup::SetChannelEnabled ch="<< (int) ch<<", on="<<on << std::endl;
       if(ch>GAPG_CHANNELS) return;
       uint64_t reg=GetChannelControl();
       uint64_t flags= (1 << ch);
       if(on)
         reg |= flags;
       else
         reg &= ~flags;
       SetChannelControl(reg);
   }

   bool IsChannelEnabled(uint8_t ch)
    {
        if(ch>GAPG_CHANNELS) return true;
        uint64_t reg=GetChannelControl();
        uint64_t flags= (1 << ch);
        bool rev = ((reg & flags) == flags);
        return rev;
    }

   void ClearSequences()
   {
     fKnownSequences.clear();
     for (int i=0; i< GAPG_CHANNELS; ++i)
            fChannelSequenceID[i]=0;
   }

   GalapagosSequence& AddSequence(GalapagosSequence& seq)
   {
     // TODO: may need ot check if sequence of this id/name already exists?
     fKnownSequences.push_back(seq);
     return seq;
   }

   /* Access a known sequence by unique id number. May be redundant if we rely on name*/
   GalapagosSequence* GetSequence(uint32_t id)
   {
     for(int t=0; t<fKnownSequences.size();++t)
     {
         if(fKnownSequences[t].Id()==id)
           return &(fKnownSequences[t]);
     }
     return 0;
   }

   /* Access a known sequence by unique name*/
   GalapagosSequence* GetSequence(const char* name)
      {
        for(int t=0; t<fKnownSequences.size();++t)
        {
          if(fKnownSequences[t].EqualsName(name))
              return &(fKnownSequences[t]);
        }
        return 0;
      }

   size_t NumKnownSequences() {return fKnownSequences.size();}

   /** access to list of known sequences by index */
   GalapagosSequence* GetKnownSequence(size_t ix)
     {
       if(ix>fKnownSequences.size()) return 0;
       return &(fKnownSequences[ix]);
     }


   /** remove sequence from list by index. also clean up all references in channels*/
       void RemoveKnownSequence(size_t ix)
       {
           uint32_t sid=fKnownSequences[ix].Id();
           fKnownSequences.erase(fKnownSequences.begin()+ix);
           uint32_t newsid=0;
           if(fKnownSequences.size()>0)
             newsid=fKnownSequences[0].Id(); // channels with erased seqs will be assigned to first sequence
           for (int i=0; i< GAPG_CHANNELS; ++i)
           {
             if(fChannelSequenceID[i]==sid)
               fChannelSequenceID[i]=newsid;
           }

       }

//       /** discard pattern of id from list*/
//       bool RemoveSequence(uint32_t id)
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



   bool SetChannelSequence(int chan, uint32_t id)
   {
     if(chan>GAPG_CHANNELS) return false;
     GalapagosSequence* seq=GetSequence(id);
     if(seq==0) return false;
     fChannelSequenceID[chan] = seq->Id();
     return true;
   }

   bool SetChannelSequence(int chan, const char* name)
     {
       if(chan>GAPG_CHANNELS) return false;
       GalapagosSequence* seq=GetSequence(name);
       if(seq==0) return false;
       fChannelSequenceID[chan] = seq->Id();
       return true;
     }

   GalapagosSequence* GetChannelSequence(int chan)
   {
     if(chan>GAPG_CHANNELS) return 0;
     uint32_t id=GetChannelSequenceID(chan);
     if(id==0) return 0;
     return GetSequence(id);
   }

   uint32_t GetChannelSequenceID(int chan)
   {
     if(chan>GAPG_CHANNELS) return 0;
     return fChannelSequenceID[chan];
   }
/////////////////////////////77

   GalapagosPattern& AddPattern(GalapagosPattern& pat)
     {
       // TODO: may need ot check if sequence of this id/name already exists?
       fKnownPatterns.push_back(pat);
       return pat;
     }

     /* Access a known sequence by unique id number. May be redundant if we rely on name*/
     GalapagosPattern* GetPattern(uint32_t id)
     {
       for(int t=0; t<fKnownPatterns.size();++t)
       {
           if(fKnownPatterns[t].Id()==id)
             return &(fKnownPatterns[t]);
       }
       return 0;
     }

     /* Access a known sequence by unique name*/
     GalapagosPattern* GetPattern(const char* name)
        {
          for(int t=0; t<fKnownPatterns.size();++t)
          {
            if(fKnownPatterns[t].EqualsName(name))
                return &(fKnownPatterns[t]);
          }
          return 0;
        }

     size_t NumKnownPatterns() {return fKnownPatterns.size();}

     /** access to list of known sequences by index */
     GalapagosPattern* GetKnownPattern(size_t ix)
       {
         if(ix>fKnownPatterns.size()) return 0;
         return &(fKnownPatterns[ix]);
       }


     /** remove pattern from list by index*/
     void RemoveKnownPattern(size_t ix)
     {
         uint32_t pid=fKnownPatterns[ix].Id();
         fKnownPatterns.erase(fKnownPatterns.begin()+ix);
         uint32_t newpid=0;
         if(fKnownPatterns.size()>0)
           newpid=fKnownPatterns[0].Id(); // channels with erased patterns will be assigned to first pattern
         for (int i=0; i< GAPG_CHANNELS; ++i)
                {
                  if(fChannelPatternID[i]==pid)
                    fChannelPatternID[i]=newpid;
                }
     }

//     /** discard pattern of id from list*/
//     bool RemovePattern(uint32_t id)
//         {
//             for(int t=0; t<fKnownPatterns.size();++t)
//                  {
//                      if(fKnownPatterns[t].Id()==id)
//                      {
//                        RemoveKnownPattern(t);
//                        return true;
//                      }
//                  }
//             return false;
//         }
//


};


#endif
