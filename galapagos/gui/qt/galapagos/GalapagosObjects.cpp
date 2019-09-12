#include "GalapagosObjects.h"
#include "GalapagosDefines.h"

#ifdef USE_GALAPAGOS_LIB
extern "C"
{
#include "galapagos/libgalapagos.h"
}
#endif

#include <QString>

namespace gapg
{

 GalapagosPattern::GalapagosPattern(uint32_t id, const char* name): gapg::BasicObject(id, name)
    {
      Clear();
    }

 GalapagosPattern::GalapagosPattern(): gapg::BasicObject(0, "null pattern")
     {
       Clear();
     }

 GalapagosPattern::GalapagosPattern(const GalapagosPattern& seq) : gapg::BasicObject(seq)
   {
     //std::cout<< "GalapagosPattern copy ctor " << std::endl;
     fData=seq.fData;
   }

  GalapagosPattern& GalapagosPattern::operator=(const GalapagosPattern& rhs)
      {
        if (this != &rhs) {
          BasicObject::operator=(rhs);
          fData=rhs.fData;
        }
        return *this;
      }



  GalapagosPattern::~GalapagosPattern(){}

  void GalapagosPattern::Clear()
    {
      fData.clear();
    }

  /** fill pattern*/
  void GalapagosPattern::AddByte (uint8_t patty)
  {
    fData.push_back(patty);
  }

  size_t GalapagosPattern:: NumBits()
  {
    /** for the moment, do not support patterns that are below one byte units*/
    return fData.size() * sizeof(uint8_t);
  }

  size_t GalapagosPattern:: NumBytes()
  {
    return fData.size();
  }

  /** read next byte of bit pattern at given vector index.*/
  uint8_t GalapagosPattern::GetByte(int ix)
  {
    if(ix>fData.size()) return 0;
    return fData[ix];
  }

  // provide tempory bytearray:
  QByteArray* GalapagosPattern::CreateByteArray()
  {
     QByteArray* theByteArray = new QByteArray;
     for(int c=0; c<NumBytes(); ++c)
       theByteArray->append(GetByte(c));
     return theByteArray;
  }

///////////////////////////////////////////////////////////////////////////////////


  GalapagosKernel::GalapagosKernel(uint32_t id, const char* name): gapg::BasicObject(id, name)
    {
      Clear();
    }

  GalapagosKernel::GalapagosKernel(): gapg::BasicObject(0, "empty kernel")
     {
       Clear();
     }

  GalapagosKernel::GalapagosKernel(const GalapagosKernel& ker) : gapg::BasicObject(ker)
    {
      //std::cout<< "GalapagosKernel copy ctor " << std::endl;
      fCoretype=ker.fCoretype;
      fCommandSkript=ker.fCommandSkript;
      fPatternID=ker.fPatternID;
      ::galapagos_copy_kernel(&(ker.fKernelBinary),&fKernelBinary);
      fPattern=ker.fPattern;
      //fCommandTokens=seq.fCommandTokens;
    }

    GalapagosKernel& GalapagosKernel::operator=(const GalapagosKernel& rhs)
      {
        if (this != &rhs) {
          //std::cout<< "GalapagosKernel operator= "<< std::endl;
          BasicObject::operator=(rhs);
          fCoretype=rhs.fCoretype;
          fCommandSkript=rhs.fCommandSkript;
          fPatternID=rhs.fPatternID;
          fPattern=rhs.fPattern;
          ::galapagos_copy_kernel(&(rhs.fKernelBinary),&fKernelBinary);
        }
        return *this;
      }

    GalapagosKernel::~GalapagosKernel(){}





    void GalapagosKernel::AddCommand(const char* cmd)
    {
      if(cmd==0) return;
      fCommandSkript.push_back(std::string(cmd));
    }


    const char* GalapagosKernel::GetCommandLine(int ix)
     {
       if(ix>=fCommandSkript.size()) return 0;
       return fCommandSkript[ix].c_str();
     }

     QString* GalapagosKernel::GetSourceCode()
     {
        QString* theSource = new QString;
        for(int c=0; c<fCommandSkript.size(); ++c)
        {
          theSource->append(fCommandSkript[c].c_str());
          theSource->append("\n");
        }
          return theSource;
     }



//    size_t GalapagosKernel::NumCodeBytes()
//      {
//        return fCompiledCode.size();
//      }
//
//      /** read next byte of bit pattern at given vector index.*/
//      uint8_t GalapagosKernel::GetCodeByte(int ix)
//      {
//        if(ix>fCompiledCode.size()) return 0;
//        return fCompiledCode[ix];
//      }
//
//      // provide tempory bytearray:
//      QByteArray GalapagosKernel::GetCodeByteArray()
//      {
//         QByteArray theByteArray;
//         for(int c=0; c<NumCodeBytes(); ++c)
//           theByteArray.append(GetCodeByte(c));
//         return theByteArray;
//      }


      size_t GalapagosKernel::NumPatternBytes()
        {
          return fPattern.NumBytes();
        }

        /** read next byte of bit pattern at given vector index.*/
        uint8_t GalapagosKernel::GetPatternByte(int ix)
        {
          return fPattern.GetByte(ix);
        }

        // provide tempory bytearray:
        QByteArray* GalapagosKernel::CreatePatternByteArray()
        {
           return fPattern.CreateByteArray();
        }


        void GalapagosKernel::UpdatePattern(const GalapagosPattern& src)
        {
          fPattern=src;
        }

