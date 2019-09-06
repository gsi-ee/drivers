#ifndef GAPG_GALSETUP_H
#define GAPG_GALSETUP_H

#include "BasicSetup.h"
#include "BasicObjectManager.h"
#include "GalapagosDefines.h"
#include "GalapagosObjects.h"
#include <stdint.h>

namespace gapg {

/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class GalapagosSetup: public gapg::BasicSetup
{

protected:

  /** True if pattern generator is running*/
  bool fGeneratorActive;


  /* mockup of a future channel control/status register
     * each bit may set corresponding channel active*/
    uint32_t fCoreStatus_0;

    /* mockup of a future channel control/status register
     * each bit may set corresponding channel active*/
    uint32_t fCoreStatus_1;


    BasicObjectManager<GalapagosKernel> fKernelManager;

    BasicObjectManager<GalapagosPattern> fPatternManager;

    BasicObjectManager<GalapagosPackage> fPackageManager;

//  /* list of known kernels  as visible in the kernel editor*/
//  std::vector<GalapagosKernel> fKnownKernels;
//
//  /* list of known patterns  as visible in the pattern editor*/
//  std::vector<GalapagosPattern> fKnownPatterns;
//
//  /* list of known patterns  as visible in the pattern editor*/
//  std::vector<GalapagosPackage> fKnownPackages;
//
//  /* index of the currently active package in the list of known*/
//  size_t fCurrentPackageIndex;

public:

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


  uint64_t GetCoreStatus ();

      uint32_t GetCoreStatus_0 ()
      {
        return fCoreStatus_0;
      }

      void SetCoreStatus_0 (uint32_t val)
      {
        fCoreStatus_0 = val;
      }

      uint32_t GetCoreStatus_1 ()
      {
        return fCoreStatus_1;
      }
      void SetCoreStatus_1 (uint32_t val)
      {
        fCoreStatus_1 = val;
      }

      void SetCoreStatus (uint64_t val);

      /** set running flag of the hardware core*/
      void SetCoreRunning (uint8_t core, bool on);

      /** show running state of the hardware core*/
      bool IsCoreRunning (uint8_t core);

      /** enable core on the currently active package*/
      void SetCoreEnabled (uint8_t core, bool on);

      /** show enable state of the currently active package*/
      bool IsCoreEnabled (uint8_t core);


      size_t GetCurrentPackageIndex();

      void SetCurrentPackageIndex(size_t ix);

//    if(fCurrentPackageIndex>=fKnownPackages.size())
//      fCurrentPackageIndex=fKnownPackages.size()-1;
//    else
//      fCurrentPackageIndex=ix;
//  }


  GalapagosPackage& AddPackage (GalapagosPackage& pak);

    /* Access a known  package by unique id number. May be redundant if we rely on name*/
    GalapagosPackage* GetPackage (uint32_t id);

    /* Access a known package by unique name*/
    GalapagosPackage* GetPackage (const char* name);

    size_t NumKnownPackages ();

    /** access to list of known package by index */
    GalapagosPackage* GetKnownPackage (size_t ix);



    /** compile the package at given list index: Compile all referenced kernel objects and copy them into the package object.
     * After compilation the package will hold the bytecode ready to be loaded into the active cores*/
    bool CompilePackage(size_t ix);


    /** remove package from list by index*/
      void RemoveKnownPackage (size_t ix);

//////////////////////////////////////////////////////

  GalapagosKernel& AddKernel (GalapagosKernel& seq);

  /* Access a known kernel by unique id number. May be redundant if we rely on name*/
  GalapagosKernel* GetKernel (uint32_t id);

  /* Access a known kernel by unique name*/
  GalapagosKernel* GetKernel (const char* name);

  size_t NumKnownKernels ();

  /** access to list of known kernels by index */
  GalapagosKernel* GetKnownKernel (size_t ix);

  /** remove kernel from list by index. also clean up all references in channels*/
  void RemoveKnownKernel (size_t ix);

  /** Set kernel of unique id to core number chan */
  bool SetCoreKernel (int core, uint32_t id);

  /** Set kernel of unique name to core number chan */
  bool SetCoreKernel (int core, const char* name);

  /** Get reference to current kernel of core number chan */
  GalapagosKernel* GetCoreKernel (int chan);

  /** Get id number of current kernel of core number chan */
  uint32_t GetCoreKernelID (int chan);


  void ClearKernels ();

/////////////////////////////

  GalapagosPattern& AddPattern (GalapagosPattern& pat);

  /** Access a known bit pattern by unique id number. May be redundant if we rely on name*/
  GalapagosPattern* GetPattern (uint32_t id);

  /** Access a known bit pattern by unique name*/
  GalapagosPattern* GetPattern (const char* name);

  size_t NumKnownPatterns ();

  /** access to list of known bit patterns by index */
  GalapagosPattern* GetKnownPattern (size_t ix);

  /** remove pattern from list by index*/
  void RemoveKnownPattern (size_t ix);



};

} //namespace

#endif
