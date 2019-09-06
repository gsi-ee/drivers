#ifndef GAPG_GALAPAGOSOBJECTS_H
#define GAPG_GALAPAGOSOBJECTS_H

#include <QByteArray>
//#include <stdint.h>
#include <vector>

#include "BasicObject.h"


namespace gapg{


typedef enum CoreType
{
  NOP,
  CHN,
  TRG,
  USP,
  LJP,
  DAC
}CoreType_t;



/** container to keep the different basic bit patterns used in the signal sequences*/
class GalapagosPattern : public gapg::BasicObject
{

private:

  /** the full stream of the bit pattern will be chunked and put into infinitively extendable vector ?*/
  std::vector<uint8_t> fData;

public:

  GalapagosPattern(uint32_t id, const char* name);

  GalapagosPattern();

  GalapagosPattern(const GalapagosPattern& seq);

  GalapagosPattern& operator=(const GalapagosPattern& rhs);

  virtual ~GalapagosPattern();

  void Clear();

  /** fill pattern*/
  void AddByte (uint8_t patty);

  /** number of bits in the pattern (needed?)*/
  size_t NumBits();

  /** number of bytes forming the pattern*/
  size_t NumBytes();

  /** read next byte of bit pattern at given byte index.*/
  uint8_t GetByte(int ix);

  /**provide tempory bytearray from pattern data: */
  QByteArray GetByteArray();
};




/** this container will keep the command code and pattern to play for each channel */
class GalapagosKernel : public gapg::BasicObject
{

private:

  gapg::CoreType_t fCoretype;


  /**unique id of pattern assigned to this kernel*/
  uint32_t fPatternID;

  /** temporary copy of the referenced pattern*/
  gapg::GalapagosPattern fPattern;


  /** the actual command sequence as strings*/
  std::vector<std::string> fCommandSkript;


  /** the compiled instruction code*/
  std::vector<uint8_t> fCompiledCode;

public:

  GalapagosKernel(uint32_t id, const char* name);

  GalapagosKernel();

  GalapagosKernel(const GalapagosKernel& ker);

  GalapagosKernel& operator=(const GalapagosKernel& rhs);

  virtual ~GalapagosKernel();


  uint32_t GetPatternID(){ return fPatternID;}
  void SetPatternID(uint32_t id){fPatternID=id;}


  gapg::CoreType_t GetCoreType(){ return fCoretype;}
  void SetCoreType(gapg::CoreType_t id){fCoretype=id;}

  /** add single command expression (with arguments) to the kernel code*/
  void AddCommand(const char* cmd);

  /** access command line at index ix as text*/
  const char* GetCommandLine(int ix);

  /** number of bytes of the compiled kernel code */
  size_t NumCodeBytes();

  /** read next byte of compiled code at given byte index.*/
   uint8_t GetCodeByte(int ix);

    // provide tempory bytearray of the command code:
    QByteArray GetCodeByteArray();

    /** number of bytes the pattern of this kernelconsists of*/
    size_t NumPatternBytes();

      /** read next byte of bit pattern at given byte index.*/
    uint8_t GetPatternByte(int ix);

    /** provide tempory bytearray of pattern data: */
      QByteArray GetPatternByteArray();

      /** set the full pattern data for this kernel*/
      void UpdatePattern(const GalapagosPattern& src);

      /** compile the code*/
      void Compile();


      /** reset this kernel to nothing*/
      void Clear();


      /** cleanup all references to the pattern object of unique id id.
        * may specify a new unique id to replace the old reference*/
      void CleanupRemovedPattern(uint32_t id, uint32_t newid=0);

};

/** This object composes the complete core setup of the board*/
class GalapagosPackage : public gapg::BasicObject
{

  private:


  /* mockup of a future channel control/status register
    * each bit may set corresponding channel active*/
   uint32_t fCoreControl_0;

   /* mockup of a future channel control/status register
    * each bit may set corresponding channel active*/
   uint32_t fCoreControl_1;


   /* this array holds the unique id of the kernels assigned to the cores of given index*/
   std::vector<uint32_t> fCoreKernelIDs;

  /* this array holds the kernels assigned to the cores of given index*/
  std::vector<gapg::GalapagosKernel> fCoreKernels;



 public:

  GalapagosPackage(uint32_t id, const char* name);

  GalapagosPackage(const GalapagosPackage& ker);

  GalapagosPackage& operator=(const GalapagosPackage& rhs);

  virtual ~GalapagosPackage();

  void Clear();

  /** set kernel object for core of index*/
  bool SetKernel(int core, const gapg::GalapagosKernel& src);

  /** get reference to kernel object for core of index. Returns 0 pointer is out of range*/
  gapg::GalapagosKernel* GetKernel(int index);


  /** set planned kernel object id for core of index*/
  bool SetKernelID(int core, uint32_t id);

  /** get reference to kernel object for core of index. Returns 0 pointer is out of range*/
  uint32_t GetKernelID(int index);

  uint64_t GetCoreControl ();

    uint32_t GetCoreControl_0 ()
    {
      return fCoreControl_0;
    }

    void SetCoreControl_0 (uint32_t val)
    {
      fCoreControl_0 = val;
    }

    uint32_t GetCoreControl_1 ()
    {
      return fCoreControl_1;
    }
    void SetCoreControl_1 (uint32_t val)
    {
      fCoreControl_1 = val;
    }

    void SetCoreControl (uint64_t val);

    void SetCoreEnabled (uint8_t ch, bool on);

    bool IsCoreEnabled (uint8_t ch);


    /** cleanup all references to the kernel object of unique id id.
     * may specify a new unique id to replace the old reference*/
    void CleanupRemovedKernel(uint32_t id, uint32_t newid=0);


    /** cleanup all references to the pattern object of unique id id.
     * may specify a new unique id to replace the old reference*/
    void CleanupRemovedPattern(uint32_t id, uint32_t newid=0);



};


} // namespace


#endif