    bool  GalapagosKernel::Compile()
    {
       // here use galapagos library
      // may get back the bytecode from it
      QString* srcstring=GetSourceCode();
      if(srcstring==0) return false;
      const char* sourcecode=srcstring->toLatin1().data();
      size_t length=srcstring->size();
      if(::galapagos_compile_kernel(sourcecode, length, &fKernelBinary)!=0)
        {
          delete srcstring;
          return false;
        }
      QByteArray* patternarray=CreatePatternByteArray();
      size_t len=NumPatternBytes();
      if(len>GALAPAGOS_KERNELPATTERNSIZE) len=GALAPAGOS_KERNELPATTERNSIZE;
      memcpy(&(fKernelBinary.pattern_buffer), patternarray->data(),len);
      delete srcstring;
      delete patternarray;
      return true;
    }



    void GalapagosKernel::Clear()
    {
      fCoretype=gapg::NOP;
      fPatternID=0;
      fCommandSkript.clear();
      fPattern.Clear();
      memset(&fKernelBinary,0,sizeof(::galapagos_kernel));
    }


    void GalapagosKernel::CleanupRemovedPattern(uint32_t id, uint32_t newid)
    {
        if(fPatternID == id)
        {
          fPatternID=newid;
          fPattern.Clear();
        }

    }


////////////////////////////////////////////////////////////////////////////////////////



    GalapagosPackage::GalapagosPackage(uint32_t id, const char* name): gapg::BasicObject(id, name)
       {
         Clear();
       }

    GalapagosPackage::GalapagosPackage(const GalapagosPackage& pak) : gapg::BasicObject(pak)
       {
         //std::cout<< "GalapagosPackage copy ctor " << std::endl;
          fCoreKernels=pak.fCoreKernels;
          fCoreKernelIDs=pak.fCoreKernelIDs;
          fCoreControl_0=pak.fCoreControl_0;
          fCoreControl_1=pak.fCoreControl_1;
       }

       GalapagosPackage& GalapagosPackage::operator=(const GalapagosPackage& rhs)
         {
           if (this != &rhs) {
             //std::cout<< "GalapagosPackage operator= "<< std::endl;
             BasicObject::operator=(rhs);
             fCoreKernels=rhs.fCoreKernels;
             fCoreKernelIDs=rhs.fCoreKernelIDs;
             fCoreControl_0=rhs.fCoreControl_0;
             fCoreControl_1=rhs.fCoreControl_1;
           }
           return *this;
         }

       GalapagosPackage::~GalapagosPackage(){}

       void GalapagosPackage::Clear()
         {
           fCoreKernels.clear();
           fCoreKernels.resize(GAPG_CORES);
           fCoreKernelIDs.clear();
           fCoreKernelIDs.resize(GAPG_CORES);
           fCoreControl_0=0;
           fCoreControl_1=0;
         }


       void GalapagosPackage::CleanupRemovedKernel(uint32_t id, uint32_t newid)
       {
         for(int k=0; k<fCoreKernelIDs.size();++k)
         {
           if(fCoreKernelIDs[k]==id)
           {
             fCoreKernelIDs[k]=newid;
             fCoreKernels[k].Clear();
           }
         }
       }

       void GalapagosPackage::CleanupRemovedPattern(uint32_t id, uint32_t newid)
       {
         for(int k=0; k<fCoreKernelIDs.size();++k)
                {
                   if(fCoreKernels[k].GetPatternID()==id)
                    {
                      fCoreKernels[k].SetPatternID(newid);
                      // we do not update the pattern data itself here, since this will be done when calling Compile anywa
                    }
                  }
       }


       bool GalapagosPackage::SetKernelID(int index, uint32_t id)
        {
         if(index>=GAPG_CORES) return false;
         fCoreKernelIDs[index]=id;
         return true;
        }

        uint32_t GalapagosPackage::GetKernelID(int index)
        {
          if(index>=GAPG_CORES) return 0;
          return fCoreKernelIDs[index];
        }


       bool GalapagosPackage::SetKernel(int index, const gapg::GalapagosKernel& src)
       {
         if(index>=GAPG_CORES) return false;
         fCoreKernels[index]=src;
         return true;
       }

       gapg::GalapagosKernel* GalapagosPackage::GetKernel(int index)
       {
         if(index>=GAPG_CORES) return 0;
         return &(fCoreKernels[index]);
       }





       uint64_t GalapagosPackage::GetCoreControl ()
       {
         uint64_t reg = 0;
         reg = fCoreControl_0 | (fCoreControl_1 << 32);
         return reg;
       }

       void GalapagosPackage::SetCoreControl (uint64_t val)
       {
         fCoreControl_0 = (val & 0xFFFFFFFF);
         fCoreControl_1 = (val >> 32) & 0xFFFFFFFF;
         //std::cout<< "GalapagosPackage::SetCoreControl for"<< std::hex << val<< "sets 0:"<< std::hex << fCoreControl_0<<", 1:" << std::hex<< fCoreControl_1 << std::endl;
       }

       void GalapagosPackage::SetCoreEnabled (uint8_t ch, bool on)
       {
         //std::cout<< "GalapagosPackage::SetCoreEnabled ch="<< (int) ch<<", on="<<on << std::endl;
         if (ch > GAPG_CORES)
           return;
         uint64_t reg = GetCoreControl ();
         uint64_t flags = (1 << ch);
         if (on)
           reg |= flags;
         else
           reg &= ~flags;
         SetCoreControl (reg);
       }

       bool GalapagosPackage::IsCoreEnabled (uint8_t ch)
       {
         if (ch > GAPG_CORES)
           return true;
         uint64_t reg = GetCoreControl ();
         uint64_t flags = (1 << ch);
         bool rev = ((reg & flags) == flags);
         return rev;
       }






} // gapg
