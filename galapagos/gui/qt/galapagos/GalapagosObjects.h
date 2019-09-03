#ifndef GAPG_GALAPAGOSOBJECTS_H
#define GAPG_GALAPAGOSOBJECTS_H

#include <QByteArray>
#include <stdint.h>
#include <vector>

#include "BasicObject.h"


namespace gapg{

/** container to keep the different basic bit patterns used in the signal sequences*/
class GalapagosPattern : public gapg::BasicObject
{

public:

  /** the full stream of the bit pattern will be chunked and put into infinitively extendable vector ?*/
  std::vector<uint8_t> fPattern;


  GalapagosPattern(uint16_t id, const char* name): gapg::BasicObject(id, name)
    {
      Clear();
    }

  GalapagosPattern(const GalapagosPattern& seq) : gapg::BasicObject(seq)
   {
     //std::cout<< "GalapagosPattern copy ctor " << std::endl;
     fPattern=seq.fPattern;
   }

  GalapagosPattern& operator=(const GalapagosPattern& rhs)
      {
        if (this != &rhs) {
          BasicObject::operator=(rhs);
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
class GalapagosSequence : public gapg::BasicObject
{

public:


  /** the actual command sequence as strings*/
  std::vector<std::string> fCommandSkript;

  /* to do: translate in bit sequence recognizable in fpga?*/
  std::vector<uint32_t> fCommandTokens;


  GalapagosSequence(uint16_t id, const char* name): gapg::BasicObject(id, name)
  {
    Clear();
  }

  GalapagosSequence(const GalapagosSequence& seq) : gapg::BasicObject(seq)
  {
    //std::cout<< "GalapagosSequence copy ctor " << std::endl;
    fCommandSkript=seq.fCommandSkript;
    fCommandTokens=seq.fCommandTokens;
  }

  GalapagosSequence& operator=(const GalapagosSequence& rhs)
    {
      if (this != &rhs) {
        //std::cout<< "GalapagosSequence operator= "<< std::endl;
        BasicObject::operator=(rhs);
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


} // namespace


#endif
